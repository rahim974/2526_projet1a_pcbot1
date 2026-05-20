/**
 * @file    motorV2.c
 * @brief   Contrôle moteurs DC via DRV8833 + STM32 TIM1 PWM
 *
 * Brochage (selon schéma DRV8833APWPR) :
 *   - Moteur 1 (MOTEUR 1) : AIN1 = TIM1_CH1 / AIN2 = TIM1_CH1N
 *   - Moteur 2 (MOTEUR 2) : BIN1 = TIM1_CH2 / BIN2 = TIM1_CH2N
 *
 * Logique DRV8833 :
 *   AIN1=PWM, AIN2=0  → Moteur 1 AVANT
 *   AIN1=0,   AIN2=PWM → Moteur 1 ARRIÈRE
 *   AIN1=AIN2=0        → Roue libre
 *   AIN1=AIN2=1        → Frein
 *
 * Comportement ToF :
 *   - Distance >= 300 mm → avance tout droit
 *   - Distance <  300 mm → tourne 90° droite (roue gauche tourne, roue droite arrêtée)
 *                          puis reprend la marche avant automatiquement
 */

#include "motorV2.h"
#include "tim.h"      /* htim1 généré par CubeMX */
#include "cmsis_os.h" /* osDelay() si FreeRTOS, sinon remplacer par HAL_Delay() */

/* ------------------------------------------------------------------ */
/*  Paramètres à adapter à votre projet                                */
/* ------------------------------------------------------------------ */

/** Période du timer (ARR+1). Doit correspondre à la config CubeMX.   */
#define MOTOR_PWM_PERIOD        1000U

/** Vitesse nominale (0 – MOTOR_PWM_PERIOD).                           */
#define MOTOR_SPEED_NORMAL      700U

/** Seuil obstacle en millimètres.                                     */
#define OBSTACLE_DISTANCE_MM    300U

/**
 * Durée du virage à 90° en ms.
 * À calibrer expérimentalement selon :
 *   - diamètre de roue, écartement, vitesse choisie.
 * Exemple de départ : 600 ms à vitesse normale.
 */
#define TURN_90DEG_DURATION_MS  600U

/* ------------------------------------------------------------------ */
/*  Macros internes                                                     */
/* ------------------------------------------------------------------ */

/* Canaux TIM1 associés aux entrées du DRV8833 */
#define CH_AIN1   TIM_CHANNEL_1    /* Moteur gauche – sens avant  */
#define CH_AIN2   TIM_CHANNEL_1    /* Complémentaire géré en CH1N */
#define CH_BIN1   TIM_CHANNEL_2    /* Moteur droit  – sens avant  */
#define CH_BIN2   TIM_CHANNEL_2    /* Complémentaire géré en CH2N */

/** Applique un rapport cyclique sur un canal PWM (mode complémentaire géré par le timer). */
#define SET_PWM(channel, duty) \
    __HAL_TIM_SET_COMPARE(&htim1, (channel), (duty))

/* ------------------------------------------------------------------ */
/*  Fonctions privées                                                   */
/* ------------------------------------------------------------------ */

/**
 * @brief  Démarre les 4 canaux PWM complémentaires du TIM1.
 *         À appeler une seule fois lors de l'initialisation.
 */
static void Motor_StartPWM(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);  /* CH1N */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);  /* CH2N */
}

/**
 * @brief  Commande le moteur GAUCHE (Moteur 1 – pont A du DRV8833).
 * @param  speed : 0 = stop / frein, >0 = vitesse en avant (0..MOTOR_PWM_PERIOD)
 *
 * Note : les signaux complémentaires (AIN1N / AIN2N) sont gérés
 *        matériellement par TIM1 en mode PWM complémentaire.
 *        AIN1=PWM  → AIN1N=0 inversé par le DT (dead-time)
 *        Résultat  → pont A fait avancer le moteur gauche.
 */
static void Motor_SetLeft(uint32_t speed)
{
    if (speed == 0U)
    {
        /* Frein : AIN1=AIN2=1 → mettre CCR à la période complète */
        SET_PWM(TIM_CHANNEL_1, MOTOR_PWM_PERIOD);
    }
    else
    {
        SET_PWM(TIM_CHANNEL_1, speed);
    }
}

/**
 * @brief  Commande le moteur DROIT (Moteur 2 – pont B du DRV8833).
 * @param  speed : 0 = stop / frein, >0 = vitesse en avant (0..MOTOR_PWM_PERIOD)
 */
static void Motor_SetRight(uint32_t speed)
{
    if (speed == 0U)
    {
        SET_PWM(TIM_CHANNEL_2, MOTOR_PWM_PERIOD);
    }
    else
    {
        SET_PWM(TIM_CHANNEL_2, speed);
    }
}

/* ------------------------------------------------------------------ */
/*  API publique                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief  Initialise le module moteur (démarre les PWM).
 *         À appeler dans main() APRÈS MX_TIM1_Init().
 */
void Motor_Init(void)
{
    Motor_StartPWM();
    Motor_Stop();  /* Freins actifs au démarrage */
}

/**
 * @brief  Marche avant à vitesse nominale (les deux roues).
 */
void Motor_Forward(void)
{
    Motor_SetLeft(MOTOR_SPEED_NORMAL);
    Motor_SetRight(MOTOR_SPEED_NORMAL);
}

/**
 * @brief  Arrêt (freinage actif) des deux moteurs.
 */
void Motor_Stop(void)
{
    Motor_SetLeft(0U);
    Motor_SetRight(0U);
}

/**
 * @brief  Virage 90° vers la droite.
 *
 *         Méthode : pivot sur place côté droit.
 *           - Roue gauche : tourne à vitesse normale  → robot pivote vers la droite
 *           - Roue droite : à l'arrêt (frein)
 *
 *         Durée calibrée par TURN_90DEG_DURATION_MS.
 */
void Motor_TurnRight90(void)
{
    Motor_SetLeft(MOTOR_SPEED_NORMAL);  /* Roue gauche : avance */
    Motor_SetRight(0U);                 /* Roue droite : frein  */

#ifdef USE_FREERTOS
    osDelay(TURN_90DEG_DURATION_MS);
#else
    HAL_Delay(TURN_90DEG_DURATION_MS);
#endif
}

/**
 * @brief  Boucle principale de navigation autonome.
 *
 *         À appeler en boucle dans une tâche FreeRTOS ou dans la
 *         boucle while(1) de main().
 *
 * @param  distance_mm  Distance mesurée par le capteur ToF (en mm).
 *
 * Comportement :
 *   - distance >= 300 mm → avance tout droit
 *   - distance <  300 mm → s'arrête brièvement, tourne 90° à droite,
 *                          puis reprend la marche avant
 */
void Motor_NavigationUpdate(uint16_t distance_mm)
{
    if (distance_mm < OBSTACLE_DISTANCE_MM)
    {
        /* 1. Freinage d'urgence */
        Motor_Stop();
#ifdef USE_FREERTOS
        osDelay(200U);
#else
        HAL_Delay(200U);
#endif

        /* 2. Virage 90° vers la droite */
        Motor_TurnRight90();

        /* 3. Petite pause avant de repartir */
#ifdef USE_FREERTOS
        osDelay(100U);
#else
        HAL_Delay(100U);
#endif

        /* 4. Reprend la marche avant automatiquement */
        Motor_Forward();
    }
    else
    {
        /* Voie libre → avance */
        Motor_Forward();
    }
}
