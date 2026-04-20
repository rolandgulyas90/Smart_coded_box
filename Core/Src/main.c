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
#include <stdio.h>        // A sprintf függvényhez
#include "ssd1306.h"      // Az OLED kijelzőhöz
#include "ssd1306_fonts.h" // A betűtípusokhoz (Font_7x10, stb.)

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
uint8_t masterKod[4] = {1, 2, 3, 4};
uint8_t probalkozas[4];
uint8_t hanyszadik = 0;
uint8_t nyitvaVan = 0; // 0: Zárva, 1: Nyitva

uint32_t utolsoGombnyomas = 0;
uint8_t oledBekapcsolva = 1;
uint8_t rosszProbaSzamlalo = 0;
uint32_t letiltasIdopontja = 0;
uint32_t utolsoInterakcio = 0;

// Segéd változó, hogy csak akkor rajzoljunk az OLED-re, ha változás van
uint8_t frissiteniKell = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
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
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_GS4 415

void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);  // Használjuk a már futó Tim2-t
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

// Egy konkrét hang lejátszása
void playTone(uint16_t freq, uint16_t duration) {
    if (freq == 0) {
        HAL_Delay(duration);
        return;
    }
    uint32_t period = 1000000 / freq;
    uint32_t half_period = period / 2;
    uint32_t start_tick = HAL_GetTick();

    while (HAL_GetTick() - start_tick < duration) {
        // A CubeMX-ben adott "BUZZER" név miatt ezeket használjuk:
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        delay_us(half_period);
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        delay_us(half_period);
    }
}

void play_BellaCiao() {
    uint16_t melody[] = { NOTE_A4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_D5, 0, NOTE_A4, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_D5 };
    uint16_t durations[] = { 200, 200, 200, 200, 400, 50, 200, 200, 200, 200, 400 };
    for (int i = 0; i < 11; i++) {
        playTone(melody[i], durations[i]);
        HAL_Delay(20);
    }
}

void play_BreakingTheHabit() {
    playTone(NOTE_E5, 300); playTone(NOTE_E5, 300); playTone(NOTE_E5, 300);
    playTone(NOTE_D5, 300); playTone(NOTE_C5, 300);
    playTone(NOTE_A4, 600);
}

void play_MarioDie() {
    playTone(NOTE_C5, 150); playTone(NOTE_G4, 150); playTone(NOTE_E4, 150);
    playTone(NOTE_A4, 200); playTone(NOTE_B4, 200); playTone(NOTE_G4, 400);
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
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    // JAVÍTOTT TESZT: Rövid csippanás a PA5 lábon indításkor
    playTone(NOTE_A4, 200);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint32_t most = HAL_GetTick();
      char buf[20];

      // --- 1. LETILTÁS KEZELÉSE (3 hiba után 1 perc büntetés) ---
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

              // Büntetés alatti "alarm" hangjelzés
              playTone(NOTE_C5, 100);
              HAL_Delay(400);
              continue;
          } else {
              rosszProbaSzamlalo = 0;
              frissiteniKell = 1;
              HAL_GPIO_WritePin(GPIOB, Red_LED_Pin, GPIO_PIN_RESET);
          }
      }

      // --- 2. OLED ELALVÁS (5 mp után kikapcsol, ha zárva van) ---
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
              // Buzzer visszajelzés gombnyomásra
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
                        int lp_kod = 1; // Feltételezzük, hogy ez a titkos kód
                        uint8_t lp_target[4] = {4, 3, 3, 4}; // A Linkin Park kódja

                        // 1. ELLENŐRZÉS: Megnézzük mindkét kódot egyszerre
                        for(int i=0; i<4; i++) {
                            if(probalkozas[i] != masterKod[i]) hiba = 1; // Ha nem a mesterkód
                            if(probalkozas[i] != lp_target[i]) lp_kod = 0; // Ha nem az LP kód
                        }

                        // 2. ELÁGAZÁS: Melyik kód jött be?

                        if (lp_kod) { // --- HA A LINKIN PARK KÓDOT ÜTÖTTÉK BE ---
                            ssd1306_Fill(Black);
                            ssd1306_SetCursor(25, 25);
                            ssd1306_WriteString("LP MODE", Font_11x18, White);
                            ssd1306_UpdateScreen();

                            play_BreakingTheHabit(); // Lejátssza a zenét

                            hanyszadik = 0; // Kód törlése a bevitel után
                            frissiteniKell = 1; // Visszaáll a LOCKED felirat a zene után
                        }
                        else if (!hiba) { // --- HA A MESTERKÓD JÓ (BELLA CIAO + NYITÁS) ---
                            nyitvaVan = 1;
                            utolsoInterakcio = most;
                            rosszProbaSzamlalo = 0;

                            // Szervó nyitása
                            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 2000);
                            HAL_GPIO_WritePin(GPIOB, Green_LED_Pin, GPIO_PIN_SET);

                            play_BellaCiao(); // Zene indítása
                        }
                        else { // --- HA SEMELYIK KÓD NEM JÓ (MARIO + WRONG) ---
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
                int hatralevo = 30 - ((most - utolsoInterakcio) / 1000);
                ssd1306_Fill(Black);

                if (hatralevo > 0) {
                    // --- NYITVA VAN, MÉG MEGY A VISSZASZÁMLÁLÁS ---
                    ssd1306_SetCursor(20, 5);
                    ssd1306_WriteString("Safe OPENED", Font_7x10, White);
                    sprintf(buf, "%02ds", hatralevo);
                    ssd1306_SetCursor(45, 25);
                    ssd1306_WriteString(buf, Font_11x18, White);

                    for(int x=0; x < (hatralevo * 4); x++) {
                        ssd1306_DrawPixel(x + 4, 60, White);
                        ssd1306_DrawPixel(x + 4, 61, White);
                    }
                }
                else {
                    // --- LEJÁRT A 30 MÁSODPERC: AUTOMATIKUS ZÁRÁS ---
                    nyitvaVan = 0;
                    hanyszadik = 0;

                    // SZERVÓ ZÁRÁSA AZONNAL
                    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1000);
                    HAL_GPIO_WritePin(GPIOB, Green_LED_Pin, GPIO_PIN_RESET);

                    // Buzzer jelzés a zárásról
                    playTone(NOTE_G4, 200);
                    frissiteniKell = 1; // Hogy visszaugorjon a LOCKED képernyőre
                    ssd1306_Fill(Black);
                    ssd1306_UpdateScreen();
                    HAL_Delay(500);
                }

                // EXTRA: Ha még nem járt le az idő, de az IR-t megszakítják (becsukják kézzel)
                // Ezt csak akkor hagyd benne, ha akarod, hogy kézzel is be lehessen csukni idő előtt
                if (nyitvaVan == 1 && hatralevo < 27) { // 3mp türelmi idő után
                    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
                        HAL_Delay(300);
                        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
                            utolsoInterakcio = most - 31000; // Ezzel kényszerítjük a fenti "else" ágat
                        }
                    }
                }

                ssd1306_UpdateScreen();
            }
      HAL_Delay(50);
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Green_LED_Pin|Red_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BUTTON_1_Pin BUTTON_2_Pin BUTTON_3_Pin BUTTON_4_Pin
                           BUTTON_ENTER_Pin PA7 */
  GPIO_InitStruct.Pin = BUTTON_1_Pin|BUTTON_2_Pin|BUTTON_3_Pin|BUTTON_4_Pin
                          |BUTTON_ENTER_Pin|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Green_LED_Pin Red_LED_Pin */
  GPIO_InitStruct.Pin = Green_LED_Pin|Red_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
