/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef huart1;
uint8_t masterKod[4] = {1, 2, 3, 4};
uint8_t probalkozas[4];
uint8_t hanyszadik = 0;
uint8_t nyitvaVan = 0;

uint32_t utolsoGombnyomas = 0;
uint8_t oledBekapcsolva = 1;
uint8_t rosszProbaSzamlalo = 0;
uint32_t letiltasIdopontja = 0;
uint32_t utolsoInterakcio = 0;
uint32_t irErzekelesEleje = 0;

uint8_t frissiteniKell = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_GS4 415
#define NOTE_G5  784

void UART_Log(char *status, char *user) {
    char msg[64];
    int len = sprintf(msg, "%s;%s\r\n", status, user);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, len, 100);
}

void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

void playTone(uint16_t freq, uint16_t duration) {
    if (freq == 0) {
        HAL_Delay(duration);
        return;
    }
    uint32_t period = 1000000 / freq;
    uint32_t half_period = period / 2;
    uint32_t start_tick = HAL_GetTick();

    while (HAL_GetTick() - start_tick < duration) {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        delay_us(half_period);
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        delay_us(half_period);
    }
}

void play_BellaCiao() {
    uint16_t melody[] = {
        NOTE_A4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_D5, 0,
        NOTE_A4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_D5, 0,
        NOTE_A4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_E5, NOTE_D5,
        NOTE_F5, NOTE_E5, NOTE_D5, NOTE_A4, 0,
        NOTE_A4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_CS5, NOTE_D5, NOTE_D5
    };
    uint16_t durations[] = {
        200, 200, 200, 200, 400, 50,
        200, 200, 200, 200, 400, 50,
        200, 200, 200, 200, 200, 200,
        200, 200, 200, 400, 100,
        150, 150, 150, 150, 150, 400, 400
    };

    int utolso_mp = -1;

    for (int i = 0; i < sizeof(melody)/sizeof(uint16_t); i++) {
        playTone(melody[i], durations[i]);
        uint32_t most = HAL_GetTick();
        int hatralevo = 30 - ((most - utolsoInterakcio) / 1000);
        if (hatralevo < 0) hatralevo = 0;

        if (hatralevo != utolso_mp) {
            char buf[10];
            ssd1306_Fill(Black);
            ssd1306_SetCursor(20, 5);
            ssd1306_WriteString("Safe OPENED", Font_7x10, White);
            sprintf(buf, "%02ds", hatralevo);
            ssd1306_SetCursor(45, 25);
            ssd1306_WriteString(buf, Font_11x18, White);
            for(int x=0; x < (hatralevo * 4); x++) {
                ssd1306_DrawPixel(x + 4, 60, White);
                ssd1306_DrawPixel(x + 4, 61, White);
            }
            ssd1306_UpdateScreen();
            utolso_mp = hatralevo;
        }
        HAL_Delay(20);
    }
}

