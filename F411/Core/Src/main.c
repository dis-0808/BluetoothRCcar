/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <time.h>

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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t rx_data;     // 單次接收的字元
uint8_t rx_flag = 0; // 收到新資料的flag
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint32_t IC_Val1 = 0; // 上升緣捕捉到的計數器值
volatile uint32_t IC_Val2 = 0; // 降緣捕捉到的計數器值
volatile uint32_t Difference = 0; // 兩者相減得到的高脈寬
volatile uint8_t  Is_First_Captured = 0; // 狀態旗標
volatile uint16_t Distance = 0; // 計算後的距離，單位 cm

void HCSR05_Read(void);
void delay(uint16_t time);

void delay (uint16_t time) {
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while (__HAL_TIM_GET_COUNTER(&htim1) < time);
}

int count = 0; // 隨機模式計數
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
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);  // 啟動 TIM1

  srand( (unsigned)HAL_GetTick() ); //以開機至今的毫秒數當亂數種子


  /* 啟動 PWM：左=CH1(PA6)、右=CH2(PA7) */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  /* 取得 ARR，方便計算 duty */
  uint32_t PWM_MAX = __HAL_TIM_GET_AUTORELOAD(&htim3);

  /* 給一個預設速度（70%） */
  uint32_t speed = (uint32_t)(PWM_MAX * 0.70f);

  // 非阻塞，避免 UART 卡死
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1); //啟動 TIM1 CH1 輸入 中斷
  HAL_UART_Receive_IT(&huart1, &rx_data, 1);  //開啟UART 中斷
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (rx_flag) { //rx_flag = 1:有新資料，中斷裡面改
	          uint8_t data = rx_data;  // 把剛收到的字元讀出
	          rx_flag = 0;             // 清除旗標

	          switch (data)
	          {
	            case 'w': // forward
	            	  // 先停 PWM 避免瞬間短路
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
	              HAL_Delay(5);
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET); // IN1=0
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   // IN2=1
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // IN3=0
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);   // IN4=1
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed);   // 左 ENA
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed);   // 右 ENB
	              break;

	            case 's': // back
		              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
		              HAL_Delay(5);
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed);
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed);
	              break;

	            case 'a':
		              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
		              HAL_Delay(5);
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET); // IN1=0
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   // IN2=1
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);   // IN3=1
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET); // IN4=0
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed);       // 左 ENA
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed);       // 右 ENB
	              break;

	            case 'd': // right
		              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
		              HAL_Delay(5);
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);   // IN1=1
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); // IN2=0
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // IN3=0
		              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);   // IN4=1
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed);       // 左 ENA
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed);       // 右 ENB
	              break;

	            case 'x': // 隨機 20 次
	              for (count = 0; count < 20; count++) {
	                int dir = (rand() % 4);
	                uint32_t rnd_speed = (uint32_t)((0.4f + 0.6f*((float)rand()/RAND_MAX)) * PWM_MAX);

	                switch (dir) {
	                  case 0: // forward
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
	                    break;
	                  case 1: // back
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	                    break;
	                  case 2: // left
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
	                    break;
	                  case 3: // right
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	                    break;
	                }

	                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, rnd_speed);
	                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, rnd_speed);
	                HAL_Delay(500);
	              }
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
	              break;

	            case 't': // stop
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
	              __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
	              HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
	              break;

	            default:
	              break;
	          }
	      }

	  // 觸發一次超音波量測
	  HCSR05_Read();

	  // 安全停車（<10cm）
	  if (Distance < 10) {
	    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);    // 蜂鳴器
	    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
	    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
	    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
	  } else {
	    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
	  }

	  HAL_Delay(20);

  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON; //啟用內部石英震盪器
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;  // 除頻 M=8 → 16MHz/8 = 2 MHz
  RCC_OscInitStruct.PLL.PLLN = 72; // 倍頻 N=72 → 2 MHz × 72 = 144 MHz
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // 再除以 P=2 → 144/2 = 72 MHz (系統時鐘 SYSCLK)
  RCC_OscInitStruct.PLL.PLLQ = 4;   // 給 USB/SDIO 用 → 144/4 = 36 MHz
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
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;  // 預分頻 PSC=71 → 72MHz / (71+1) = 1MHz
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP; // 正向遞增計數
  htim1.Init.Period = 65535;  // ARR=65535 → 計數範圍 0~65535（約 65.535ms）
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // 時鐘分頻=1（對 IC 取樣時基）
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL; // 用內部時鐘
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim1) != HAL_OK) //啟用輸入捕捉功能
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 3599;  // ARR=3599 → 週期計數 3600
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // 時鐘分頻=1
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0; //// 初始佔空比=0
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH; // 比較值以上為高電平
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B; // 每個字元 8 bit
  huart1.Init.StopBits = UART_STOPBITS_1;    // 停止位 1
  huart1.Init.Parity = UART_PARITY_NONE;     // 不用奇偶校驗
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  /* 如果你之後要用中斷收資料，可以在這裡呼叫
       HAL_UART_Receive_IT(&huart1, &rx_data, 1); */
  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PD11 PD12 PD13 PD14
                           PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* 如果你之後還要加按鍵/感測器，可以在這裡設定輸入腳位 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HCSR05_Read(void) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET); //產生 Trigger 高脈衝
  delay(10); // 10 µs trigger
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET); //結束 Trigger
  __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1); // 開啟 TIM1 CH1 的捕捉中斷（等待 Echo 邊緣）
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) { //CH1 觸發
    if (Is_First_Captured == 0) { //第一次：上升緣
      IC_Val1 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1); //記錄上升緣時間戳
      Is_First_Captured = 1; //進入「等待下降緣」狀態
      __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
    } else { //第二次：下降緣
      IC_Val2 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1); //記錄下降緣時間戳
      __HAL_TIM_SET_COUNTER(&htim1, 0); //清計數器，避免下次溢位誤差累積

      if (IC_Val2 >= IC_Val1) Difference = IC_Val2 - IC_Val1;
      else                    Difference = (65535 - IC_Val1) + IC_Val2;

      Distance = (uint16_t)(Difference * 0.0343f / 2.0f); // 距離(cm)=脈寬(µs)*聲速(cm/µs)/2
      Is_First_Captured = 0; // 重置

      __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
      __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_CC1);
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) { //確認是 USART1 的收訊中斷
    rx_flag = 1;  // 通知主迴圈有新資料
    // 再次啟動接收下一個字元
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  }
}


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
