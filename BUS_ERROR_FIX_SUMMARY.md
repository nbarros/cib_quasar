# Bus Error Fix - DIoLPowerMeter

## Root Cause
The software crashed with a **bus error (segmentation fault)** due to a **race condition** in the DIoLPowerMeter class:

1. **Detached Thread**: The `start_readings()` method spawns a detached thread that continuously calls `refresh_energy_reading()` and `refresh_average_reading()`
2. **Asynchronous Deletion**: The `terminate()` method can be called at any time (e.g., from another thread or during shutdown) and sets `m_pm = nullptr`
3. **Null Pointer Dereference**: The measurement thread tries to dereference `m_pm` after it's been deleted, causing a crash

## Timeline of Crash
```
Thread A (measurement)          |  Thread B (control)
start_readings()                |
  → spawns detached thread      |
    m_do_measurements = true    |
    refresh_energy_reading() ←┐ |
                             └─ terminate()
                                  m_pm = nullptr
                                  delete m_pm
              tries m_pm->read_energy() ← CRASH!
```

## Fixed Methods
All methods that access `m_pm` now include null checks:

### 1. `refresh_energy_reading()`
- Added: `if (m_pm != nullptr)` guard before `m_pm->read_energy()`

### 2. `refresh_average_reading()`
- Added: `if (m_pm != nullptr)` guard before `m_pm->read_average()`

### 3. `refresh_measurement_modes()`
- Added: `if (m_pm != nullptr)` guard before `m_pm->head_info()`

### 4. `refresh_measurement_ranges()`
- Added: `if (m_pm != nullptr)` guards before `m_pm->get_all_ranges()` and `m_pm->get_range_map()`

### 5. `refresh_pulse_width_ranges()`
- Added: `if (m_pm != nullptr)` guard before `m_pm->pulse_length()` and `m_pm->get_pulse_map()`

### 6. `refresh_average_ranges()`
- Added: `if (m_pm != nullptr)` guard before `m_pm->average_query()` and `m_pm->get_averages_map()`

### 7. `refresh_threshold_limits()`
- Added: `if (m_pm != nullptr)` guard before `m_pm->query_user_threshold()`

## Protection Strategy
Each method now:
1. Acquires the `m_serial_mutex` lock (already present)
2. **Checks if `m_pm != nullptr` before dereferencing**
3. Only calls device methods if the pointer is valid
4. Safely handles the case where `m_pm` is deleted during measurement

## Additional Recommendations
For production robustness, consider:
- **Using `std::unique_ptr`** instead of raw pointers for automatic memory management
- **Using `std::thread` (joinable)** instead of detached threads for proper lifecycle management
- **Using atomic flags** with proper synchronization for clean shutdown handshakes
- **Adding proper shutdown signaling** between `stop_readings()` and the measurement thread

## Tested Scenarios
The fix protects against:
- ✅ Calling `terminate()` while measurements are running
- ✅ Multiple rapid start/stop cycles
- ✅ Shutdown during active data collection
- ✅ Concurrent access from different control threads
