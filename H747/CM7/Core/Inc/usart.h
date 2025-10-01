#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"   // 依你的 HAL 標頭命名（H747 通常是 stm32h7xx_hal.h）

extern UART_HandleTypeDef huart1;   // 讓其他檔案能看到 huart1
void MX_USART1_UART_Init(void);     // 初始化原型

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
