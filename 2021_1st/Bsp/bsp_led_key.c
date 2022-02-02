#include "bsp_led_key.h"

void LED_Disp(uint8_t LED)
{
	HAL_GPIO_WritePin(GPIOC, 0xFF00, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, LED << 8, GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(LED_LOCK_GPIO_Port, LED_LOCK_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_LOCK_GPIO_Port, LED_LOCK_Pin, GPIO_PIN_RESET);
}

uint8_t KEY_Scan(void)
{
	if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
		return 1;
	if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
		return 2;	
	if(HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET)
		return 3;	
	if(HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_RESET)
		return 4;	
	return 0;
}


