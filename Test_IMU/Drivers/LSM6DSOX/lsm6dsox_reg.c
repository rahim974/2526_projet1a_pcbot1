/**
  ******************************************************************************
  * @file    lsm6dsox_reg.c
  * @brief   Implémentation du driver léger LSM6DSOX
  *
  * Sous-ensemble de l'API officielle STMicroelectronics — voir lsm6dsox_reg.h.
  ******************************************************************************
  */

#include "lsm6dsox_reg.h"

/* ================================================================ */
/*  Lecture / écriture bas-niveau                                    */
/* ================================================================ */

int32_t lsm6dsox_read_reg(stmdev_ctx_t *ctx, uint8_t reg,
                          uint8_t *data, uint16_t len)
{
    if (ctx == NULL) return -1;
    return ctx->read_reg(ctx->handle, reg, data, len);
}

int32_t lsm6dsox_write_reg(stmdev_ctx_t *ctx, uint8_t reg,
                           uint8_t *data, uint16_t len)
{
    if (ctx == NULL) return -1;
    return ctx->write_reg(ctx->handle, reg, data, len);
}

/* ================================================================ */
/*  Identification                                                   */
/* ================================================================ */

int32_t lsm6dsox_device_id_get(stmdev_ctx_t *ctx, uint8_t *buff)
{
    return lsm6dsox_read_reg(ctx, LSM6DSOX_WHO_AM_I, buff, 1);
}

/* ================================================================ */
/*  Reset et configuration globale (CTRL3_C)                         */
/* ================================================================ */

int32_t lsm6dsox_reset_set(stmdev_ctx_t *ctx, uint8_t val)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL3_C, &reg, 1);
    if (ret == 0) {
        /* SW_RESET = bit 0 */
        reg = (reg & 0xFEU) | (val & 0x01U);
        ret = lsm6dsox_write_reg(ctx, LSM6DSOX_CTRL3_C, &reg, 1);
    }
    return ret;
}

int32_t lsm6dsox_reset_get(stmdev_ctx_t *ctx, uint8_t *val)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL3_C, &reg, 1);
    *val = reg & 0x01U;
    return ret;
}

int32_t lsm6dsox_block_data_update_set(stmdev_ctx_t *ctx, uint8_t val)
{
    /* BDU = bit 6 de CTRL3_C */
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL3_C, &reg, 1);
    if (ret == 0) {
        reg = (reg & 0xBFU) | ((val & 0x01U) << 6);
        ret = lsm6dsox_write_reg(ctx, LSM6DSOX_CTRL3_C, &reg, 1);
    }
    return ret;
}

int32_t lsm6dsox_auto_increment_set(stmdev_ctx_t *ctx, uint8_t val)
{
    /* IF_INC = bit 2 de CTRL3_C — auto-incrément d'adresse en lecture multiple */
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL3_C, &reg, 1);
    if (ret == 0) {
        reg = (reg & 0xFBU) | ((val & 0x01U) << 2);
        ret = lsm6dsox_write_reg(ctx, LSM6DSOX_CTRL3_C, &reg, 1);
    }
    return ret;
}

/* ================================================================ */
/*  Accéléromètre — pleine échelle et ODR (CTRL1_XL)                 */
/*  Structure de CTRL1_XL :                                          */
/*    [7:4] ODR_XL  [3:2] FS_XL  [1] LPF2_XL_EN  [0] (réservé)       */
/* ================================================================ */

int32_t lsm6dsox_xl_full_scale_set(stmdev_ctx_t *ctx, lsm6dsox_fs_xl_t fs)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL1_XL, &reg, 1);
    if (ret == 0) {
        reg = (reg & 0xF3U) | (((uint8_t)fs & 0x03U) << 2);
        ret = lsm6dsox_write_reg(ctx, LSM6DSOX_CTRL1_XL, &reg, 1);
    }
    return ret;
}

int32_t lsm6dsox_xl_data_rate_set(stmdev_ctx_t *ctx, lsm6dsox_odr_xl_t odr)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL1_XL, &reg, 1);
    if (ret == 0) {
        reg = (reg & 0x0FU) | (((uint8_t)odr & 0x0FU) << 4);
        ret = lsm6dsox_write_reg(ctx, LSM6DSOX_CTRL1_XL, &reg, 1);
    }
    return ret;
}

/* ================================================================ */
/*  Gyroscope — pleine échelle et ODR (CTRL2_G)                      */
/*  Structure de CTRL2_G :                                           */
/*    [7:4] ODR_G  [3:1] FS_G(incl. FS_125)  [0] (réservé)           */
/* ================================================================ */

