/*
 * emetteur.c
 *
 *  Created on: May 28, 2026
 *      Author: khady
 */

/**
 * @file emetteur.c
 * @brief Implémentation des fonctions de contrôle pour le nRF24L01+ en mode émetteur.
 */

#include "emetteur.h"

// Adresse matérielle unique du canal (5 octets) codée en dur ici pour vider le main
static const uint8_t ADRESSE_RESEAU[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

/* ========================================================================
   1. FONCTIONS LOCALES PRIVÉES (ACCÈS AUX REGISTRES DU nRF24L01)
   ======================================================================== */

/**
 * @brief Écrit un octet dans un registre spécifique du nRF24L01.
 */
static void nRF24_WriteRegister(uint8_t reg, uint8_t value) {
    nRF24_SetCSN(0);
    nRF24_SPI_TransmitReceive(NRF24_CMD_W_REGISTER | reg);
    nRF24_SPI_TransmitReceive(value);
    nRF24_SetCSN(1);
}

/**
 * @brief Écrit un tableau d'octets (tampon) dans un registre (ex: pour l'adresse).
 */
static void nRF24_WriteRegisterBuf(uint8_t reg, const uint8_t *buf, uint8_t len) {
    nRF24_SetCSN(0);
    nRF24_SPI_TransmitReceive(NRF24_CMD_W_REGISTER | reg);
    for (uint8_t i = 0; i < len; i++) {
        nRF24_SPI_TransmitReceive(buf[i]);
    }
    nRF24_SetCSN(1);
}



   //IMPLÉMENTATION DE L'API PUBLIQUE


void Appeler_Initialisation_nRF24(void) {
    // Initialisation du bus SPI matériel (géré par votre MCU)
    nRF24_SPI_Init();

    // États initiaux indispensables des broches de contrôle
    nRF24_SetCE(0);
    nRF24_SetCSN(1);

    // Attendre que le module se réveille électriquement (Power-on Reset)
    nRF24_DelayMs(10);

    // Activation du protocole Enhanced ShockBurst (Auto-ACK)
    nRF24_WriteRegister(NRF24_REG_EN_AA, 0x01); // Activer l'ACK auto sur le canal 0

    //  Configuration des retransmissions (Délai: 500µs, Compteur: 10 essais max)
    nRF24_WriteRegister(NRF24_REG_SETUP_RETR, 0x1A);

    //  Configuration de la fréquence (Canal 76 = 2476 MHz pour éviter le Wi-Fi classique)
    nRF24_WriteRegister(NRF24_REG_RF_CH, 76);

    //  Débit d'air et puissance (1 Mbps, Puissance Max 0dBm pour une portée optimale)
    nRF24_WriteRegister(NRF24_REG_RF_SETUP, 0x06);

    //  Configuration de l'adresse réseau d'émission
    nRF24_WriteRegisterBuf(NRF24_REG_TX_ADDR, ADRESSE_RESEAU, 5);
    // Pour que l'Auto-ACK fonctionne, le canal de réception 0 doit avoir la même adresse
    nRF24_WriteRegisterBuf(0x0A, ADRESSE_RESEAU, 5);

    //  Configuration du registre principal CONFIG
    // Bit 1 = 1 (Power Up), Bit 0 = 0 (Mode Émetteur CRC 2 octets activé)
    nRF24_WriteRegister(NRF24_REG_CONFIG, 0x0E);

    // Attente de la stabilisation du quartz interne du nRF24
    nRF24_DelayMs(2);
}

bool Appeler_Envoi_nRF24(const void* donnees, uint8_t taille) {
    // Protection : La taille maximale d'un paquet nRF24L01 est de 32 octets
    if (taille > 32) {
        taille = 32;
    }

    // Conversion du pointeur universel void* en pointeur d'octets exploitables (uint8_t*)
    const uint8_t* ptr_octets = (const uint8_t*)donnees;

    //  Vider le tampon d'émission (TX FIFO) pour éliminer d'anciens résidus
    nRF24_SetCSN(0);
    nRF24_SPI_TransmitReceive(NRF24_CMD_FLUSH_TX);
    nRF24_SetCSN(1);

    //  Charger la commande d'écriture et envoyer la donnée octet par octet
    nRF24_SetCSN(0);
    nRF24_SPI_TransmitReceive(NRF24_CMD_W_TX_PAYLOAD);
    for (uint8_t i = 0; i < taille; i++) {
        nRF24_SPI_TransmitReceive(ptr_octets[i]);
    }
    nRF24_SetCSN(1);

    // Envoyer une impulsion sur la broche CE (min 10µs) pour déclencher l'onde radio
    nRF24_SetCE(1);
    nRF24_DelayMs(1); // Maintien court (1ms est largement suffisant)
    nRF24_SetCE(0);

    // Attente de la fin de la transmission (Scrutation du registre STATUS)
    uint8_t status = 0;
    uint32_t timeout = 2000; // Sécurité anti-blocage (2000 boucles max)

    while (timeout > 0) {
        // Lecture rapide du registre de statut
        nRF24_SetCSN(0);
        status = nRF24_SPI_TransmitReceive(NRF24_CMD_R_REGISTER | NRF24_REG_STATUS);
        nRF24_SetCSN(1);

        // Bit 5 (TX_DS)  : Données envoyées ET accusé de réception (ACK) bien reçu !
        // Bit 4 (MAX_RT) : Nombre max de retransmissions atteint (Le récepteur n'a pas répondu)
        if ((status & (1 << 5)) || (status & (1 << 4))) {
            break; // On sort de la boucle dès qu'un des deux événements arrive
        }

        nRF24_DelayMs(1);
        timeout--;
    }

    // Réinitialiser les drapeaux (flags) d'interruption dans le nRF24 pour le prochain envoi
    nRF24_WriteRegister(NRF24_REG_STATUS, 0x70);

    //  Analyse du résultat de l'envoi
    if (status & (1 << 5)) {
        return true;  // Succès : La commande ou donnée est bien arrivée à destination
    }

    return false; // Échec : Timeout ou récepteur éteint/hors de portée
}

