/**
  ******************************************************************************
  * @file    lsm6dsox_reg.h
  * @brief   Driver léger LSM6DSOX — sous-ensemble compatible API ST PID
  *
  * Cette version respecte les conventions de l'API officielle de
  * STMicroelectronics (https://github.com/STMicroelectronics/lsm6dsox-pid) :
  *   - structure de contexte `stmdev_ctx_t` avec callbacks read/write
  *   - noms de fonctions identiques (`lsm6dsox_device_id_get`, etc.)
  *
  * Elle ne contient que les fonctions nécessaires à un essai basique :
  *   - lecture du WHO_AM_I
  *   - configuration ODR / pleine échelle accéléromètre et gyroscope
  *   - lecture des données brutes (raw)
  *   - conversion en mg / mdps
  *
  * Pour le projet final (STM32G431KBT6) il suffira de remplacer ce fichier
  * et son .c par les fichiers officiels du dépôt ST sans rien changer ailleurs.
  ******************************************************************************
  */

#ifndef LSM6DSOX_REGS_H
#define LSM6DSOX_REGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* ============================================================== */
/*  Types partagés (callbacks plateforme)                          */
/* ============================================================== */

typedef int32_t (*stmdev_write_ptr)(void *handle, uint8_t reg,
                                    const uint8_t *bufp, uint16_t len);
typedef int32_t (*stmdev_read_ptr) (void *handle, uint8_t reg,
                                    uint8_t *bufp, uint16_t len);
typedef void    (*stmdev_mdelay_ptr)(uint32_t millisec);

typedef struct {
    stmdev_write_ptr  write_reg;
    stmdev_read_ptr   read_reg;
    stmdev_mdelay_ptr mdelay;
    void             *handle;
} stmdev_ctx_t;

/* ============================================================== */
/*  Adresses I2C et identification                                  */
/* ============================================================== */

#define LSM6DSOX_I2C_ADD_L   0xD5U   /* SA0 = 0  →  7 bits = 0x6A */
#define LSM6DSOX_I2C_ADD_H   0xD7U   /* SA0 = 1  →  7 bits = 0x6B */
#define LSM6DSOX_ID          0x6CU   /* valeur attendue dans WHO_AM_I */

/* ============================================================== */
/*  Registres principaux                                            */
/* ============================================================== */

#define LSM6DSOX_WHO_AM_I    0x0FU

#define LSM6DSOX_CTRL1_XL    0x10U   /* config accéléromètre */
#define LSM6DSOX_CTRL2_G     0x11U   /* config gyroscope     */
#define LSM6DSOX_CTRL3_C     0x12U   /* BDU, IF_INC, BOOT, SW_RESET */

#define LSM6DSOX_STATUS_REG  0x1EU

#define LSM6DSOX_OUT_TEMP_L  0x20U

#define LSM6DSOX_OUTX_L_G    0x22U   /* gyro : 6 octets à partir d'ici */
#define LSM6DSOX_OUTX_L_A    0x28U   /* accel : 6 octets à partir d'ici */

/* ============================================================== */
/*  Énumérations — pleine échelle et ODR                            */
/* ============================================================== */

typedef enum {
    LSM6DSOX_2g  = 0,
    LSM6DSOX_16g = 1,
    LSM6DSOX_4g  = 2,
    LSM6DSOX_8g  = 3,
} lsm6dsox_fs_xl_t;

typedef enum {
    LSM6DSOX_XL_ODR_OFF    = 0,
    LSM6DSOX_XL_ODR_12Hz5  = 1,
    LSM6DSOX_XL_ODR_26Hz   = 2,
    LSM6DSOX_XL_ODR_52Hz   = 3,
    LSM6DSOX_XL_ODR_104Hz  = 4,
    LSM6DSOX_XL_ODR_208Hz  = 5,
    LSM6DSOX_XL_ODR_416Hz  = 6,
    LSM6DSOX_XL_ODR_833Hz  = 7,
    LSM6DSOX_XL_ODR_1666Hz = 8,
    LSM6DSOX_XL_ODR_3332Hz = 9,
    LSM6DSOX_XL_ODR_6667Hz = 10,
} lsm6dsox_odr_xl_t;

typedef enum {
    LSM6DSOX_125dps  = 0x02,
    LSM6DSOX_250dps  = 0x00,
    LSM6DSOX_500dps  = 0x04,
    LSM6DSOX_1000dps = 0x08,
    LSM6DSOX_2000dps = 0x0C,
} lsm6dsox_fs_g_t;

