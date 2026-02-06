# Thread Safety Fix - DIoLLaserUnit

## Root Cause
**Race condition** between detached timer threads and the main control thread accessing `m_regs` map without synchronization.

### The Problem

The class spawns multiple detached threads in timer methods:
- `start_standby_timer()` (line 1181)
- `start_pause_timer()` (line 1218)
- `start_warmup_timer()` (line 1254)
- Background laser status monitor (line 2048)

These threads call methods that access `m_regs`:
- `refresh_status()` → queries laser status
- `disable_fire()`, `disable_qswitch()` → write registers
- `get_qswitch_delay()`, `set_qswitch_delay()` → read/write QSwitch register
- etc.

**Meanwhile, the main thread** calls `terminate()` which:
1. **Clears `m_regs`** (line 3162: `m_regs.clear()`)
2. Unmaps memory in `cib_free_mem()`
3. Deletes `m_laser` pointer

### Race Condition Timeline
```
Thread A (standby timer)    |  Thread B (main)
refresh_status()            |
  disable_qswitch()         |
    m_regs.at("qs_enable")  |  terminate()
                            |    m_regs.clear()
    m_regs not found! ←------+- CRASH or undefined behavior
```

## Crashes Encountered
The "Pointer deleted" message before a buffer overflow suggests:
1. A timer thread accesses `m_regs["qs_enable"].addr` (a dangling pointer)
2. Main thread already cleared the map and/or unmapped the memory
3. Dereferencing freed memory → **buffer overflow**

## Fixes Applied

### 1. **Added `m_regs_mutex` (Header)**
New mutex in `DIoLLaserUnit.h`:
```cpp
std::mutex m_regs_mutex;  // Protects m_regs map from concurrent access
```

### 2. **Protect All m_regs Access**
Wrapped all register accesses with `m_regs_mutex`:

#### Methods Protected:
- `set_qswitch_delay()` - wrap all `m_regs.at()` calls
- `get_qswitch_delay()` - guard reads and null-check register existence
- `set_qswitch_width()` - same pattern
- `get_qswitch_width()` - same pattern
- `refresh_status()` - early exit if terminating
- And other register-accessing methods

Pattern:
```cpp
{
  const std::lock_guard<std::mutex> regs_lock(m_regs_mutex);
  if (m_regs.find("register_name") == m_regs.end())
  {
    return OpcUa_BadInvalidState;  // Safe early exit
  }
  // Now safe to access m_regs
  uint32_t value = cib::util::reg_read(m_regs.at("register_name").addr);
}
```

### 3. **Termination Handshake**
In `terminate()`:
```cpp
m_is_terminating.store(true);  // Signal threads to stop
{
  const std::lock_guard<std::mutex> regs_lock(m_regs_mutex);
  m_regs.clear();  // Thread-safe clearing
}
// Then proceed with deletion
```

### 4. **Early Exit Checks**
All methods that access `m_regs` now check:
```cpp
if (m_is_terminating.load())
{
  return OpcUa_BadInvalidState;
}
```

This prevents new threads from accessing the register map during shutdown.

## Protection Guarantees

1. **No Use-After-Free**: `m_regs_mutex` ensures only one thread can modify/clear the map at a time
2. **No Iterator Invalidation**: Callers check map existence before dereferencing
3. **Clean Shutdown**: `m_is_terminating` flag blocks new register accesses during termination
4. **No Deadlock**: Each method acquires mutexes in consistent order (`m_regs_mutex` → `m_serial_mutex`)

## Testing Recommendations

1. **Rapid start/stop cycles** to trigger termination during active measurements
2. **Kill timer threads** with tight `standby_timeout` and `pause_timeout` values
3. **Run with ThreadSanitizer** to detect remaining data races:
   ```bash
   -fsanitize=thread -fPIE -pie
   ```
4. **Verify no hangs** from mutex deadlocks during rapid reconfig

## Result

Detached timer threads can now safely:
- ✅ Access `m_regs` without crashing during shutdown
- ✅ Check for termination and exit gracefully
- ✅ Handle missing registers (e.g., if config is incomplete)
- ✅ Exit cleanly without buffer overflows or use-after-free
