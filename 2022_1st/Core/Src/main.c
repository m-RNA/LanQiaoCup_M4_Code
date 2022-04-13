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
  
  // ※ 晶振24M
  // ※ 仅供学习使用
  
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "bsp.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    Set_1KHz,
    Set_2KHz
}PWM_f_Type;

typedef enum
{
    PSD,
    STA
}Meau_State_Type;


/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define Task_Delay(ID, MS) \
    if(SW_Timer_Tick[ID] > HAL_GetTick()) return;\
    SW_Timer_Tick[ID] = HAL_GetTick() + MS;

#define UART_RX_LENTH_MAX 50

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
vu32 SW_Timer_Tick[5] = {0};
u8 LED_State = 0;
u8 KEY_Val, KEY_Down, KEY_Old = 0;
u8 UART_Rx_Index = 0;
u8 UART_Rx_Temp = 0;
u8 UART_Rx_Buffer[UART_RX_LENTH_MAX] = {0};
u8 LCD_String_Buffer[21] = {0};

Meau_State_Type Meau_State = PSD;

u8 Password_Disp[3] = {'@', '@', '@'};
u8 Password_Ctrl[3] = {'1', '2', '3'};
u8 Password_Right_Error_Flag = 0;
u8 Password_Error_Times = 0;
u8 Password_Error_Blink_Times = 0;

vu32 i = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void LED1_Task(void);
void LED2_Task(void);
void KEY_Task(void);
void LCD_Task(void);
void UART_Task(void);




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

    LCD_Init();
    LCD_Clear(Black);
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    
    LED_Disp(LED_State);
   
    
    

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &UART_Rx_Temp, 1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    LED1_Task();
    LED2_Task();
    KEY_Task();
    LCD_Task();
    UART_Task();
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Set_PWM_State(PWM_f_Type f)
{
    if(f == Set_1KHz)
    {
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 500); // ※ 先调用这个函数
        __HAL_TIM_SetAutoreload(&htim2, 999);             // 然后在调用这个函数
    }
    else if(f == Set_2KHz)
    {
        __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, 100);
        __HAL_TIM_SetAutoreload(&htim2, 499);
    }
}

void Set_Meau_State(Meau_State_Type State)
{
    LCD_Clear(Black);    
    
    if(State == PSD)
    {
        Meau_State = PSD;
    }
    else
    {
        Meau_State = STA;
        Password_Right_Error_Flag = 1;
        LED_State |= 0x01;
        LED_Disp(LED_State);
        SW_Timer_Tick[3] = HAL_GetTick() + 5000;
    }
}

void LED1_Task(void)
{
    Task_Delay(0, 100);
    
    if(Password_Error_Blink_Times)
    {
        Password_Error_Blink_Times--;
        
        LED_State ^= 2;
    }
    else
    {
        LED_State &= ~0X02;
    }
    LED_Disp(LED_State);
}

void LED2_Task(void)
{
    if(Password_Right_Error_Flag)
    {
        Task_Delay(3, 5000);
        
        Password_Right_Error_Flag = 0;
        
        Set_PWM_State(Set_1KHz);
        Set_Meau_State(PSD);
        
        LED_State &= ~0x01;
        LED_Disp(LED_State);
    }
}

void KEY_Task(void)
{
    Task_Delay(1, 50);
    KEY_Val = KEY_Scan();
    KEY_Down = KEY_Val & (KEY_Val ^ KEY_Old);
    KEY_Old = KEY_Val;
    
    if(Meau_State == 0)
    {
        switch(KEY_Down)
        {
        case 1:
            if(++Password_Disp[0]>'9')
                Password_Disp[0] = '0';
            break;
            
        case 2:
            if(++Password_Disp[1]>'9')
                Password_Disp[1] = '0';
            break;
            
        case 3:
            if(++Password_Disp[2]>'9')
                Password_Disp[2] = '0';                
            break;       
             
        case 4:
            if((Password_Ctrl[0] == Password_Disp[0]) && (Password_Ctrl[1] == Password_Disp[1]) && (Password_Ctrl[2] == Password_Disp[2]))
            {
                Set_PWM_State(Set_2KHz);
                Set_Meau_State(STA);
                Password_Error_Times = 0;
            }
            else
            {
                Password_Error_Times++;
                if(Password_Error_Times >= 3)
                    Password_Error_Blink_Times = 50;
            }

            for(i = 0; i < 3; i++)
            {
                Password_Disp[i] = '@';
            }
            break;        
        }   
    }
}

void LCD_Task(void)
{
    Task_Delay(2, 100);

    if(Meau_State == 0)
    {
        snprintf((char *)LCD_String_Buffer, 20, "       PSD");
        LCD_DisplayStringLine(Line1, LCD_String_Buffer);
        
        snprintf((char *)LCD_String_Buffer, 20, "    B1:%c",Password_Disp[0]);
        LCD_DisplayStringLine(Line3, LCD_String_Buffer);
        snprintf((char *)LCD_String_Buffer, 20, "    B2:%c",Password_Disp[1]);
        LCD_DisplayStringLine(Line4, LCD_String_Buffer);
        snprintf((char *)LCD_String_Buffer, 20, "    B3:%c",Password_Disp[2]);
        LCD_DisplayStringLine(Line5, LCD_String_Buffer);
    }
    else
    {
        snprintf((char *)LCD_String_Buffer, 20, "       STA");
        LCD_DisplayStringLine(Line1, LCD_String_Buffer);
        
        snprintf((char *)LCD_String_Buffer, 20, "    F:2000Hz");
        LCD_DisplayStringLine(Line3, LCD_String_Buffer);
        snprintf((char *)LCD_String_Buffer, 20, "    D:10%%");
        LCD_DisplayStringLine(Line4, LCD_String_Buffer);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(UART_Rx_Index < UART_RX_LENTH_MAX)
    {
        SW_Timer_Tick[4] = HAL_GetTick() + 20;
        UART_Rx_Buffer[UART_Rx_Index] = UART_Rx_Temp;
        UART_Rx_Index++;
    }
    else
    {
        SW_Timer_Tick[4] = HAL_GetTick() - 1; // 强制完成
    }
    
    HAL_UART_Receive_IT(&huart1, &UART_Rx_Temp, 1);
}

void UART_Task(void)
{
    if(UART_Rx_Index == 0) return;
    if(SW_Timer_Tick[4] > HAL_GetTick()) return;
    
    if(UART_Rx_Index == 7)
    {
        if((Password_Ctrl[0] == UART_Rx_Buffer[0]) && (Password_Ctrl[1] == UART_Rx_Buffer[1]) && (Password_Ctrl[2] == UART_Rx_Buffer[2]) 
            && ('-' == UART_Rx_Buffer[3]))
        {
            for(i = 0; i < 3; i++)
            {
                if(((UART_Rx_Buffer[4 + i] <= '9') && (UART_Rx_Buffer[4 + i] >= '0')) == 0)
                {
                    goto PASS;
                }
            }
            for(i = 0; i < 3; i++)
            {
                Password_Ctrl[i] = UART_Rx_Buffer[4 + i];
            }
        }
    }
    
    PASS:
    UART_Rx_Index = 0;
    memset(UART_Rx_Buffer, 0, 7);
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