typedef enum {
    LSM6DSOX_GY_ODR_OFF    = 0,
    LSM6DSOX_GY_ODR_12Hz5  = 1,
    LSM6DSOX_GY_ODR_26Hz   = 2,
    LSM6DSOX_GY_ODR_52Hz   = 3,
    LSM6DSOX_GY_ODR_104Hz  = 4,
    LSM6DSOX_GY_ODR_208Hz  = 5,
    LSM6DSOX_GY_ODR_416Hz  = 6,
    LSM6DSOX_GY_ODR_833Hz  = 7,
    LSM6DSOX_GY_ODR_1666Hz = 8,
    LSM6DSOX_GY_ODR_3332Hz = 9,
    LSM6DSOX_GY_ODR_6667Hz = 10,
} lsm6dsox_odr_g_t;

/* ============================================================== */
/*  API — lecture/écriture registres bas-niveau                     */
/* ============================================================== */

int32_t lsm6dsox_read_reg (stmdev_ctx_t *ctx, uint8_t reg,
                           uint8_t *data, uint16_t len);
int32_t lsm6dsox_write_reg(stmdev_ctx_t *ctx, uint8_t reg,
                           uint8_t *data, uint16_t len);

/* ============================================================== */
/*  API — identification, reset, configuration                      */
/* ============================================================== */

int32_t lsm6dsox_device_id_get(stmdev_ctx_t *ctx, uint8_t *buff);
int32_t lsm6dsox_reset_set    (stmdev_ctx_t *ctx, uint8_t val);
int32_t lsm6dsox_reset_get    (stmdev_ctx_t *ctx, uint8_t *val);
int32_t lsm6dsox_block_data_update_set(stmdev_ctx_t *ctx, uint8_t val);
int32_t lsm6dsox_auto_increment_set   (stmdev_ctx_t *ctx, uint8_t val);

int32_t lsm6dsox_xl_full_scale_set(stmdev_ctx_t *ctx, lsm6dsox_fs_xl_t fs);
int32_t lsm6dsox_xl_data_rate_set (stmdev_ctx_t *ctx, lsm6dsox_odr_xl_t odr);
int32_t lsm6dsox_gy_full_scale_set(stmdev_ctx_t *ctx, lsm6dsox_fs_g_t fs);
int32_t lsm6dsox_gy_data_rate_set (stmdev_ctx_t *ctx, lsm6dsox_odr_g_t odr);

/* ============================================================== */
/*  API — drapeaux de disponibilité (status)                        */
/* ============================================================== */

int32_t lsm6dsox_xl_flag_data_ready_get(stmdev_ctx_t *ctx, uint8_t *val);
int32_t lsm6dsox_gy_flag_data_ready_get(stmdev_ctx_t *ctx, uint8_t *val);

/* ============================================================== */
/*  API — lecture des données brutes (raw int16_t × 3)              */
/* ============================================================== */

int32_t lsm6dsox_acceleration_raw_get(stmdev_ctx_t *ctx, int16_t *val);
int32_t lsm6dsox_angular_rate_raw_get(stmdev_ctx_t *ctx, int16_t *val);
int32_t lsm6dsox_temperature_raw_get (stmdev_ctx_t *ctx, int16_t *val);

/* ============================================================== */
/*  Fonctions de conversion (raw → unités physiques)                */
/* ============================================================== */
/* Le L021 a un Cortex-M0+ sans FPU : les calculs flottants sont    */
/* faits en software. Si la flash devient tendue, vous pouvez       */
/* simplement supprimer ces conversions et travailler en LSB.       */

float lsm6dsox_from_fs2_to_mg  (int16_t lsb);
float lsm6dsox_from_fs4_to_mg  (int16_t lsb);
float lsm6dsox_from_fs8_to_mg  (int16_t lsb);
float lsm6dsox_from_fs16_to_mg (int16_t lsb);

float lsm6dsox_from_fs125_to_mdps (int16_t lsb);
float lsm6dsox_from_fs250_to_mdps (int16_t lsb);
float lsm6dsox_from_fs500_to_mdps (int16_t lsb);
float lsm6dsox_from_fs1000_to_mdps(int16_t lsb);
float lsm6dsox_from_fs2000_to_mdps(int16_t lsb);

float lsm6dsox_from_lsb_to_celsius(int16_t lsb);

#ifdef __cplusplus
}
#endif

#endif /* LSM6DSOX_REGS_H */
