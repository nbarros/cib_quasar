/*
 * cib_registers.h
 *
 *  Created on: Mar 15, 2024
 *      Author: Nuno Barros
 *
 *  Note: Register memory address definitions are now included from cib_utils/common/cib_mem.h
 *  to maintain consistency with the parent project and avoid duplication.
 *  Local register indices remain for organizational purposes.
 */

#ifndef DEVICE_INCLUDE_CIB_REGISTERS_H_
#define DEVICE_INCLUDE_CIB_REGISTERS_H_

// Include register memory definitions from parent cib_utils project
#include <cib_mem.h>

// Register indices (used for internal organization)
#define PDTS_REG 0
#define I_0_REG 1
#define I_1_REG 2
#define ALIGN_REG 3
#define LASER_REG 4
#define MISC_REG 5
#define MOTOR_1_REG 6
#define MOTOR_2_REG 7
#define MOTOR_3_REG 8
#define TRIGGER_REG 9
#define TSTAMP_REG 10

// GPIO channel offsets are now defined in cib_mem.h as GPIO_CH_OFFSET

#endif /* DEVICE_INCLUDE_CIB_REGISTERS_H_ */
