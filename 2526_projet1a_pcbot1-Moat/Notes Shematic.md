### 1. Communication et Capteurs

- **nRF24L01+ :** Connecteur mâle `Connector_Generic:Conn_02x04_Odd_Even`. 
- **Capteur TOF (VL53L0X ou VL53L1X) :** Connecteur `Conn_01x04` 
- **Accéléromètre (LSM6DSOX) :**  `Sensor_Motion:LSM6DSOX`  https://www.snapeda.com/parts/LSM6DSOX/STMicroelectronics/view-part/?ref=search&t=LSM6DSO 

### 2. Motorisation (Puissance)

- **Driver Moteur (DRV8411A) :**  `Interface_Expansion:DRV8411APWPR` https://www.snapeda.com/parts/DRV8411APWPR/Texas%20Instruments/view-part/?ref=dk&t=DRV8411APWPR&con_ref=None&ab_test_case=b
- **Connecteurs Moteurs :** Deux connecteurs `Connector Screw_Terminal_01x02` pour brancher physiquement les fils des moteurs.

### 3. Gestion de l'Énergie (Batterie 18650)

- **Chargeur de Batterie (BQ25896) :**  `Power_Management:BQ25896` https://www.snapeda.com/parts/BQ25896/Texas%20Instruments/view-part/?ref=search&t=BQ25896&ab_test_case=b
- **Inductance de puissance :** Bobine (ex : `Inductor_SMD:L_6.3x6.3mm`) d'environ 1μH à 2.2μH ( à vérifier dans la datasheet ).
- **Connecteur Batterie :** Connecteur `Connector_JST:JST_PH_S2B-PH-K_1x02` https://www.snapeda.com/parts/B4B-PH-K%28LF%29%28SN%29/JST%20Sales%20America%20Inc./view-part/?ref=search&t=JST_PH_S2B-PH-K_1x02&ab_test_case=b

### 4. Composants Passifs et Interface

- **Résistances de Pull-up I2C :** Ajouter deux résistances `R` de 2.2kΩ ou 4.7kΩ connectées entre les lignes SDA/SCL et le +3.3V.
- **Bouton d'alimentation :** Un interrupteur `Switch:SW_Push` ou un commutateur à glissière `SW_SPDT`.
- **Connecteur de programmation :** Un `Connector:Conn_01x05_Pin` pour brancher le ST-LINK (VCC, SWDIO, SWCLK, NRST, GND).



