#ifndef NRF24_TX_H
#define NRF24_TX_H

#include "stm32f4xx_hal.h"  // Adapter selon ta famille STM32 (f1xx, f3xx, etc.)
#include <stdint.h>

/* ============================================================
 *  GPIO — à adapter selon ta configuration CubeMX
 * ============================================================ */
#define NRF_CE_GPIO_Port    GPIOA
#define NRF_CE_Pin          GPIO_PIN_1
#define NRF_CSN_GPIO_Port   GPIOA
#define NRF_CSN_Pin         GPIO_PIN_2

/* ============================================================
 *  Registres nRF24L01+
 * ============================================================ */
#define NRF_REG_CONFIG      0x00
#define NRF_REG_EN_AA       0x01
#define NRF_REG_EN_RXADDR   0x02
#define NRF_REG_SETUP_AW    0x03
#define NRF_REG_SETUP_RETR  0x04
#define NRF_REG_RF_CH       0x05
#define NRF_REG_RF_SETUP    0x06
#define NRF_REG_STATUS      0x07
#define NRF_REG_OBSERVE_TX  0x08
#define NRF_REG_RX_ADDR_P0  0x0A
#define NRF_REG_TX_ADDR     0x10
#define NRF_REG_RX_PW_P0    0x11
#define NRF_REG_FIFO_STATUS 0x17

/* ============================================================
 *  Commandes SPI nRF24L01+
 * ============================================================ */
#define NRF_CMD_R_REGISTER      0x00
#define NRF_CMD_W_REGISTER      0x20
#define NRF_CMD_W_TX_PAYLOAD    0xA0
#define NRF_CMD_FLUSH_TX        0xE1
#define NRF_CMD_FLUSH_RX        0xE2
#define NRF_CMD_NOP             0xFF

/* Bits du registre STATUS */
#define NRF_STATUS_RX_DR        (1 << 6)   /* Donnée reçue */
#define NRF_STATUS_TX_DS        (1 << 5)   /* Transmission réussie */
#define NRF_STATUS_MAX_RT       (1 << 4)   /* Nombre max de retransmissions atteint */

/* ============================================================
 *  Configuration RF — modifier selon le projet
 * ============================================================ */
#define NRF_PAYLOAD_SIZE    32              /* Taille du payload en octets (1-32) */
#define NRF_RF_CHANNEL      76              /* Canal RF (0-125) */

/* Adresse TX/RX — doit être identique sur l'émetteur ET le récepteur */
#define NRF_ADDRESS         { 0xE7, 0xE7, 0xE7, 0xE7, 0xE7 }

/* ============================================================
 *  Code de retour
 * ============================================================ */
typedef enum {
    NRF_OK       = 0,   /* Transmission réussie */
    NRF_ERR_MAX_RT,     /* Échec : retransmissions épuisées */
    NRF_ERR_TIMEOUT     /* Échec : timeout SPI */
} NRF_Status;

/* ============================================================
 *  Prototypes publics
 * ============================================================ */

/**
 * @brief  Initialise le module nRF24L01+ en mode émetteur.
 * @param  hspi  Pointeur vers le handle SPI configuré dans CubeMX.
 */
void NRF24_TX_Init(SPI_HandleTypeDef *hspi);

/**
 * @brief  Écrit un registre 8 bits.
 * @param  reg    Adresse du registre.
 * @param  value  Valeur à écrire.
 */
void NRF24_TX_WriteRegister(uint8_t reg, uint8_t value);

/**
 * @brief  Lit un registre 8 bits.
 * @param  reg  Adresse du registre.
 * @return Valeur lue.
 */
uint8_t NRF24_TX_ReadRegister(uint8_t reg);

/**
 * @brief  Écrit plusieurs octets consécutifs dans un registre (ex. adresse).
 * @param  reg   Adresse du registre.
 * @param  data  Pointeur vers les données.
 * @param  size  Nombre d'octets.
 */
void NRF24_TX_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t size);

/**
 * @brief  Envoie une commande SPI sans donnée.
 * @param  cmd  Commande à envoyer.
 */
void NRF24_TX_SendCommand(uint8_t cmd);

/**
 * @brief  Transmet un paquet et vérifie le résultat.
 * @param  data  Pointeur vers les données à envoyer.
 * @param  size  Taille en octets (max NRF_PAYLOAD_SIZE).
 * @return NRF_OK si succès, NRF_ERR_MAX_RT si échec.
 */
NRF_Status NRF24_TX_Transmit(uint8_t *data, uint8_t size);

/**
 * @brief  Lit et retourne le registre STATUS.
 * @return Valeur du registre STATUS.
 */
uint8_t NRF24_TX_GetStatus(void);

#endif /* NRF24_TX_H */
