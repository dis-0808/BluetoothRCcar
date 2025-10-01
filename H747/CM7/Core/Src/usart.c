#include "usart.h"

/* 真正定義 huart1（只有一個 .c 裡定義，其他檔用 extern） */
UART_HandleTypeDef huart1;

/* === MX 初始化：設定 baud/格式等 === */
void MX_USART1_UART_Init(void)
{
  huart1.Instance          = USART1;
  huart1.Init.BaudRate     = 115200;
  huart1.Init.WordLength   = UART_WORDLENGTH_8B;
  huart1.Init.StopBits     = UART_STOPBITS_1;
  huart1.Init.Parity       = UART_PARITY_NONE;
  huart1.Init.Mode         = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* === MSP：時脈、GPIO、NVIC（H7 必要）=== */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  if (uartHandle->Instance == USART1)
  {
    /* 1) 外設時脈（H7：若你在 SystemClock_Config 已用 RCC_PeriphCLK 指定 USART1 clock，可保留這邊只開總線時脈） */
    __HAL_RCC_USART1_CLK_ENABLE();

    /* 2) GPIO 時脈 */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 3) 腳位複用：PA9=TX, PA10=RX, AF7 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 4) （可選）中斷 */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
}
