/**
 * @file    motorV2.h
 * @brief   Interface publique du module de contrôle moteurs (DRV8833 + STM32 TIM1)
 */

#ifndef MOTORV2_H
#define MOTORV2_H

#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  API publique                                                        */
/* ------------------------------------------------------------------ */

void Motor_Init(void);
void Motor_Forward(void);
void Motor_Stop(void);
void Motor_TurnRight90(void);
void Motor_NavigationUpdate(uint16_t distance_mm);

#endif /* MOTORV2_H */
