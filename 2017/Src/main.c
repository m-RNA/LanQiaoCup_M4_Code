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
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "bsp.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define Task_Delay(ID, MS)\
    if(SW_Timer_Tick[ID] > HAL_GetTick()) return; \
    SW_Timer_Tick[ID] = HAL_GetTick() + MS;

#define Stop_Time    2000
#define Door_Time    4000
#define Up_Down_Time 6000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
vu32 SW_Timer_Tick[10] = {0};
u8 LED_State = 0;
u8 KEY_Val, KEY_Down, KEY_Old = 0;
u8 LCD_String_Buffer[21] = {0};

RTC_TimeTypeDef xTime = {0};
RTC_DateTypeDef xDate = {0};

char Dir = 1; // 1:up    -1:down
char Current_Platform = 1;
u8 LCD_Display_CP_Flag = 1;
u8 LCD_Blink_Times = 0;
u8 Elevator_State = 0;

vu32 i = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void KEY_Task(void);
void LCD_Task(void);
void LOGIC_Task(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_RTC_Init();
  MX_TIM3_Init();
  MX_TIM17_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  LCD_Clear(Black);
  LCD_SetBackColor(Black);
  LCD_SetTextColor(White);
  
  LED_Disp(LED_State);
  
  snprintf((char *)LCD_String_Buffer, 20, "  Current Platform ");
  LCD_DisplayStringLine(Line2, LCD_String_Buffer);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    KEY_Task();
    LCD_Task();
    LOGIC_Task();
      
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Elevator_Set_Open(void)
{
    HAL_GPIO_WritePin(Elevator_Door_GPIO_Port, Elevator_Door_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(Elevator_Door_Timer, TIM_CHANNEL_1, 300);
    HAL_TIM_PWM_Start(Elevator_Door_Timer, TIM_CHANNEL_1);
}

void Elevator_Set_Close(void)
{
    HAL_GPIO_WritePin(Elevator_Door_GPIO_Port, Elevator_Door_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(Elevator_Door_Timer, TIM_CHANNEL_1, 250);
    HAL_TIM_PWM_Start(Elevator_Door_Timer, TIM_CHANNEL_1);    
}

void Elevator_Set_Up(void)
{
    HAL_GPIO_WritePin(Elevator_Up_Down_GPIO_Port, Elevator_Up_Down_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(Elevator_Up_Down_Timer, TIM_CHANNEL_1, 800);
    HAL_TIM_PWM_Start(Elevator_Up_Down_Timer, TIM_CHANNEL_1);

}

void Elevator_Set_Down(void)
{
    HAL_GPIO_WritePin(Elevator_Up_Down_GPIO_Port, Elevator_Up_Down_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(Elevator_Up_Down_Timer, TIM_CHANNEL_1, 600);
    HAL_TIM_PWM_Start(Elevator_Up_Down_Timer, TIM_CHANNEL_1);    
}

void Elevator_Set_Stop(void)
{    
    HAL_TIM_PWM_Stop(Elevator_Door_Timer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(Elevator_Up_Down_Timer, TIM_CHANNEL_1);
}

void KEY_Task(void)
{
    if(Elevator_State) return; // 仅在状态0时，按键可用
    
    Task_Delay(1, 50);

    KEY_Val = KEY_Scan();
    KEY_Down = KEY_Val & (KEY_Val ^ KEY_Old);
    KEY_Old = KEY_Val;
    
    if(KEY_Down)
    {
        if(Current_Platform != KEY_Down)
        {
            LED_State |= 1<<(KEY_Down - 1);
            LED_Disp(LED_State);
            
            SW_Timer_Tick[3] = HAL_GetTick() + 1000;
        }
    }
}

void LCD_Task(void)
{
    Task_Delay(2, 250);
    
    if(LCD_Blink_Times)
    {
        LCD_Blink_Times--;
        LCD_Display_CP_Flag ^= 1;
    }
    
    if(LCD_Display_CP_Flag)
    {
        snprintf((char *)LCD_String_Buffer, 20, "          %d   ",Current_Platform);
        LCD_DisplayStringLine(Line5, LCD_String_Buffer);        
    }
    else
    {
        LCD_ClearLine(Line5);
    }

    HAL_RTC_GetTime(&hrtc, &xTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &xDate, RTC_FORMAT_BIN);    
    
    snprintf((char *)LCD_String_Buffer, 20, "       %02d:%02d:%02d      ", xTime.Hours, xTime.Minutes, xTime.Seconds);
    LCD_DisplayStringLine(Line8, LCD_String_Buffer);
    
//    snprintf((char *)LCD_String_Buffer, 20, "          %d   ",Elevator_State);
//    LCD_DisplayStringLine(Line9, LCD_String_Buffer);
}

void LOGIC_Task(void)
{
    switch(Elevator_State)
    {
    case 0: // 待机 检测
        if(LED_State & 0x0F)
        {
            if(SW_Timer_Tick[3] > HAL_GetTick()) return;
            Elevator_State = 1;        
        }
    
        break;
    
    case 1: // 设置关门
        Elevator_Set_Close();
        SW_Timer_Tick[3] = HAL_GetTick() + Door_Time;
        Elevator_State = 2;
        
        break;    
    
    case 2: // 等待关门完成
        if(SW_Timer_Tick[3] > HAL_GetTick()) return;
        Elevator_State = 3;

        break;    
    
    case 3: // 判断升降 设置PWM
        Elevator_Set_Stop();
    
        if((LED_State & 0x0F) > (1<<(Current_Platform - 1)))
        {
            Dir = 1;
            Elevator_Set_Up();
            SW_Timer_Tick[3] = HAL_GetTick() + Up_Down_Time;

        }
        else if((LED_State & 0x0F) < (1<<(Current_Platform - 1)))
        {
            Dir = (char)-1; // ※ 这样没警告
            Elevator_Set_Down();
            SW_Timer_Tick[3] = HAL_GetTick() + Up_Down_Time;
        }
        
        Elevator_State = 4;

        break;    
    
    case 4: // 等待升降一层完成 同时流水灯
        if(SW_Timer_Tick[4] < HAL_GetTick())
        {
            SW_Timer_Tick[4] = HAL_GetTick() + 125;
            
            if(++i == 8) i = 0;
            
            if(Dir == 1)
            {
                LED_State = ((u8)(0xF<<i) & 0xF0) | (LED_State & 0x0F);
                LED_Disp(LED_State);
            }
            else if (Dir == (char)-1) // ※ 必须的 ※ 这里 -1 需要强制转换为char类型
            {
                LED_State = ((u8)(0x0F00>>i) & 0xF0) | (LED_State & 0x0F);
                LED_Disp(LED_State);            
            }        
        }

        if(SW_Timer_Tick[3] > HAL_GetTick()) return;
        Elevator_State = 5;
    
        break;    
    
    case 5: // 到达一层
        LED_State &=  0x0F;
        LED_Disp(LED_State);
        i = 0;
    
        Current_Platform += Dir;
    
        if((LED_State & 0x0F) & (1<<(Current_Platform - 1))) // 该层为目标楼层
        {
            LED_State &= ~(1<<(Current_Platform - 1));
            LED_Disp(LED_State);
            
            Elevator_Set_Stop();
            Elevator_Set_Open();
            SW_Timer_Tick[3] = HAL_GetTick() + Door_Time;
            
            LCD_Blink_Times = 4;
            Elevator_State = 6;
        }
        else  // 该层bushi目标楼层
        {
            SW_Timer_Tick[3] = HAL_GetTick() + Up_Down_Time; // 再运行6s
            Elevator_State = 4;
        }
    
        break;
    
    case 6: // 等待开门完成
        if(SW_Timer_Tick[3] > HAL_GetTick()) return;
        Elevator_State = 7;
    
        break;
    
    case 7: // 判断 还 有无目标楼层
        Elevator_Set_Stop();
        if((LED_State & 0x0F) == 0)
        {
            Elevator_State = 0; // 没有目标楼层 待机
        }
        else
        {
            Elevator_State = 8; // 还有目标楼层 停2s电梯
        }
        break;    

    case 8: // 设置停止电梯2s
        SW_Timer_Tick[3] = HAL_GetTick() + Stop_Time;
        Elevator_State = 9;
        break;
    
    case 9: // 等待2s跑完
        if(SW_Timer_Tick[3] > HAL_GetTick()) return;
        Elevator_State = 1;  // 去先关门
    
        break;    
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
