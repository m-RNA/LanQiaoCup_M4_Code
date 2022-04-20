#include "bsp.h"

void LED_Disp(u8 LED)
{
    HAL_GPIO_WritePin(GPIOC, 0xFF00, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, LED<<8, GPIO_PIN_RESET);
    
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

u8 KEY_Scan(void)
{
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET)
        return 1;
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
        return 2;
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_RESET)
        return 3;
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
        return 4;
    return 0;
}

