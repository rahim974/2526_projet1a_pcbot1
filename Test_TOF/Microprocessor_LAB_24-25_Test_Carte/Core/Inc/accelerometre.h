/*
 * accelerometre.h
 *
 *  Created on: Mar 16, 2026
 *      Author: moatsembellhelnemrawi
 */

#ifndef INC_ACCELEROMETRE_H_
#define INC_ACCELEROMETRE_H_


#include <stdint.h>
#include "main.h"
#include "stm32l4xx_hal.h"
extern uint8_t stop_variable;

HAL_StatusTypeDef VL53L0X_Init(I2C_HandleTypeDef *hi2c);
uint16_t TOF_Read_Distance(I2C_HandleTypeDef *hi2c);
#endif /* INC_ACCELEROMETRE_H_ */
