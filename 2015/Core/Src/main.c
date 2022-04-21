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
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "bsp.h"
#include "bsp_iic.h"

#include "stdio.h"
#include "string.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define EEPROM_CHECK_ADD 0x22
#define EEPROM_CHECK_PSD 0x20

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
__IO u32 TimerTick[10] = {0};

u8 KEY_Val, KEY_Down, KEY_Old = 0;
u8 LED_State = 0;
u8 LED_Ctrl = 1;
u8 LCD_String_Buffer[21];
u8 Uart_Rx_Temp = 0;
u8 Uart_Rx_Buffer[50] = {0};
u8 Uart_Rx_Index = 0;
u8 Uart_Tx[128] = {0};
u8 Uart_Report_Flag = 0;

u8 Menu_State = 0;
u8 Setting_Index = 0;
u16 ADC_Val = 0;
float R37_Vol;
u8 k_x10_int = 1;
u8 LCD_Setting_Blink_Flag = 0;

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

u8 Report_Time_Disp[3] = {0};
u8 Report_Time_Ctrl[3] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LCD_Task(void);
void KEY_Task(void);
void UART_Task(void);
void Linsten_Task(void);   

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
  MX_RTC_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  I2CInit(); // ※ 这里别忘了
  
  LCD_Init();
  LCD_Clear(Black);
  LCD_SetBackColor(Black);
  LCD_SetTextColor(White);
  
  LED_Disp(LED_State);
  
  // 检测是否为默认
  Read_EEPROM(&k_x10_int, 1, EEPROM_CHECK_ADD);
  if(k_x10_int != EEPROM_CHECK_PSD)
  {
    k_x10_int = 1;
    Write_EEPROM(&k_x10_int, 1, 0x00);
  }
  else if(k_x10_int == 0 || k_x10_int > 9)
  {
    k_x10_int = 1;
    Write_EEPROM(&k_x10_int, 1, 0x00);  
  }
  
  HAL_UART_Receive_IT(&huart1, &Uart_Rx_Temp, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    LCD_Task();
    KEY_Task();
    UART_Task();
    Linsten_Task();   
            
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void LCD_Task(void)
{
  if(TimerTick[6] < HAL_GetTick())
  {
    LCD_Setting_Blink_Flag ^= 1;
    TimerTick[6] = HAL_GetTick() + 200;
  }
    
  if(TimerTick[0] > HAL_GetTick())
    return;
  TimerTick[0] = HAL_GetTick() + 100;
  
  if(Menu_State == 0)
  {
    sprintf((char *)LCD_String_Buffer, "    V1:%.2fV   ", R37_Vol); // 
    LCD_DisplayStringLine(Line1, LCD_String_Buffer);
    sprintf((char *)LCD_String_Buffer, "    k:0.%d   ", k_x10_int); // 
    LCD_DisplayStringLine(Line3, LCD_String_Buffer);  
    
    if(LED_Ctrl)
    {
      sprintf((char *)LCD_String_Buffer, "    LED:ON "); // 
      LCD_DisplayStringLine(Line5, LCD_String_Buffer);      
    }
    else
    {
      sprintf((char *)LCD_String_Buffer, "    LED:OFF"); // 
      LCD_DisplayStringLine(Line5, LCD_String_Buffer);       
    }
  
    sprintf((char *)LCD_String_Buffer, "    T:%02d-%02d-%02d", sTime.Hours, sTime.Minutes, sTime.Seconds); // 
    LCD_DisplayStringLine(Line7, LCD_String_Buffer);     
  }
  else
  {
    sprintf((char *)LCD_String_Buffer, "       Setting");
    LCD_DisplayStringLine(Line1, LCD_String_Buffer); 

    sprintf((char *)LCD_String_Buffer, "      %02d-%02d-%02d", Report_Time_Disp[0], Report_Time_Disp[1], Report_Time_Disp[2]);   

    if(LCD_Setting_Blink_Flag)
    {
      #define LCD_SETTING_OFFSER 0
       switch(Setting_Index)
      {
       case 0:
         LCD_String_Buffer[6 + LCD_SETTING_OFFSER] = ' ';
         LCD_String_Buffer[7 + LCD_SETTING_OFFSER] = ' ';    
         break;

       case 1:
         LCD_String_Buffer[ 9 + LCD_SETTING_OFFSER] = ' ';
         LCD_String_Buffer[10 + LCD_SETTING_OFFSER] = ' ';         
         break;

       case 2:
         LCD_String_Buffer[12 + LCD_SETTING_OFFSER] = ' ';
         LCD_String_Buffer[13 + LCD_SETTING_OFFSER] = ' ';
         break;
      }       
    }
    LCD_DisplayStringLine(Line5, LCD_String_Buffer);      
  
  
  }
}

void KEY_Task(void)
{
  if(TimerTick[1] > HAL_GetTick())
    return;
  TimerTick[1] = HAL_GetTick() + 50;
  
  KEY_Val = KEY_Scan();
  KEY_Down = KEY_Val & (KEY_Val ^ KEY_Old);
  KEY_Old = KEY_Val;
  
  switch( KEY_Down)
  {
    case 1:
      LED_Ctrl ^= 1;
    
      Report_Time_Ctrl[0] = Report_Time_Disp[0];
      Report_Time_Ctrl[1] = Report_Time_Disp[1];
      Report_Time_Ctrl[2] = Report_Time_Disp[2];
      
      break;

     case 2:
       Menu_State ^= 1;
       LCD_Clear(Black);
       break;   
    
     case 3:
       if(Menu_State ==1)
       {
         if(++Setting_Index == 3)
           Setting_Index = 0;
       }
      break;   
    
    case 4:
      if(Menu_State ==1)
      {
        if(Setting_Index == 0)
        {
          if(++Report_Time_Disp[Setting_Index] == 24)
            Report_Time_Disp[Setting_Index] = 0;
        }
        else
        {
           if(++Report_Time_Disp[Setting_Index] == 60)
            Report_Time_Disp[Setting_Index] = 0;       
        }
      }
      break;    
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  Uart_Rx_Buffer[Uart_Rx_Index] = Uart_Rx_Temp;
  Uart_Rx_Index++;
  
  TimerTick[2] = HAL_GetTick() + 20;  
  
  HAL_UART_Receive_IT(&huart1, &Uart_Rx_Temp, 1);
}


void UART_Task(void)
{
  if(Uart_Rx_Index == 0) 
    return;

  if(TimerTick[2] > HAL_GetTick())
    return;
  
  if(Uart_Rx_Index == 6)
  {
    if(Uart_Rx_Buffer[0] == 'k' && Uart_Rx_Buffer[1] == '0' && Uart_Rx_Buffer[2] == '.' &&
      (Uart_Rx_Buffer[3] >= '1' && Uart_Rx_Buffer[3] <= '9') && Uart_Rx_Buffer[4] == 0x5C && Uart_Rx_Buffer[5] == 'n')
    {
      k_x10_int = Uart_Rx_Buffer[3] - '0';
      
      Write_EEPROM(&k_x10_int, 1, 0x00);
      
      sprintf((char *)Uart_Tx, "ok\n");
      HAL_UART_Transmit(&huart1, Uart_Tx, 3, 50);
    }

  }

//  sprintf((char *)Uart_Tx, "error\n");
//  HAL_UART_Transmit(&huart1, Uart_Tx, 7, 50);
  Uart_Rx_Index = 0;
  memset(Uart_Rx_Buffer, 0, 6);
}


void Linsten_Task(void)
{
  if(TimerTick[3] < HAL_GetTick())
  {
    TimerTick[3] = HAL_GetTick() + 50;
    
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    
    ADC_Val = ADC_GetVal();
    R37_Vol = ADC_Val * 3.3f / 4096;
    
    if(sTime.Hours == Report_Time_Ctrl[0] && sTime.Minutes == Report_Time_Ctrl[1] && sTime.Seconds == Report_Time_Ctrl[2])
    {
      if(Uart_Report_Flag == 0)
      {
        sprintf((char *)Uart_Tx, "%.2f+0.%d+%02d%02d%02d\n", R37_Vol, k_x10_int, Report_Time_Ctrl[0], Report_Time_Ctrl[1], Report_Time_Ctrl[2]);
        HAL_UART_Transmit(&huart1, Uart_Tx, strlen((const char *)Uart_Tx), 250);
        Uart_Report_Flag = 1;
      }
    }
    else
    {
      Uart_Report_Flag = 0;
    }  
  }
  
  if(LED_Ctrl)
  {
    if(TimerTick[4] < HAL_GetTick())
    {
      TimerTick[4] = HAL_GetTick() + 200;
      
      if(ADC_Val * 10 > 4096 * k_x10_int)
      {  
        LED_State ^= 1;
      }
      else
      {
        LED_State = 0;
      }
      LED_Disp(LED_State);
    }    
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
