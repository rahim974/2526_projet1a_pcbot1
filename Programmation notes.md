# Fonctionnalités à coder

### 1. Communication et Capteurs (I2C)

- **Driver Capteur TOF (Distance) :** Implémenter le protocole I2C pour lire les distances et repérer les obstacles devant le robot.
- **Driver Accéléromètre (LSM6DSOX) :** Configurer les registres pour récupérer l'accélération et l'inclinaison (angle) afin de connaître la position du robot.
- **Gestion de la Batterie (BQ25896) :** Lire l'état de charge de la batterie Samsung 18650 via I2C pour savoir quand retourner à la prise.

### 2. Propulsion et Mouvement (PWM)

- **Contrôle du DRV8411A :** Générer des signaux PWM pour contrôler la vitesse des moteurs à courant continu.
- **Logique de Direction :** Coder les fonctions de base : `avancer()`, `reculer()`, `tourner_gauche()`, `tourner_droite()` et `stop()`.

### 3. Communication Radio (SPI)

- **Driver nRF24L01 :** Configurer l'interface SPI pour envoyer et recevoir des données en continu.
- **Protocole Station de Base :** Créer une structure de message pour envoyer les coordonnées (angle, distance) à la station de base.

### 4. Interface et Feedback

- **Gestion des LEDs :** Coder les états visuels (ex: clignotement en cas de batterie faible ou obstacle proche).
- **Lecture du Bouton :** Gérer l'anti-rebond (debouncing) pour le bouton d'alimentation ou de démarrage.

### 5. Algorithme Principal (Boucle de contrôle)

- **Évitement d'obstacles :** Fusionner les données du TOF et des moteurs pour s'arrêter avant une collision.
- **Calcul de position :** Utiliser les données de l'accéléromètre pour estimer le déplacement du robot.

# Fonctions déjà existantes à utiliser 

## 1. Pour les Moteurs (Timer & PWM)

Le fichier de référence est `stm32g4xx_hal_tim.h`.

- **`HAL_TIM_PWM_Start(&htimx, TIM_CHANNEL_y)`** :
  - **Rôle** : Allume physiquement le signal PWM sur la broche du moteur.
  - **Utilisation** : À appeler une seule fois dans l'initialisation pour chaque moteur.
- **`__HAL_TIM_SET_COMPARE(&htimx, TIM_CHANNEL_y, valeur)`** :
  - **Rôle** : C'est la fonction la plus importante. Elle change la **vitesse** en modifiant le registre de comparaison (CCR).
  - **Note** : Ce n'est pas une fonction classique mais une "macro" ultra-rapide.

## 2. Pour les Capteurs (I2C)

Le fichier de référence est `stm32g4xx_hal_i2c.h`. Pour ton capteur TOF et le LSM6DSOX, on utilise souvent le mode "Mem" (Mémoire) car ces capteurs fonctionnent avec des registres internes.

- **`HAL_I2C_Mem_Write(...)`** :
  - **Rôle** : Écrit une configuration dans le capteur (ex: allumer le laser du TOF).
- **`HAL_I2C_Mem_Read(...)`** :
  - **Rôle** : Lit une donnée précise (ex: lire les 2 octets de la distance mesurée).

| **Paramètre** | **Signification**    | **Exemple (LSM6DSOX)**                  |
| ------------- | -------------------- | --------------------------------------- |
| `&hi2c1`      | Le bus utilisé       | I2C numéro 1                            |
| `DevAddress`  | Adresse du composant | `0x6B << 1` (Toujours décaler d'un bit) |
| `MemAddress`  | Registre interne     | `0x22` (Donnée Gyro)                    |
| `pData`       | Variable de stockage | Tableau d'octets                        |

## 3. Pour la Radio nRF24L01 (SPI)

Le fichier de référence est `stm32g4xx_hal_spi.h`.

- **`HAL_SPI_Transmit(&hspix, pData, Size, Timeout)`** :
  - **Rôle** : Envoie un paquet de données vers la station de base via le module radio.
- **`HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState)`** :
  - **Rôle** : Très important pour le SPI ! Sert à activer/désactiver la broche **CSN** (Chip Select) pour dire au nRF24 "je te parle à toi".



# 1 ) Initialisation des Moteurs

```c
// Activation des PWM pour le moteur Gauche (ex: TIM1 et TIM2)
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); 
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); 

// Activation des PWM pour le moteur Droit (ex: TIM3 et TIM4)
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); 
HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
```

# 2) Avancer, Reculer et Pivoter

#### Avancer tout droit

```c
void robot_avancer(uint16_t vitesse) {
    // Moteur Gauche : TIM1 actif, TIM2 à l'arrêt
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, vitesse);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);

    // Moteur Droit : TIM3 actif, TIM4 à l'arrêt
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, vitesse);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
}
```

#### Pivoter sur place (Rotation)

```c
void robot_pivoter_gauche(uint16_t vitesse) {
    // Roue Gauche recule
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, vitesse);

    // Roue Droite avance
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, vitesse);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
}
```

# 3) Boucle principale

```c
while (1) {
    uint16_t distance;
    // Lecture via I2C (fonction simplifiée pour l'exemple)
    distance = TOF_GetDistance(); 

    if (distance > 200) {
        robot_avancer(500); // On roule à 50%
    } else {
        robot_pivoter_gauche(400); // On tourne pour éviter le mur
        HAL_Delay(500); // Petit délai pour effectuer la rotation
    }
    
    // Envoi de la télémétrie vers la station de base via nRF24L01
    NRF24_SendData(distance);
}
```

# 4) Fonction GetDistance()

```c
#define TOF_ADDR (0x29 << 1)  // Adresse par défaut du VL53L0X décalée pour la HAL
#define RESULT_RANGE_STATUS 0x14 // Registre où se trouve la distance

uint16_t TOF_GetDistance(void) {
    uint8_t data[2];
    uint16_t distance = 0;

    // On lit 2 octets à partir du registre 0x14
    // &hi2c1 est la structure générée par CubeIDE pour ton bus I2C
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1, TOF_ADDR, RESULT_RANGE_STATUS,I2C_MEMADD_SIZE_8BIT, data, 2, 100);

    if (status == HAL_OK) {
        // Le TOF renvoie [MSB, LSB]. On les assemble.
        distance = (data[0] << 8) | data[1];
    } else {
        // Gestion d'erreur (optionnel : retourner une valeur spécifique)
        distance = 8190; // Valeur "hors de portée" par défaut
    }

    return distance;
}
```

