/*
 * cib_registers.h
 *
 *  Created on: Mar 15, 2024
 *      Author: Nuno Barros
 */

#ifndef DEVICE_INCLUDE_CIB_REGISTERS_H_
#define DEVICE_INCLUDE_CIB_REGISTERS_H_

#define CIB_CONFIG_ADDR_BASE      0x00A0040000
#define CIB_CONFIG_ADDR_HIGH      0x00A004FFFF

#define CIB_CTRL_GPIO_ADDR_BASE   0x00A0010000
#define CIB_CTRL_GPIO_ADDR_HIGH   0x00A001FFFF

#define CIB_MON1_GPIO_ADDR_BASE   0x00A0020000
#define CIB_MON1_GPIO_ADDR_HIGH   0x00A002FFFF

#define CIB_MON2_GPIO_ADDR_BASE   0x00A0030000
#define CIB_MON2_GPIO_ADDR_HIGH   0x00A003FFFF


// GPIO offsets
#define CIB_GPIO_OFFSET_CH0 0x0
#define CIB_GPIO_OFFSET_CH1 0x8


// config offsets
#define CIB_CONFIG_NUM_REGS 16


#endif /* DEVICE_INCLUDE_CIB_REGISTERS_H_ */
