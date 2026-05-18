/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @brief   Essai du LSM6DSOX (accéléromètre + gyroscope) sur STM32L021K4TX
  *
  *  Test progressif :
  *   1. Vérifier que le capteur répond sur le bus I2C (HAL_I2C_IsDeviceReady)
  *   2. Lire le registre WHO_AM_I — doit valoir 0x6C
  *   3. Reset logiciel, configurer ODR (104 Hz) et pleines échelles
  *   4. Boucle : lire accéléromètre + gyroscope + température, afficher
  *      les valeurs sur l'UART pour visualisation dans un terminal série
  *
  *  Brochage I2C par défaut (à confirmer dans CubeMX) :
  *    PB7 → SDA, PB6 → SCL  (I2C1)
  *  UART de log :
  *    PA2 → TX,  PA3 → RX  (USART2)  à 115200 bauds
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>

#include "lsm6dsox_reg.h"
#include "lsm6dsox_platform.h"

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef  hi2c1;
UART_HandleTypeDef huart2;

/* Contexte du driver LSM6DSOX */
static stmdev_ctx_t dev_ctx;

/* Buffers de mesure */
static int16_t accel_raw[3];
static int16_t gyro_raw[3];
static int16_t temp_raw;

/* Buffer pour formatage des chaînes */
static char tx_buf[128];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);

