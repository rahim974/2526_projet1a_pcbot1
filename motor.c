/* ===========================================================
 * motor.c
 * DRV8411A - Contrôle 2 moteurs DC en PWM sur STM32L4
 * Mode    : IN1/IN2 PWM
 * Timer   : TIM2 — PA0 (AIN1), PA1 (AIN2), PA2 (BIN1), PA3 (BIN2)
 * PWM     : 20 kHz
 *
 * CONFIGURATION CUBEMX :
 *   TIM2 → Clock Source : Internal Clock
 *         CH1 : PWM Generation → PA0   (AIN1)
 *         CH2 : PWM Generation → PA1   (AIN2)
 *         CH3 : PWM Generation → PA2   (BIN1)
 *         CH4 : PWM Generation → PA3   (BIN2)
 *         Prescaler       : 3    (80MHz / 4 = 20MHz)
 *         Counter Period  : 999  (20MHz / 1000 = 20kHz)
 *         PWM Mode        : PWM Mode 1
 *         Fast Mode       : Disable
 *
 *   nSLEEP du DRV8411A → brancher sur 3.3V pour activer le driver
 * ===========================================================*/

#include "stm32l4xx_hal.h"

/* =========================================================
 * Types
 * ========================================================= */

typedef enum {
    MOTOR_A = 0,    /* AIN1 = TIM2_CH1 (PA0), AIN2 = TIM2_CH2 (PA1) */
    MOTOR_B         /* BIN1 = TIM2_CH3 (PA2), BIN2 = TIM2_CH4 (PA3) */
} Motor_t;

typedef enum {
    MOTOR_FORWARD = 0,  /* Sens avant  : IN1=PWM, IN2=0 */
    MOTOR_BACKWARD      /* Sens arrière : IN1=0,   IN2=PWM */
} Motor_Dir_t;

/* =========================================================
 * Définitions internes
 * ========================================================= */

#define PWM_MAX         1000U   /* = ARR + 1 */
#define SPEED_MAX_PCT   100U

extern TIM_HandleTypeDef htim2; /* Généré par CubeMX dans main.c */

/* =========================================================
 * Fonction privée : % → valeur CCR
 * ========================================================= */

static uint32_t percent_to_ccr(uint8_t percent)
{
    if (percent > SPEED_MAX_PCT) percent = SPEED_MAX_PCT;
    return (uint32_t)((percent * PWM_MAX) / 100U);
}

/* =========================================================
 * Motor_Init
 * Démarre les 4 canaux PWM — à appeler après MX_TIM2_Init()
 * ========================================================= */

void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  /* AIN1 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  /* AIN2 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);  /* BIN1 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);  /* BIN2 */

    /* Moteurs à l'arrêt au démarrage */
    Motor_Stop(MOTOR_A);
    Motor_Stop(MOTOR_B);
}

/* =========================================================
 * Motor_Run
 * Fait tourner un moteur dans un sens à une vitesse donnée
 *   motor     : MOTOR_A ou MOTOR_B
 *   dir       : MOTOR_FORWARD ou MOTOR_BACKWARD
 *   speed_pct : 0 à 100 (%)
 * ========================================================= */

void Motor_Run(Motor_t motor, Motor_Dir_t dir, uint8_t speed_pct)
{
    uint32_t ccr = percent_to_ccr(speed_pct);

    if (motor == MOTOR_A)
    {
        if (dir == MOTOR_FORWARD)
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        }
        else
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, ccr);
        }
    }
    else /* MOTOR_B */
    {
        if (dir == MOTOR_FORWARD)
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccr);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);
        }
        else
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, ccr);
        }
    }
}

/* =========================================================
 * Motor_Stop
 * Arrêt en roue libre : IN1=0, IN2=0
 * ========================================================= */

void Motor_Stop(Motor_t motor)
{
    if (motor == MOTOR_A)
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
    }
    else
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);
    }
}

/* =========================================================
 * Motor_Brake
 * Freinage actif : IN1=1, IN2=1
 * ========================================================= */

void Motor_Brake(Motor_t motor)
{
    if (motor == MOTOR_A)
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, PWM_MAX);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, PWM_MAX);
    }
    else
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, PWM_MAX);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, PWM_MAX);
    }
}

/* =========================================================
 * EXEMPLE D'UTILISATION DANS main.c :
 *
 *   #include "motor.c"   // ou ajouter motor.c au projet
 *
 *   int main(void)
 *   {
 *       HAL_Init();
 *       SystemClock_Config();
 *       MX_GPIO_Init();
 *       MX_TIM2_Init();
 *
 *       Motor_Init();
 *
 *       Motor_Run(MOTOR_A, MOTOR_FORWARD,  75);  // A : avant  75%
 *       Motor_Run(MOTOR_B, MOTOR_BACKWARD, 50);  // B : arrière 50%
 *
 *       HAL_Delay(3000);
 *
 *       Motor_Brake(MOTOR_A);   // Freinage A
 *       Motor_Stop(MOTOR_B);    // Roue libre B
 *
 *       while (1) { }
 *   }
 * ========================================================= */