int32_t lsm6dsox_gy_full_scale_set(stmdev_ctx_t *ctx, lsm6dsox_fs_g_t fs)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL2_G, &reg, 1);
    if (ret == 0) {
        reg = (reg & 0xF1U) | ((uint8_t)fs & 0x0EU);
        ret = lsm6dsox_write_reg(ctx, LSM6DSOX_CTRL2_G, &reg, 1);
    }
    return ret;
}

int32_t lsm6dsox_gy_data_rate_set(stmdev_ctx_t *ctx, lsm6dsox_odr_g_t odr)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_CTRL2_G, &reg, 1);
    if (ret == 0) {
        reg = (reg & 0x0FU) | (((uint8_t)odr & 0x0FU) << 4);
        ret = lsm6dsox_write_reg(ctx, LSM6DSOX_CTRL2_G, &reg, 1);
    }
    return ret;
}

/* ================================================================ */
/*  Status — drapeaux data-ready                                     */
/*  STATUS_REG : [0] XLDA  [1] GDA  [2] TDA                          */
/* ================================================================ */

int32_t lsm6dsox_xl_flag_data_ready_get(stmdev_ctx_t *ctx, uint8_t *val)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_STATUS_REG, &reg, 1);
    *val = reg & 0x01U;
    return ret;
}

int32_t lsm6dsox_gy_flag_data_ready_get(stmdev_ctx_t *ctx, uint8_t *val)
{
    uint8_t reg;
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_STATUS_REG, &reg, 1);
    *val = (reg >> 1) & 0x01U;
    return ret;
}

/* ================================================================ */
/*  Lecture des données brutes (3 axes × 2 octets, little-endian)    */
/* ================================================================ */

int32_t lsm6dsox_acceleration_raw_get(stmdev_ctx_t *ctx, int16_t *val)
{
    uint8_t buf[6];
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_OUTX_L_A, buf, 6);
    val[0] = (int16_t)((uint16_t)buf[1] << 8) | buf[0];
    val[1] = (int16_t)((uint16_t)buf[3] << 8) | buf[2];
    val[2] = (int16_t)((uint16_t)buf[5] << 8) | buf[4];
    return ret;
}

int32_t lsm6dsox_angular_rate_raw_get(stmdev_ctx_t *ctx, int16_t *val)
{
    uint8_t buf[6];
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_OUTX_L_G, buf, 6);
    val[0] = (int16_t)((uint16_t)buf[1] << 8) | buf[0];
    val[1] = (int16_t)((uint16_t)buf[3] << 8) | buf[2];
    val[2] = (int16_t)((uint16_t)buf[5] << 8) | buf[4];
    return ret;
}

int32_t lsm6dsox_temperature_raw_get(stmdev_ctx_t *ctx, int16_t *val)
{
    uint8_t buf[2];
    int32_t ret = lsm6dsox_read_reg(ctx, LSM6DSOX_OUT_TEMP_L, buf, 2);
    *val = (int16_t)((uint16_t)buf[1] << 8) | buf[0];
    return ret;
}

/* ================================================================ */
/*  Conversions — sensibilités identiques à la datasheet ST          */
/* ================================================================ */

float lsm6dsox_from_fs2_to_mg (int16_t lsb) { return ((float)lsb) * 0.061f;   }
float lsm6dsox_from_fs4_to_mg (int16_t lsb) { return ((float)lsb) * 0.122f;   }
float lsm6dsox_from_fs8_to_mg (int16_t lsb) { return ((float)lsb) * 0.244f;   }
float lsm6dsox_from_fs16_to_mg(int16_t lsb) { return ((float)lsb) * 0.488f;   }

float lsm6dsox_from_fs125_to_mdps (int16_t lsb) { return ((float)lsb) * 4.375f;  }
float lsm6dsox_from_fs250_to_mdps (int16_t lsb) { return ((float)lsb) * 8.750f;  }
float lsm6dsox_from_fs500_to_mdps (int16_t lsb) { return ((float)lsb) * 17.500f; }
float lsm6dsox_from_fs1000_to_mdps(int16_t lsb) { return ((float)lsb) * 35.000f; }
float lsm6dsox_from_fs2000_to_mdps(int16_t lsb) { return ((float)lsb) * 70.000f; }

/* Température : sensibilité 256 LSB/°C, offset 25 °C */
float lsm6dsox_from_lsb_to_celsius(int16_t lsb)
{
    return (((float)lsb) / 256.0f) + 25.0f;
}
