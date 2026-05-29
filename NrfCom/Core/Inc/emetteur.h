/*
 * emetteur.h
 *
 *  Created on: May 28, 2026
 *      Author: khady
 */

#ifndef INC_EMETTEUR_H_
#define INC_EMETTEUR_H_

/**
 * @file nrf24_transmitter.h
 * @brief Bibliothèque C pour la gestion d'un module nRF24L01+ en mode émetteur.
 * Structure épurée pour un appel minimaliste depuis le main.
 */
#include <stdint.h>
#include <stdbool.h>

/* --- 1. Définition des Commandes Radio (1 octet par commande) --- */
typedef enum {
    CMD_LED_OFF   = 0x00,  // Éteindre la LED
    CMD_LED_ON    = 0x01,  // Allumer la LED
    CMD_LED_BLINK = 0x02   // Faire clignoter la LED
} CommandesRadio_t;

/* --- 2. Configuration des broches par défaut (À adapter selon votre MCU) --- */
#define NRF24_CE_PIN   3
#define NRF24_CSN_PIN 4

/* --- 3. Commandes et Registres Internes du nRF24L01 --- */
#define NRF24_CMD_R_REGISTER    0x00
#define NRF24_CMD_W_REGISTER    0x20
#define NRF24_CMD_W_TX_PAYLOAD  0xA0
#define NRF24_CMD_FLUSH_TX      0xE1

#define NRF24_REG_CONFIG      0x00
#define NRF24_REG_EN_AA       0x01
#define NRF24_REG_SETUP_RETR  0x04
#define NRF24_REG_RF_CH       0x05
#define NRF24_REG_RF_SETUP    0x06
#define NRF24_REG_STATUS      0x07
#define NRF24_REG_TX_ADDR     0x10

/* ========================================================================
   4. FONCTIONS D'APPEL SIMPLES (LES SEULES À UTILISER DANS LE MAIN)
   ======================================================================== */

/**
 * @brief Initialise et configure entièrement le module nRF24L01 en émetteur.
 * Cette fonction masque toute la complexité des registres.
 */
void Appeler_Initialisation_nRF24(void);

/**
 * @brief Envoie un paquet de données générique (ex: une commande LED, une variable...)
 * @param donnees Pointeur universel vers la donnée à envoyer.
 * @param taille Taille de la donnée en octets (maximum 32 octets).
 * @return true si le récepteur a bien reçu le message (ACK reçu), false sinon.
 */
bool Appeler_Envoi_nRF24(const void* donnees, uint8_t taille);


/* ========================================================================
   5. ABSTRACTION MATÉRIELLE (À implémenter selon votre STM32, AVR, PIC...)
   ======================================================================== */
extern void nRF24_SPI_Init(void);
extern uint8_t nRF24_SPI_TransmitReceive(uint8_t data);
extern void nRF24_SetCE(uint8_t state);
extern void nRF24_SetCSN(uint8_t state);
extern void nRF24_DelayMs(uint32_t ms);

void EnvoieCommande();

#endif /* INC_EMETTEUR_H_ */
