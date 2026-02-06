# Buffer Overflow Fix - DIoLLaserUnit

## Root Cause
A **buffer overflow** occurred during `DIoLLaserUnit` termination because memory-mapped register addresses were calculated **without bounds validation**. 

### The Problem

In the register configuration parsing (line 3223 of the original code):
```cpp
tmp.addr = (m_reg_map.at(tmp.reg_id).vaddr+(tmp.offset*GPIO_CH_OFFSET));
```

The code calculated a pointer address by:
1. Taking a base virtual address from the memory map
2. Adding `offset * GPIO_CH_OFFSET` to it
3. **Never validating** that this new address stays within the mapped region

Each memory region is only **0xFFF bytes (4095 bytes)**, but the `offset` value from the JSON configuration could be arbitrarily large, creating pointers that point beyond the allocated memory.

## Impact
When these invalid pointers were later dereferenced during:
- Register reads/writes in `set_qswitch_delay()`, `disable_fire()`, etc.
- Destructor cleanup (`cib_free_mem()`)

The code would access memory outside the mapped region → **buffer overflow crash**.

## Fixes Applied

### 1. **Validate Bit Range (lines 3220-3228)**
Added validation to ensure `bit_high` and `bit_low` form a valid bit mask:
```cpp
if (tmp.bit_high < tmp.bit_low || tmp.bit_high >= 32 || tmp.bit_low >= 32)
{
  // Return error with descriptive message
}
```

### 2. **Validate Register Offset (lines 3230-3238)**
Added validation to ensure the calculated offset doesn't exceed the memory region size:
```cpp
uint32_t calculated_offset = tmp.offset * GPIO_CH_OFFSET;
uint32_t region_size = m_reg_map.at(tmp.reg_id).size;
if (calculated_offset > region_size)
{
  // Return error with descriptive message
}
```

### 3. **Hardened Memory Cleanup (lines 4151-4167)**
Improved `cib_free_mem()` to safely handle cleanup:
- Check that `vaddr != 0` before unmapping
- Check that `m_mmap_fd > 0` before closing the file descriptor
- Call `m_reg_map.clear()` to invalidate all register entries
- Fixed typo: "Unammping" → "Unmapping"

## Memory Layout
Each memory region layout:
```
Base Address (vaddr)
       ↓
    [0x00][0x08][0x10][0x18]...[0xFF8]
    offset  offset  offset      max_offset
      ×8     ×8     ×8          (0xFFF / GPIO_CH_OFFSET)
      0      1      2           N
```

If `offset` exceeds `region_size / GPIO_CH_OFFSET`, the pointer escapes the bounds.

## Testing Recommendations
1. **Fuzz the JSON configuration** with large offset values (> 4095)
2. **Verify log messages** appear for invalid configurations instead of crashes
3. **Check shutdown sequence** completes cleanly with valid configs
4. **Run with AddressSanitizer** to catch any remaining out-of-bounds access:
   ```bash
   export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
   ```

## Result
Invalid configurations are now **safely rejected at parse time** with clear error messages, instead of causing **buffer overflows during runtime or shutdown**.