/* Redirection de printf vers UART2 (optionnel) */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/* Helper : écriture d'une chaîne sur l'UART */
static void uart_print(const char *s)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)s, strlen(s), HAL_MAX_DELAY);
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* MCU Configuration ----------------------------------------------------- */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    HAL_Delay(50);   /* Le LSM6DSOX a besoin de ~35 ms après alimentation */

    uart_print("\r\n===== Essai LSM6DSOX sur STM32L021K4TX =====\r\n");

    /* ---------------------------------------------------------------------- */
    /*  Étape 1 — détection sur le bus I2C                                   */
    /* ---------------------------------------------------------------------- */
    /* Le LSM6DSOX a SA0=0 par défaut → adresse 7 bits 0x6A, soit 0xD4 en
     * écriture (8 bits) du point de vue HAL. On essaye d'abord SA0=0,
     * puis SA0=1 si rien ne répond.                                         */
    uint8_t sa0 = 0;
    if (HAL_I2C_IsDeviceReady(&hi2c1, LSM6DSOX_I2C_ADD_L, 3, 100) != HAL_OK) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, LSM6DSOX_I2C_ADD_H, 3, 100) == HAL_OK) {
            sa0 = 1;
        } else {
            uart_print("[KO] Capteur introuvable sur le bus I2C\r\n");
            while (1) { HAL_Delay(1000); }
        }
    }
    sprintf(tx_buf, "[OK] Capteur detecte (SA0=%d)\r\n", sa0);
    uart_print(tx_buf);

    /* ---------------------------------------------------------------------- */
    /*  Étape 2 — initialisation du contexte PID et lecture WHO_AM_I         */
    /* ---------------------------------------------------------------------- */
    lsm6dsox_platform_init(&dev_ctx, &hi2c1, sa0);

    uint8_t whoami = 0;
    lsm6dsox_device_id_get(&dev_ctx, &whoami);
    sprintf(tx_buf, "WHO_AM_I = 0x%02X (attendu 0x%02X)\r\n", whoami, LSM6DSOX_ID);
    uart_print(tx_buf);

    if (whoami != LSM6DSOX_ID) {
        uart_print("[KO] WHO_AM_I incorrect — verifier cablage/adresse\r\n");
        while (1) { HAL_Delay(1000); }
    }

    /* ---------------------------------------------------------------------- */
    /*  Étape 3 — reset logiciel puis configuration                          */
    /* ---------------------------------------------------------------------- */
    lsm6dsox_reset_set(&dev_ctx, 1);
    uint8_t rst;
    do {
        lsm6dsox_reset_get(&dev_ctx, &rst);
    } while (rst);

    /* BDU = 1 : protège les mesures multi-octets contre les mises à jour
     *           en cours de lecture (lecture cohérente MSB+LSB). */
    lsm6dsox_block_data_update_set(&dev_ctx, 1);

    /* Auto-increment pour les lectures multi-registres */
    lsm6dsox_auto_increment_set(&dev_ctx, 1);

    /* Accéléromètre : 104 Hz, ±2 g  (bon compromis vibration/bruit) */
    lsm6dsox_xl_full_scale_set(&dev_ctx, LSM6DSOX_2g);
    lsm6dsox_xl_data_rate_set (&dev_ctx, LSM6DSOX_XL_ODR_104Hz);

    /* Gyroscope    : 104 Hz, ±250 dps */
    lsm6dsox_gy_full_scale_set(&dev_ctx, LSM6DSOX_250dps);
    lsm6dsox_gy_data_rate_set (&dev_ctx, LSM6DSOX_GY_ODR_104Hz);

    uart_print("[OK] Capteur configure — debut des mesures\r\n\r\n");

    /* ---------------------------------------------------------------------- */
    /*  Étape 4 — boucle de mesure                                           */
    /* ---------------------------------------------------------------------- */
    while (1)
    {
        uint8_t drdy_xl = 0, drdy_g = 0;
        lsm6dsox_xl_flag_data_ready_get(&dev_ctx, &drdy_xl);
        lsm6dsox_gy_flag_data_ready_get(&dev_ctx, &drdy_g);

        if (drdy_xl) {
            lsm6dsox_acceleration_raw_get(&dev_ctx, accel_raw);
        }
        if (drdy_g) {
            lsm6dsox_angular_rate_raw_get(&dev_ctx, gyro_raw);
        }
        lsm6dsox_temperature_raw_get(&dev_ctx, &temp_raw);

        /* Conversion en unités physiques */
        float ax_mg = lsm6dsox_from_fs2_to_mg(accel_raw[0]);
        float ay_mg = lsm6dsox_from_fs2_to_mg(accel_raw[1]);
        float az_mg = lsm6dsox_from_fs2_to_mg(accel_raw[2]);

        float gx_mdps = lsm6dsox_from_fs250_to_mdps(gyro_raw[0]);
        float gy_mdps = lsm6dsox_from_fs250_to_mdps(gyro_raw[1]);
        float gz_mdps = lsm6dsox_from_fs250_to_mdps(gyro_raw[2]);

        float t_degC = lsm6dsox_from_lsb_to_celsius(temp_raw);

        /* On affiche en entiers (millig, millideg/s, dixièmes de °C) pour
         * éviter d'activer le support de printf-float (coûteux en flash). */
        sprintf(tx_buf,
                "ACC[mg] X=%+5d Y=%+5d Z=%+5d  |  "
                "GYR[mdps] X=%+6d Y=%+6d Z=%+6d  |  "
                "T=%d.%d C\r\n",
                (int)ax_mg, (int)ay_mg, (int)az_mg,
                (int)gx_mdps, (int)gy_mdps, (int)gz_mdps,
                (int)t_degC, (int)((t_degC - (int)t_degC) * 10));
        uart_print(tx_buf);

        HAL_Delay(200);   /* 5 affichages/seconde */
    }
}

/* ========================================================================== */
/*                  Initialisations périphériques (générées par CubeMX)        */
/* ========================================================================== */

/**
  * @brief System Clock Configuration — HSI à 16 MHz (par défaut au boot)
  *        Le LSM6DSOX ne demande aucun timing critique côté MCU pour ce test.
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState       = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
        Error_Handler();
}

/**
  * @brief I2C1 Initialization  — 100 kHz standard mode
  *        Pins : PB6 = SCL, PB7 = SDA (à confirmer dans CubeMX)
  */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance              = I2C1;
    hi2c1.Init.Timing           = 0x00303D5B;   /* 100 kHz @ HSI 16 MHz   */
    hi2c1.Init.OwnAddress1      = 0;
    hi2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2      = 0;
    hi2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

/**
  * @brief USART2 Initialization — 115200 8N1
  *        Pins : PA2 = TX, PA3 = RX
  */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

/**
  * @brief GPIO Initialization — horloges des ports utilisés
  */
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
  * @brief Gestionnaire d'erreur — clignote ou bloque selon votre préférence
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { (void)file; (void)line; }
#endif
