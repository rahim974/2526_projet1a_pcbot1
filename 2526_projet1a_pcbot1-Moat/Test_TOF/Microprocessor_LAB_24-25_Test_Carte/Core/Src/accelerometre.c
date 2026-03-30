/*
 * accelerometre.c
 *
 *  Created on: Mar 16, 2026
 *      Author: moatsembellhelnemrawi
 */

#include "accelerometre.h"
#include "main.h"
#include "stm32l4xx_hal.h"
extern I2C_HandleTypeDef hi2c1;
uint8_t stop_variable = 0;

// Stop variable global — doit être lu pendant l'init


HAL_StatusTypeDef VL53L0X_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t val;

    // 1. Vérifier le modèle
    HAL_I2C_Mem_Read(hi2c, 0x52, 0xC0, 1, &val, 1, 10);
    if (val != 0xEE) return HAL_ERROR; // mauvais capteur

    // 2. Séquence de déverrouillage CORRECTE
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x88, 1, &val, 1, 10);

    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x80, 1, &val, 1, 10);
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0xFF, 1, &val, 1, 10);
    val = 0x00; HAL_I2C_Mem_Write(hi2c, 0x52, 0x00, 1, &val, 1, 10);

    // Lire stop_variable (valeur dynamique !)
    HAL_I2C_Mem_Read(hi2c, 0x52, 0x91, 1, &stop_variable, 1, 10);

    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x00, 1, &val, 1, 10);
    val = 0x00; HAL_I2C_Mem_Write(hi2c, 0x52, 0xFF, 1, &val, 1, 10);
    val = 0x00; HAL_I2C_Mem_Write(hi2c, 0x52, 0x80, 1, &val, 1, 10);

    // 3. Désactiver SIGNAL_RATE_MSRC et SIGNAL_RATE_PRE_RANGE
    val = 0x12; HAL_I2C_Mem_Write(hi2c, 0x52, 0x60, 1, &val, 1, 10);

    // 4. Config interruption sur new sample ready
    val = 0x04; HAL_I2C_Mem_Write(hi2c, 0x52, 0x0A, 1, &val, 1, 10); // GPIO_HV_MUX_ACTIVE_HIGH
    val = 0x00; HAL_I2C_Mem_Write(hi2c, 0x52, 0x84, 1, &val, 1, 10); // SYSTEM_INTERRUPT_CONFIG_GPIO

    // 5. Clear interrupt
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x0B, 1, &val, 1, 10);

    return HAL_OK;
}

uint16_t VL53L0X_ReadDistance(I2C_HandleTypeDef *hi2c) {
    uint8_t val, data[2];

    // 1. Séquence de démarrage single shot
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x80, 1, &val, 1, 10);
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0xFF, 1, &val, 1, 10);
    val = 0x00; HAL_I2C_Mem_Write(hi2c, 0x52, 0x00, 1, &val, 1, 10);
    HAL_I2C_Mem_Write(hi2c, 0x52, 0x91, 1, &stop_variable, 1, 10);
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x00, 1, &val, 1, 10);
    val = 0x00; HAL_I2C_Mem_Write(hi2c, 0x52, 0xFF, 1, &val, 1, 10);
    val = 0x00; HAL_I2C_Mem_Write(hi2c, 0x52, 0x80, 1, &val, 1, 10);

    // 2. Lancer la mesure
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x00, 1, &val, 1, 10);

    // 3. Attendre data ready (bit[2:0] de RESULT_INTERRUPT_STATUS != 0)
    uint32_t timeout = HAL_GetTick();
    do {
        HAL_I2C_Mem_Read(hi2c, 0x52, 0x13, 1, &val, 1, 10);
        if (HAL_GetTick() - timeout > 500) return 8190; // timeout
    } while ((val & 0x07) == 0);

    // 4. Lire la distance (registre 0x1E = MSB, 0x1F = LSB)
    HAL_I2C_Mem_Read(hi2c, 0x52, 0x1E, 1, data, 2, 10);

    // 5. Clear interrupt
    val = 0x01; HAL_I2C_Mem_Write(hi2c, 0x52, 0x0B, 1, &val, 1, 10);

    return (uint16_t)((data[0] << 8) | data[1]);
}
