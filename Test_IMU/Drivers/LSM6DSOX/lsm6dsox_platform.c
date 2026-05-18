/**
  ******************************************************************************
  * @file    lsm6dsox_platform.c
  * @brief   Implémentation des callbacks I2C HAL pour le driver PID
  ******************************************************************************
  */

#include "lsm6dsox_platform.h"

/* Structure interne — on stocke ici le handle I2C et l'adresse du device
 * pour ne pas avoir à passer plusieurs paramètres aux callbacks. */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t            addr8;   /* adresse 8 bits (déjà décalée à gauche) */
} lsm6dsox_handle_t;

/* Un seul capteur dans cet essai → handle statique. */
static lsm6dsox_handle_t g_handle;

/* ============================================================== */
/*  Callback d'écriture I2C — signature stmdev_write_ptr           */
/* ============================================================== */
static int32_t platform_write(void *handle, uint8_t reg,
                              const uint8_t *bufp, uint16_t len)
{
    lsm6dsox_handle_t *h = (lsm6dsox_handle_t *)handle;
    HAL_StatusTypeDef st = HAL_I2C_Mem_Write(h->hi2c, h->addr8, reg,
                                             I2C_MEMADD_SIZE_8BIT,
                                             (uint8_t *)bufp, len, 100);
    return (st == HAL_OK) ? 0 : -1;
}

/* ============================================================== */
/*  Callback de lecture I2C — signature stmdev_read_ptr            */
/* ============================================================== */
static int32_t platform_read(void *handle, uint8_t reg,
                             uint8_t *bufp, uint16_t len)
{
    lsm6dsox_handle_t *h = (lsm6dsox_handle_t *)handle;
    HAL_StatusTypeDef st = HAL_I2C_Mem_Read(h->hi2c, h->addr8, reg,
                                            I2C_MEMADD_SIZE_8BIT,
                                            bufp, len, 100);
    return (st == HAL_OK) ? 0 : -1;
}

/* ============================================================== */
/*  Callback de délai en millisecondes                              */
/* ============================================================== */
static void platform_mdelay(uint32_t ms)
{
    HAL_Delay(ms);
}

/* ============================================================== */
/*  Initialisation du contexte                                      */
/* ============================================================== */
void lsm6dsox_platform_init(stmdev_ctx_t *ctx,
                            I2C_HandleTypeDef *hi2c,
                            uint8_t sa0)
{
    g_handle.hi2c  = hi2c;
    g_handle.addr8 = (sa0 == 0) ? LSM6DSOX_I2C_ADD_L : LSM6DSOX_I2C_ADD_H;

    ctx->write_reg = platform_write;
    ctx->read_reg  = platform_read;
    ctx->mdelay    = platform_mdelay;
    ctx->handle    = &g_handle;
}
