/**
  ******************************************************************************
  * @file    lsm6dsox_platform.h
  * @brief   Couche d'adaptation — fournit les callbacks I2C HAL au driver PID
  *
  * Équivalent du dossier `platform/` du driver VL53L0X.
  * À adapter si vous changez de MCU (par ex. STM32G4 → modifier l'include).
  ******************************************************************************
  */

#ifndef LSM6DSOX_PLATFORM_H
#define LSM6DSOX_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l0xx_hal.h"      /* STM32L021K4TX → famille L0 */
#include "lsm6dsox_reg.h"

/**
 * @brief Initialise le contexte stmdev_ctx_t pour utiliser un bus I2C HAL.
 *        Renseigne les callbacks read_reg / write_reg / mdelay.
 *
 * @param ctx    contexte à remplir
 * @param hi2c   pointeur sur le handle HAL I2C déjà initialisé
 * @param sa0    valeur de la broche SA0 du capteur (0 ou 1)
 */
void lsm6dsox_platform_init(stmdev_ctx_t *ctx, I2C_HandleTypeDef *hi2c, uint8_t sa0);

#ifdef __cplusplus
}
#endif

#endif /* LSM6DSOX_PLATFORM_H */
