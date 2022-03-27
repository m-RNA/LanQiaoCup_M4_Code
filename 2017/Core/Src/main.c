/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_i2c.h"
#include "bsp.h"
#include "lcd.h"

#include "stdio.h"
#include "string.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define Task_Delay(Task_ID, Tick) \
  if(TimerTick[Task_ID] > HAL_GetTick()) return; \
  TimerTick[Task_ID] = HAL_GetTick() + Tick




/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
vu32 TimerTick[10] = {0};

u8 LED_State = 0x01;
u8 KEY_Val, KEY_Down, KEY_Old = 0;
u8 LCD_String_Buffer[21] = {0};
u8 UART_Rx_Temp = 0;
u8 UART_Tx_Buffer[50] = {0};
u16 ADC_Val = 0;
float R37_Vol = 0;


u8 Menu_State = 0;
u8 Setting_Index = 1;
u8 LED2_Blink_Timers = 0;
u8 LED3_Blink_Timers = 0;
u8 LCD_Update_Flag = 1; // LCD更新标志位

u8 Liquid_Height = 0;
u8 Liquid_Height_Old = 0;
u8 Liquid_Level = 0;
u8 Liquid_Thershold_Disp[5] = {5, 30, 50, 70, 95};
u8 Liquid_Thershold_Ctrl[3] = {30, 50, 70};



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Listen_Task(void);
void LCD_Task(void);
void KEY_Task(void);
void LED_Task(void);


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
  MX_ADC2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  LCD_Init();
  LCD_Clear(Black);
  LCD_SetBackColor(Black);
  LCD_SetTextColor(White);
  
  LED_Disp(LED_State);
 
  I2CInit(); // ※ 别忘了
  
  EEPROM_Read(&Liquid_Thershold_Disp[1], 3, 0x01);
  
  Liquid_Thershold_Ctrl[0] = Liquid_Thershold_Disp[1];
  Liquid_Thershold_Ctrl[1] = Liquid_Thershold_Disp[2];
  Liquid_Thershold_Ctrl[2] = Liquid_Thershold_Disp[3];
  
  ADC_Val = ADC_GetVal();
  Liquid_Height = ADC_Val * 100 / 4096;  
  Liquid_Height_Old = Liquid_Height;
  
  
  
  HAL_UART_Receive_IT(&huart1, &UART_Rx_Temp, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Listen_Task();
    LCD_Task();
    KEY_Task();
    LED_Task();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Listen_Task(void)
{
  Task_Delay(0, 1000);
  
  ADC_Val = ADC_GetVal();
  R37_Vol = ADC_Val * 3.3f / 4096;
  Liquid_Height = ADC_Val * 100 / 4096;
  
  LCD_Update_Flag = 1;
  
  if(Liquid_Height > Liquid_Height_Old)
  {
    LED2_Blink_Timers = 5;
    
    sprintf((char*)UART_Tx_Buffer, "A:H%d+L%d+U\r\n",Liquid_Height, Liquid_Level);
    HAL_UART_Transmit(&huart1, UART_Tx_Buffer, strlen((const char *)UART_Tx_Buffer), 50);  
  }
  else if(Liquid_Height < Liquid_Height_Old)
  {
    LED2_Blink_Timers = 5;
    
    sprintf((char*)UART_Tx_Buffer, "A:H%d+L%d+D\r\n",Liquid_Height, Liquid_Level);
    HAL_UART_Transmit(&huart1, UART_Tx_Buffer, strlen((const char *)UART_Tx_Buffer), 50);      
  }
  Liquid_Height_Old = Liquid_Height;
  
  LED_State ^= 1;
  LED_Disp(LED_State);
  
  if(Liquid_Height <= Liquid_Thershold_Ctrl[0])
    Liquid_Level = 0;
  else if(Liquid_Height <= Liquid_Thershold_Ctrl[1])
    Liquid_Level = 1;
  else if(Liquid_Height <= Liquid_Thershold_Ctrl[2])
    Liquid_Level = 2;  
  else
    Liquid_Level = 3;
}


void LCD_Task(void)
{
  if(LCD_Update_Flag)
  {
    LCD_Update_Flag = 0;
      
    if(Menu_State == 0)
    {
      sprintf((char *)LCD_String_Buffer, "    Liquid Level  ");
      LCD_DisplayStringLine(Line1, LCD_String_Buffer);  
      sprintf((char *)LCD_String_Buffer, "  Height:%3dcm  ", Liquid_Height);
      LCD_DisplayStringLine(Line3, LCD_String_Buffer); 
      sprintf((char *)LCD_String_Buffer, "  ADC:%4.2fV  ", R37_Vol);
      LCD_DisplayStringLine(Line5, LCD_String_Buffer);  
      sprintf((char *)LCD_String_Buffer, "  Level:%d  ", Liquid_Level);
      LCD_DisplayStringLine(Line7, LCD_String_Buffer);     
    }
    else
    {
      sprintf((char *)LCD_String_Buffer, "  Parameter Setup  ");
      LCD_DisplayStringLine(Line1, LCD_String_Buffer);   

      switch(Setting_Index)
      {
        case 1:
          LCD_SetBackColor(Blue);
          sprintf((char *)LCD_String_Buffer, " Thershold 1:%3dcm  ",Liquid_Thershold_Disp[1]);
          LCD_DisplayStringLine(Line3, LCD_String_Buffer);
          LCD_SetBackColor(Black);
          sprintf((char *)LCD_String_Buffer, " Thershold 2:%3dcm  ",Liquid_Thershold_Disp[2]);
          LCD_DisplayStringLine(Line5, LCD_String_Buffer);    
          sprintf((char *)LCD_String_Buffer, " Thershold 3:%3dcm  ",Liquid_Thershold_Disp[3]);
          LCD_DisplayStringLine(Line7, LCD_String_Buffer);           
          break;
        
        case 2:
          sprintf((char *)LCD_String_Buffer, " Thershold 1:%3dcm  ",Liquid_Thershold_Disp[1]);
          LCD_DisplayStringLine(Line3, LCD_String_Buffer);
          LCD_SetBackColor(Blue);
          sprintf((char *)LCD_String_Buffer, " Thershold 2:%3dcm  ",Liquid_Thershold_Disp[2]);
          LCD_DisplayStringLine(Line5, LCD_String_Buffer);    
          LCD_SetBackColor(Black);
          sprintf((char *)LCD_String_Buffer, " Thershold 3:%3dcm  ",Liquid_Thershold_Disp[3]);
          LCD_DisplayStringLine(Line7, LCD_String_Buffer);          
        break;
        
        case 3:
           sprintf((char *)LCD_String_Buffer, " Thershold 1:%3dcm  ",Liquid_Thershold_Disp[1]);
          LCD_DisplayStringLine(Line3, LCD_String_Buffer);
          sprintf((char *)LCD_String_Buffer, " Thershold 2:%3dcm  ",Liquid_Thershold_Disp[2]);
          LCD_DisplayStringLine(Line5, LCD_String_Buffer);    
          LCD_SetBackColor(Blue);
          sprintf((char *)LCD_String_Buffer, " Thershold 3:%3dcm  ",Liquid_Thershold_Disp[3]);
          LCD_DisplayStringLine(Line7, LCD_String_Buffer);         
          LCD_SetBackColor(Black);
          break;
      }
    }
  }
}

void KEY_Task(void)
{
  Task_Delay(1, 50);
  
  KEY_Val = KEY_Scan();
  KEY_Down = KEY_Val & (KEY_Val ^ KEY_Old);
  KEY_Old = KEY_Val;
    
  if(KEY_Down)
    LCD_Update_Flag = 1;

  switch(KEY_Down)
  {
    case 1:
      Menu_State ^= 1;
      LCD_Clear(Black);
      if(Menu_State == 0)
      {
        Liquid_Thershold_Ctrl[0] = Liquid_Thershold_Disp[1];
        Liquid_Thershold_Ctrl[1] = Liquid_Thershold_Disp[2];
        Liquid_Thershold_Ctrl[2] = Liquid_Thershold_Disp[3];
        
        EEPROM_Write(Liquid_Thershold_Ctrl, 3, 0x01);
        
        Setting_Index = 1;
      }
      break;  
      
    case 2:
      if(++Setting_Index == 4)
        Setting_Index = 1;
      break; 
      
    case 3:
      if(Liquid_Thershold_Disp[Setting_Index] < Liquid_Thershold_Disp[Setting_Index + 1])
        Liquid_Thershold_Disp[Setting_Index] += 5;
      break;  
      
    case 4:
      if(Liquid_Thershold_Disp[Setting_Index - 1] < Liquid_Thershold_Disp[Setting_Index])
        Liquid_Thershold_Disp[Setting_Index] -= 5;    
      break;      
  }
}

void LED_Task(void)
{
  Task_Delay(2, 200);
  
  if(LED2_Blink_Timers)
  {
    LED2_Blink_Timers--;
    LED_State ^= 0x02;
  }
  else
    LED_State &= ~0x02;
    

  if(LED3_Blink_Timers)
  {
    LED3_Blink_Timers--;
    LED_State ^= 0x04;
  } 
  else
    LED_State &= ~0x04;  
  
  LED_Disp(LED_State);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(UART_Rx_Temp == 'C')
  {
    sprintf((char*)UART_Tx_Buffer, "C:H%d+L%d\r\n",Liquid_Height, Liquid_Level);
    HAL_UART_Transmit(&huart1, UART_Tx_Buffer, strlen((const char *)UART_Tx_Buffer), 50);
  
    LED3_Blink_Timers= 5;
  }
  else if(UART_Rx_Temp == 'S')
  {
    sprintf((char*)UART_Tx_Buffer, "S:TL%d+TM%d+TH%d\r\n",Liquid_Thershold_Ctrl[0], Liquid_Thershold_Ctrl[1], Liquid_Thershold_Ctrl[2]);
    HAL_UART_Transmit(&huart1, UART_Tx_Buffer, strlen((const char *)UART_Tx_Buffer), 50);  

    LED3_Blink_Timers= 5;
  }

  HAL_UART_Receive_IT(&huart1, &UART_Rx_Temp, 1);
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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
