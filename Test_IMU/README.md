# Test_IMU — Essai LSM6DSOX sur STM32L021K4TX

Dossier d'essai pour valider l'accéléromètre/gyroscope **LSM6DSOX** avant intégration dans le projet final (STM32G431KBT6).

## Arborescence

```
Test_IMU/
├── Core/
│   └── Src/
│       └── main.c                    ← code d'essai (à copier dans votre projet CubeIDE)
├── Drivers/
│   └── LSM6DSOX/
│       ├── lsm6dsox_reg.h            ← API du capteur (style ST PID)
│       ├── lsm6dsox_reg.c            ← implémentation
│       ├── lsm6dsox_platform.h       ← couche d'adaptation HAL
│       └── lsm6dsox_platform.c
└── README.md
```

La séparation **core** (API portable) + **platform** (adaptation MCU) reprend exactement la structure du driver VL53L0X. Le `lsm6dsox_reg.{c,h}` est compatible avec l'[API officielle ST](https://github.com/STMicroelectronics/lsm6dsox-pid) — même prototypes, mêmes enums, même struct `stmdev_ctx_t`. Le jour où vous passez sur le STM32G431 vous pourrez remplacer ces deux fichiers par les fichiers officiels sans toucher au reste.

## Étapes d'intégration dans STM32CubeIDE

1. **Créer un projet** pour la cible `STM32L021K4Tx` (File → New → STM32 Project)
2. **Configurer CubeMX** :
   - `I2C1` en mode **I2C** (PB6=SCL, PB7=SDA — à confirmer selon votre câblage)
   - `USART2` en mode **Asynchronous**, 115200 8N1 (pour les logs)
   - Horloge système : HSI 16 MHz (par défaut)
   - Generate Code
3. **Ajouter les fichiers du driver** :
   - Copier `Drivers/LSM6DSOX/` dans le dossier `Drivers/` de votre projet CubeIDE
   - Clic droit sur le projet → Properties → C/C++ Build → Settings → MCU GCC Compiler → Include paths → ajouter `../Drivers/LSM6DSOX`
   - Vérifier que les `.c` du dossier sont bien compilés (clic droit dossier → Add/Remove Include Path / Source Folder)
4. **Remplacer le `main.c` généré** par celui de ce dossier (ou copier les sections `USER CODE BEGIN/END`)

## Câblage minimal

| Signal | STM32L021K4T (LQFP32) | LSM6DSOX |
|---|---|---|
| 3,3 V | VDD | VDD |
| GND | GND | GND, SDO/SA0 (pull-down → SA0=0) |
| SCL | PB6 (I2C1_SCL) | SCL |
| SDA | PB7 (I2C1_SDA) | SDA |
| UART TX | PA2 (USART2_TX) | → vers PC (USB-UART) |

Ne pas oublier les **pull-ups I2C 4,7 kΩ** sur SDA et SCL vers +3,3 V.

## Lecture attendue dans le terminal série (115200 bauds)

```
===== Essai LSM6DSOX sur STM32L021K4TX =====
[OK] Capteur detecte (SA0=0)
WHO_AM_I = 0x6C (attendu 0x6C)
[OK] Capteur configure — debut des mesures

ACC[mg] X=  +12 Y=  -34 Z= +998  |  GYR[mdps] X=  +123 Y=  -45 Z=  +67  |  T=24.5 C
ACC[mg] X=  +14 Y=  -32 Z= +999  |  GYR[mdps] X=  +120 Y=  -48 Z=  +71  |  T=24.5 C
...
```

**Z ≈ +1000 mg** au repos sur une surface horizontale (gravité). Inclinez le capteur : X et Y bougent, la norme reste ≈ 1g.

## Migration vers STM32G431KBT6

Quand vous voudrez déployer sur la carte finale :
1. Remplacez `#include "stm32l0xx_hal.h"` par `#include "stm32g4xx_hal.h"` dans `lsm6dsox_platform.h`
2. *(optionnel)* Remplacez `lsm6dsox_reg.{c,h}` par les fichiers officiels du dépôt [STMicroelectronics/lsm6dsox-pid](https://github.com/STMicroelectronics/lsm6dsox-pid) — le STM32G431 a 128 KB de flash, vous pouvez vous le permettre
3. Le `main.c` n'a besoin d'aucune modification : l'API est strictement la même

## API exposée

Identification :
- `lsm6dsox_device_id_get(ctx, &whoami)` — lit WHO_AM_I (doit valoir 0x6C)
- `lsm6dsox_reset_set/get(ctx, val)` — reset logiciel

Configuration :
- `lsm6dsox_block_data_update_set(ctx, 1)` — lecture cohérente MSB/LSB
- `lsm6dsox_auto_increment_set(ctx, 1)` — auto-incrément en lecture multiple
- `lsm6dsox_xl_full_scale_set(ctx, LSM6DSOX_2g|_4g|_8g|_16g)`
- `lsm6dsox_xl_data_rate_set(ctx, LSM6DSOX_XL_ODR_104Hz)` (et autres ODR)
- `lsm6dsox_gy_full_scale_set(ctx, LSM6DSOX_125dps|_250dps|...)`
- `lsm6dsox_gy_data_rate_set(ctx, LSM6DSOX_GY_ODR_104Hz)`

Lecture :
- `lsm6dsox_xl_flag_data_ready_get` / `lsm6dsox_gy_flag_data_ready_get`
- `lsm6dsox_acceleration_raw_get(ctx, int16_t[3])`
- `lsm6dsox_angular_rate_raw_get(ctx, int16_t[3])`
- `lsm6dsox_temperature_raw_get(ctx, &int16)`

Conversions :
- `lsm6dsox_from_fs2_to_mg`, `_fs4_to_mg`, `_fs8_to_mg`, `_fs16_to_mg`
- `lsm6dsox_from_fs125_to_mdps`, `_fs250_to_mdps`, `_fs500_to_mdps`, `_fs1000_to_mdps`, `_fs2000_to_mdps`
- `lsm6dsox_from_lsb_to_celsius`

## Points d'attention spécifiques au STM32L021K4TX

- **Flash 16 KB / RAM 2 KB** : pas de marge énorme. Si vous activez `printf` flottant (`-u _printf_float`) vous risquez de dépasser. Le main.c fourni utilise `(int)` pour rester en intier.
- **Cortex-M0+ sans FPU** : les conversions `float` sont calculées en soft. Acceptable à 104 Hz, mais en cas de besoin de plus haut débit utilisez les valeurs `raw` (`int16_t`).
- **Pas de DMA configuré** : tout est en polling. Suffisant pour ce test.
