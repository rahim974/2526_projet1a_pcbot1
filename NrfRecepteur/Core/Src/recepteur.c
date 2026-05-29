/*
 * recepteur.c
 *
 *  Created on: May 28, 2026
 *      Author: khady
 */


/**
 * @file nrf24_receiver.c
 * @brief Implémentation du mode récepteur pour le nRF24L01+.
 */

#include "recepteur.h"

// L'adresse doit être EXACTEMENT la même que celle de l'émetteur
static const uint8_t ADRESSE_RESEAU[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

/* --- Fonctions locales privées d'accès aux registres --- */
static void nRF24_WriteRegister(uint8_t reg, uint8_t value) {
    nRF24_SetCSN(0);
    nRF24_SPI_TransmitReceive(NRF24_CMD_W_REGISTER | reg);
    nRF24_SPI_TransmitReceive(value);
    nRF24_SetCSN(1);
}

static void nRF24_WriteRegisterBuf(uint8_t reg, const uint8_t *buf, uint8_t len) {
    nRF24_SetCSN(0);
    nRF24_SPI_TransmitReceive(NRF24_CMD_W_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
        nRF24_SPI_TransmitReceive(buf[i]);
    }
    nRF24_SetCSN(1);
}


//   REMANIEMENT DE L'API POUR LE RÉCEPTEUR


void Appeler_Initialisation_Recepteur_nRF24(void) {
    nRF24_SPI_Init();
    nRF24_SetCE(0);
    nRF24_SetCSN(1);
    nRF24_DelayMs(10);

    // 1. Activation de l'Auto-ACK sur le canal (Pipe) 0
    nRF24_WriteRegister(NRF24_REG_EN_AA, 0x01);

    // 2. Activation de la réception sur le Pipe 0
    nRF24_WriteRegister(NRF24_REG_EN_RXADDR, 0x01);

    // 3. Même fréquence (Canal 76)
    nRF24_WriteRegister(NRF24_REG_RF_CH, 76);

    // 4. Même configuration radio (1 Mbps, Puissance max)
    nRF24_WriteRegister(NRF24_REG_RF_SETUP, 0x06);

    // 5. Configuration de la taille fixe du paquet attendu sur le Pipe 0 (1 octet pour l'enum)
    nRF24_WriteRegister(NRF24_REG_RX_PW_P0, 1);

    // 6. Attribution de l'adresse de réception sur le Pipe 0
    nRF24_WriteRegisterBuf(NRF24_REG_RX_ADDR_P0, ADRESSE_RESEAU, 5);

    // 7. Configuration du registre principal CONFIG
    // Bit 1 = 1 (Power Up), Bit 0 = 1 (Mode RÉCEPTEUR), CRC 2 octets activé
    nRF24_WriteRegister(NRF24_REG_CONFIG, 0x0F);

    // Étape cruciale en réception : Mettre CE à 1 pour écouter l'air en continu
    nRF24_SetCE(1);
    nRF24_DelayMs(2);
}

bool Appeler_Lecture_nRF24(void* donnees_recues, uint8_t taille) {
    uint8_t status = 0;
    uint8_t fifo_status = 0;

    // Lire le registre de statut pour voir si des données sont arrivées (Bit 6 = RX_DR)
    nRF24_SetCSN(0);
    status = nRF24_SPI_TransmitReceive(NRF24_CMD_R_REGISTER | NRF24_REG_STATUS);
    nRF24_SetCSN(1);

    // Si le bit RX_DR (Data Ready) est levé
    if (status & (1 << 6)) {
        uint8_t* ptr_octets = (uint8_t*)donnees_recues;

        // Lire la charge utile (Payload) reçue
        nRF24_SetCSN(0);
        nRF24_SPI_TransmitReceive(NRF24_CMD_R_RX_PAYLOAD);
        for (uint8_t i = 0; i < taille; i++) {
            ptr_octets[i] = nRF24_SPI_TransmitReceive(0x00); // Récupération des octets
        }
        nRF24_SetCSN(1);

        // Effacer le drapeau d'interruption RX_DR en écrivant un 1 dessus
        nRF24_WriteRegister(NRF24_REG_STATUS, (1 << 6));
        return true; // Données lues avec succès !
    }

    // Sécurité : Si le buffer RX déborde malgré tout, on le nettoie
    nRF24_SetCSN(0);
    fifo_status = nRF24_SPI_TransmitReceive(NRF24_CMD_R_REGISTER | NRF24_REG_FIFO_STATUS);
    nRF24_SetCSN(1);

    if (fifo_status & (1 << 4)) { // RX_EMPTY == 0 (donc plein mais flag bloqué)
        nRF24_SetCSN(0);
        nRF24_SPI_TransmitReceive(NRF24_CMD_FLUSH_RX);
        nRF24_SetCSN(1);
    }

    return false; // Pas de nouveau message
}
