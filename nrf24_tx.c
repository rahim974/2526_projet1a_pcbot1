#include "nrf24_tx.h"

/* ============================================================
 *  Variable privée
 * ============================================================ */
static SPI_HandleTypeDef *nrf_tx_hspi;

/* ============================================================
 *  Utilitaire : délai microseconde
 *
 *  Utilise le compteur DWT (Data Watchpoint and Trace) présent
 *  sur tous les Cortex-M3/M4/M7. À activer une seule fois avec
 *  NRF24_TX_Init (déjà fait ci-dessous).
 *
 *  Alternative : configurer un timer hardware en mode us.
 * ============================================================ */
static void delay_us(uint32_t us)
{
    uint32_t cycles = (SystemCoreClock / 1000000UL) * us;
    uint32_t start  = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles);
}

/* ============================================================
 *  Implémentation
 * ============================================================ */

void NRF24_TX_Init(SPI_HandleTypeDef *hspi)
{
    nrf_tx_hspi = hspi;

    /* Activer le compteur DWT pour les délais µs */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    /* CE bas au démarrage (pas d'émission) */
    HAL_GPIO_WritePin(NRF_CE_GPIO_Port,  NRF_CE_Pin,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);

    HAL_Delay(10); /* Attente de démarrage du module (Power-on reset) */

    /* ---- 1. Configuration générale ---- */
    /* CONFIG : PWR_UP=1, PRIM_RX=0 (mode TX), CRC 2 octets activé */
    NRF24_TX_WriteRegister(NRF_REG_CONFIG, 0x0E);

    /* Auto-Acknowledgment activé sur Pipe 0 (obligatoire pour l'Auto-Ack) */
    NRF24_TX_WriteRegister(NRF_REG_EN_AA, 0x01);

    /* Adresses sur 5 octets */
    NRF24_TX_WriteRegister(NRF_REG_SETUP_AW, 0x03);

    /* Retransmissions : délai 500 µs, max 10 essais */
    NRF24_TX_WriteRegister(NRF_REG_SETUP_RETR, 0x2A);

    /* ---- 2. Canal & débit RF ---- */
    NRF24_TX_WriteRegister(NRF_REG_RF_CH,    NRF_RF_CHANNEL);
    NRF24_TX_WriteRegister(NRF_REG_RF_SETUP, 0x06); /* 1 Mbps, 0 dBm */

    /* ---- 3. Adresses ---- */
    uint8_t address[] = NRF_ADDRESS;

    /* Adresse TX */
    NRF24_TX_WriteRegisterMulti(NRF_REG_TX_ADDR, address, 5);

    /* Adresse Pipe 0 = adresse TX (requis pour recevoir les ACK) */
    NRF24_TX_WriteRegisterMulti(NRF_REG_RX_ADDR_P0, address, 5);

    /* Taille du payload sur Pipe 0 */
    NRF24_TX_WriteRegister(NRF_REG_RX_PW_P0, NRF_PAYLOAD_SIZE);

    /* ---- 4. Nettoyage ---- */
    NRF24_TX_SendCommand(NRF_CMD_FLUSH_TX);
    NRF24_TX_SendCommand(NRF_CMD_FLUSH_RX);

    /* Effacer les drapeaux de statut (MAX_RT, TX_DS, RX_DR) */
    NRF24_TX_WriteRegister(NRF_REG_STATUS, 0x70);

    HAL_Delay(2); /* Temps de stabilisation après PWR_UP */
}

/* ---------------------------------------------------------- */

void NRF24_TX_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t buf[2];
    buf[0] = (reg & 0x1F) | NRF_CMD_W_REGISTER;
    buf[1] = value;

    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf_tx_hspi, buf, 2, 100);
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);
}

/* ---------------------------------------------------------- */

uint8_t NRF24_TX_ReadRegister(uint8_t reg)
{
    uint8_t tx[2] = { reg & 0x1F, NRF_CMD_NOP };
    uint8_t rx[2] = { 0x00, 0x00 };

    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(nrf_tx_hspi, tx, rx, 2, 100);
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);

    return rx[1]; /* rx[0] = STATUS, rx[1] = valeur du registre */
}

/* ---------------------------------------------------------- */

void NRF24_TX_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t size)
{
    uint8_t cmd = (reg & 0x1F) | NRF_CMD_W_REGISTER;

    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf_tx_hspi, &cmd,  1,    100);
    HAL_SPI_Transmit(nrf_tx_hspi, data,  size, 100);
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);
}

/* ---------------------------------------------------------- */

void NRF24_TX_SendCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf_tx_hspi, &cmd, 1, 100);
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);
}

/* ---------------------------------------------------------- */

uint8_t NRF24_TX_GetStatus(void)
{
    uint8_t cmd    = NRF_CMD_NOP;
    uint8_t status = 0x00;

    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(nrf_tx_hspi, &cmd, &status, 1, 100);
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);

    return status;
}

/* ---------------------------------------------------------- */

NRF_Status NRF24_TX_Transmit(uint8_t *data, uint8_t size)
{
    uint8_t cmd    = NRF_CMD_W_TX_PAYLOAD;
    uint8_t status = 0x00;

    /* ---- 1. Écrire le payload dans le FIFO TX ---- */
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf_tx_hspi, &cmd, 1,    100);
    HAL_SPI_Transmit(nrf_tx_hspi, data, size, 100);
    HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);

    /* ---- 2. Impulsion CE ≥ 10 µs pour déclencher l'envoi ---- */
    HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_SET);
    delay_us(15);   /* 15 µs : largement au-dessus du minimum de 10 µs */
    HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET);

    /* ---- 3. Attendre la fin de la transmission (TX_DS ou MAX_RT) ---- */
    HAL_Delay(2); /* Délai conservateur : inclut retransmissions éventuelles */

    status = NRF24_TX_GetStatus();

    /* ---- 4. Gérer le résultat ---- */
    if (status & NRF_STATUS_MAX_RT)
    {
        /* Échec : vider le FIFO TX et réinitialiser le drapeau MAX_RT */
        NRF24_TX_SendCommand(NRF_CMD_FLUSH_TX);
        NRF24_TX_WriteRegister(NRF_REG_STATUS, NRF_STATUS_MAX_RT);
        return NRF_ERR_MAX_RT;
    }

    if (status & NRF_STATUS_TX_DS)
    {
        /* Succès : effacer le drapeau TX_DS */
        NRF24_TX_WriteRegister(NRF_REG_STATUS, NRF_STATUS_TX_DS);
        return NRF_OK;
    }

    /* Cas improbable : ni TX_DS ni MAX_RT → timeout */
    return NRF_ERR_TIMEOUT;
}
