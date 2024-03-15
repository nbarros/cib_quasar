/*
 * cib_registers.h
 *
 *  Created on: Mar 15, 2024
 *      Author: Nuno Barros
 */

#ifndef DEVICE_INCLUDE_CIB_REGISTERS_H_
#define DEVICE_INCLUDE_CIB_REGISTERS_H_

extern "C"
{
#include <inttypes.h>
};

namespace cib
{
  namespace mem
  {
    uintptr_t config_base_addr= 0x00A0040000;
    uintptr_t config_high_addr= 0x00A004FFFF;
    uintptr_t control_gpio_addr_lo =  0x00A0010000;
    uintptr_t control_gpio_addr_hi =  0x00A001FFFF;
    uintptr_t mon1_gpio_addr_lo =  0x00A0020000;
    uintptr_t mon1_gpio_addr_hi =  0x00A002FFFF;
    uintptr_t mon2_gpio_addr_lo =  0x00A0030000;
    uintptr_t mon3_gpio_addr_hi =  0x00A003FFFF;

    // GPIO offsets
    uint32_t gpio_ch0_offset = 0x0;
    uint32_t gpio_ch1_offset = 0x8;

    // config offsets
#define CONFIG_NUM_REGS 16
#define EXT_SHUTTER_REG 0
#define EXT_SHUTTER_BIT 30

    // motor relevant registers
#define MOTOR_

  };
};



#endif /* DEVICE_INCLUDE_CIB_REGISTERS_H_ */
