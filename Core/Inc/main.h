#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

void Error_Handler(void);


#define LCD_BL_CTRL_Pin GPIO_PIN_3
#define LCD_BL_CTRL_GPIO_Port GPIOK
#define GREEN_LED_Pin GPIO_PIN_1
#define GREEN_LED_GPIO_Port GPIOI
#define USER_BUTTON_Pin GPIO_PIN_11
#define USER_BUTTON_GPIO_Port GPIOI



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