void play_MarioDie() {
    uint16_t melody[] = {
        NOTE_C5, NOTE_CS5, NOTE_D5, 0,
        NOTE_GS4, 0, NOTE_G4, 0, NOTE_F4,
        0, NOTE_E4, NOTE_D4, NOTE_C4
    };
    uint16_t durations[] = {
        100, 100, 100, 150,
        200, 50, 200, 50, 200,
        100, 200, 200, 400
    };
    for (int i = 0; i < 13; i++) {
        playTone(melody[i], durations[i]);
        HAL_Delay(10);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    playTone(NOTE_A4, 200);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    while (1)
      {
          uint32_t most = HAL_GetTick();
          char buf[20];

          // --- 1. LETILTÁS KEZELÉSE ---
          if (rosszProbaSzamlalo >= 3) {
              if (most - letiltasIdopontja < 60000) {
                  ssd1306_Fill(Black);
                  ssd1306_SetCursor(35, 20);
                  ssd1306_WriteString("WRONG!", Font_11x18, White);
                  int hatra = 60 - ((most - letiltasIdopontja) / 1000);
                  sprintf(buf, "WAIT: %ds", hatra);
                  ssd1306_SetCursor(40, 45);
                  ssd1306_WriteString(buf, Font_7x10, White);
                  ssd1306_UpdateScreen();
                  HAL_GPIO_TogglePin(GPIOB, Red_LED_Pin);
                  playTone(NOTE_C5, 100);
                  HAL_Delay(400);
                  continue;
              } else {
                  rosszProbaSzamlalo = 0;
                  frissiteniKell = 1;
                  HAL_GPIO_WritePin(GPIOB, Red_LED_Pin, GPIO_PIN_RESET);
              }
          }

          // --- 2. OLED ELALVÁS ---
          if (nyitvaVan == 0 && most - utolsoGombnyomas > 5000 && oledBekapcsolva) {
              ssd1306_WriteCommand(0xAE);
              oledBekapcsolva = 0;
          }

          // --- 3. GOMBOK ÉS IR ÉRZÉKELÉS ---
          int gomb = 0;
          if (HAL_GPIO_ReadPin(GPIOA, BUTTON_1_Pin) == GPIO_PIN_RESET) gomb = 1;
          else if (HAL_GPIO_ReadPin(GPIOA, BUTTON_2_Pin) == GPIO_PIN_RESET) gomb = 2;
          else if (HAL_GPIO_ReadPin(GPIOA, BUTTON_3_Pin) == GPIO_PIN_RESET) gomb = 3;
          else if (HAL_GPIO_ReadPin(GPIOA, BUTTON_4_Pin) == GPIO_PIN_RESET) gomb = 4;
          else if (HAL_GPIO_ReadPin(GPIOA, BUTTON_ENTER_Pin) == GPIO_PIN_RESET) gomb = 6;

          if (gomb > 0 || HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
              utolsoGombnyomas = most;
              if (!oledBekapcsolva) {
                  ssd1306_WriteCommand(0xAF);
                  oledBekapcsolva = 1;
                  frissiteniKell = 1;
              }
          }

          // --- 4. ÁLLAPOT: ZÁRVA (LOCKED) ---
          if (nyitvaVan == 0) {
        	  if (frissiteniKell) {
        	      ssd1306_Fill(Black);
        	      ssd1306_SetCursor(40, 5);
        	      ssd1306_WriteString("LOCKED", Font_7x10, White);
        	      ssd1306_SetCursor(32, 35);
        	      ssd1306_WriteString("_ _ _ _", Font_11x18, White);
        	      ssd1306_UpdateScreen();
        	      frissiteniKell = 0;
        	  }

        	  if (gomb >= 1 && gomb <= 4 && hanyszadik < 4) {
        	      playTone(NOTE_E5, 50);
        	      probalkozas[hanyszadik++] = gomb;
        	      ssd1306_SetCursor(32 + ((hanyszadik - 1) * 16), 33);
        	      ssd1306_WriteString("*", Font_11x18, White);
        	      ssd1306_UpdateScreen();
                  HAL_Delay(250);
                  while(HAL_GPIO_ReadPin(GPIOA, BUTTON_1_Pin) == GPIO_PIN_RESET ||
                        HAL_GPIO_ReadPin(GPIOA, BUTTON_2_Pin) == GPIO_PIN_RESET ||
                        HAL_GPIO_ReadPin(GPIOA, BUTTON_3_Pin) == GPIO_PIN_RESET ||
                        HAL_GPIO_ReadPin(GPIOA, BUTTON_4_Pin) == GPIO_PIN_RESET);
              }

              if (gomb == 6 && hanyszadik == 4) {
                    int hiba = 0;
                    for(int i=0; i<4; i++) {
                        if(probalkozas[i] != masterKod[i]) hiba = 1;
                    }

                    if (!hiba) { // SIKERES NYITÁS
                        UART_Log("OPEN", "MASTER");
                        nyitvaVan = 1;

                        // 1. Motor elforgatása (Nyitás: 1.5ms impulzus = 90 fok)
                        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1500);
                        HAL_GPIO_WritePin(GPIOB, Green_LED_Pin, GPIO_PIN_SET);

                        // 2. Zene lejátszása (Ez blokkolja a kódot)
                        play_BellaCiao();

                        // 3. CSAK MOST mentsük el a kezdőidőt, miután a zene véget ért!
                        utolsoInterakcio = HAL_GetTick();
                        rosszProbaSzamlalo = 0;
                    }
                    else { // ROSSZ KÓD
                        UART_Log("WRONG", "UNKNOWN");
                        play_MarioDie();
                        rosszProbaSzamlalo++;
                        ssd1306_Fill(Black);
                        ssd1306_SetCursor(35, 25);
                        ssd1306_WriteString("WRONG!", Font_11x18, White);
                        ssd1306_UpdateScreen();
                        HAL_GPIO_WritePin(GPIOB, Red_LED_Pin, GPIO_PIN_SET);
                        if (rosszProbaSzamlalo >= 3) letiltasIdopontja = most;
                        HAL_Delay(1000);
                        HAL_GPIO_WritePin(GPIOB, Red_LED_Pin, GPIO_PIN_RESET);
                        hanyszadik = 0;
                        frissiteniKell = 1;
                    }
                }
          }
          // --- 5. ÁLLAPOT: NYITVA ---
          else {
              uint32_t elteltIdo = (most - utolsoInterakcio) / 1000;
              int hatralevo = 30 - elteltIdo;

              ssd1306_Fill(Black);

              if (hatralevo > 0) {
                  // Kijelzés kezelése
                  ssd1306_SetCursor(20, 5);
                  ssd1306_WriteString("Safe OPENED", Font_7x10, White);
                  sprintf(buf, "%02ds", hatralevo);
                  ssd1306_SetCursor(45, 25);
                  ssd1306_WriteString(buf, Font_11x18, White);

                  // Progress bar rajzolása
                  for(int x=0; x < (hatralevo * 4); x++) {
                      ssd1306_DrawPixel(x + 4, 60, White);
                  }

                  // --- IR SZENZOR FIGYELÉSE ---
                  // Ha az IR (PA0) alacsony szintű (érzékel valamit)
                  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
                      static uint32_t irErzekelesEleje = 0;
                      if (irErzekelesEleje == 0) irErzekelesEleje = HAL_GetTick();

                      if (HAL_GetTick() - irErzekelesEleje > 1500) { // Csak ha 1.5 mp-ig folyamatosan takarják
                          utolsoInterakcio = HAL_GetTick() - 31000;
                      }
                  } else {
                      irErzekelesEleje = 0;
                  }
              }
              else {
                  // --- AUTOMATIKUS VAGY IR MIATTI ZÁRÁS ---
                  nyitvaVan = 0;
                  hanyszadik = 0;

                  // MOTOR VISSZAFORDÍTÁSA (Zárás: 1ms impulzus)
                  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1000);

                  HAL_GPIO_WritePin(GPIOB, Green_LED_Pin, GPIO_PIN_RESET);
                  UART_Log("CLOSED", "AUTO");
                  playTone(NOTE_G4, 200);
                  frissiteniKell = 1;
                  ssd1306_Fill(Black);
                  ssd1306_UpdateScreen();
                  HAL_Delay(500);
              }
              ssd1306_UpdateScreen();
          }
          HAL_Delay(50);
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
