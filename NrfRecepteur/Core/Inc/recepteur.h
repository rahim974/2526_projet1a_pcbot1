/*
 * recepteur.h
 *
 *  Created on: May 28, 2026
 *      Author: khady
 */

#ifndef INC_RECEPTEUR_H_
#define INC_RECEPTEUR_H_


/**
 * @file nrf24_receiver.h
 * @brief Bibliothèque C pour la gestion d'un module nRF24L01+ en mode récepteur.
 */
#include <stdint.h>
#include <stdbool.h>

/* --- 1. Définition des Commandes Radio (Doit être identique à l'émetteur) --- */
typedef enum {
    CMD_LED_OFF   = 0x00,
    CMD_LED_ON    = 0x01,
    CMD_LED_BLINK = 0x02
} CommandesRadio_t;

/* --- 2. Commandes et Registres Internes du nRF24L01 --- */
#define NRF24_CMD_R_REGISTER     0x00
#define NRF24_CMD_W_REGISTER     0x20
#define NRF24_CMD_R_RX_PAYLOAD   0x61
#define NRF24_CMD_FLUSH_RX       0xE2

#define NRF24_REG_CONFIG         0x00
#define NRF24_REG_EN_AA          0x01
#define NRF24_REG_EN_RXADDR      0x02
#define NRF24_REG_RF_CH          0x05
#define NRF24_REG_RF_SETUP       0x06
#define NRF24_REG_STATUS         0x07
#define NRF24_REG_RX_ADDR_P0     0x0A
#define NRF24_REG_RX_PW_P0       0x11
#define NRF24_REG_FIFO_STATUS    0x17

/* ========================================================================
   3. FONCTIONS D'APPEL SIMPLES POUR LE MAIN DU RÉCEPTEUR
   ======================================================================== */

/**
 * @brief Initialise et configure le nRF24L01 en mode réception continue.
 */
void Appeler_Initialisation_Recepteur_nRF24(void);

/**
 * @brief Vérifie si un paquet est disponible et le lit.
 * @param donnees_recues Pointeur vers la zone mémoire où stocker la commande.
 * @param taille Taille maximale attendue (1 octet pour notre enum).
 * @return true si une nouvelle commande valide a été reçue, false sinon.
 */
bool Appeler_Lecture_nRF24(void* donnees_recues, uint8_t taille);


/* ========================================================================
   4. ABSTRACTION MATÉRIELLE (À lier avec vos fonctions HAL de la carte réceptrice)
   ======================================================================== */
extern void nRF24_SPI_Init(void);
extern uint8_t nRF24_SPI_TransmitReceive(uint8_t data);
extern void nRF24_SetCE(uint8_t state);
extern void nRF24_SetCSN(uint8_t state);
extern void nRF24_DelayMs(uint32_t ms);

#endif /* INC_RECEPTEUR_H_ */
