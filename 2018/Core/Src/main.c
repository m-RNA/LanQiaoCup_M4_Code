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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "bsp.h"
#include "bsp_i2c.h"

#include "stdio.h"
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
    StandBy,
    Setting,
    Running,
    Pause,
} Timer_State_Type;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// 软件定时器
#define Task_Delay(Task_ID, Tick)               \
    if (SW_Timer_Tick[Task_ID] > HAL_GetTick()) \
        return;                                 \
    SW_Timer_Tick[Task_ID] = HAL_GetTick() + Tick;

// 地址偏移 每道题不会覆盖之前的数据
#define EEPROM_OFFSET 4
    
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static vu32 SW_Timer_Tick[6] = {0};

u8 LCD_Update_Flag = 1;
u8 LCD_String_Buffer[21] = {0};
u8 KEY_Val, KEY_Down, KEY_Old = 0, KEY_Up = 0;
u8 LED_State = 0x00;

u8 Timer_Index = 0;
Timer_State_Type Timer_State = StandBy;
u8 Timer_Data_Disp[3] = {0};
u32 Timer_Convert_Sec = 0; // 将时间转换为秒
u32 Timer_Tick_Dt = 0;     // ※ 保存现场的变量
u8 Setting_Index = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void LED_Task(void);
void LCD_Task(void);
void Timer_Task(void);
void KEY_With_Logic_Task(void);
void Timer_Set_State(Timer_State_Type State);

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
    MX_TIM3_Init();
    /* USER CODE BEGIN 2 */
    LCD_Init();
    LCD_Clear(Black);
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);

    LED_Disp(LED_State);

    I2CInit();
    // EEPROM_Write((u8 *)Timer_Data_Ctrl, 15, EEPROM_OFFSET);
    Timer_Set_State(StandBy);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 800);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        LED_Task();
        LCD_Task();
        Timer_Task();
        KEY_With_Logic_Task();

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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
void Timer_Set_State(Timer_State_Type State)
{
    switch (State)
    {
    case StandBy:
        Timer_State = StandBy;
        Timer_Tick_Dt = 0;

        LED_State = 0;
        LED_Disp(LED_State);
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);

        EEPROM_Read((u8 *)Timer_Data_Disp, 3, Timer_Index * 3 + EEPROM_OFFSET);
        break;

    case Setting:
        Timer_State = Setting;
        Timer_Tick_Dt = 0;

        LED_State = 0;
        LED_Disp(LED_State);
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
        break;

    case Running:
        if (Timer_Data_Disp[0] || Timer_Data_Disp[1] || Timer_Data_Disp[2])
        {
            Timer_State = Running;
            SW_Timer_Tick[3] = HAL_GetTick() + Timer_Tick_Dt; // ※ 恢复现场

            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        }
        break;

    case Pause:
        Timer_State = Pause;
        Timer_Tick_Dt = SW_Timer_Tick[3] - HAL_GetTick(); // ※ 保存现场

        LED_State = 0;
        LED_Disp(LED_State);
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
        break;
    }
}

void LED_Task(void)
{
    if (Timer_State == Running)
    {
        Task_Delay(4, 250);
        LED_State ^= 1;
        LED_Disp(LED_State);
    }
}

void LCD_Task(void)
{
    Task_Delay(0, 100);

    sprintf((char *)LCD_String_Buffer, "   No:%d", Timer_Index + 1);
    LCD_DisplayStringLine(Line2, LCD_String_Buffer);

    sprintf((char *)LCD_String_Buffer, "      %02d:%02d:%02d    ", Timer_Data_Disp[0], Timer_Data_Disp[1], Timer_Data_Disp[2]);
    if (Timer_State == Setting)
    {
        switch (Setting_Index)
        {
        case 0:
            LCD_SetBackColor(Blue);
            LCD_DisplayChar(Line5, 319 - 16 * 6, LCD_String_Buffer[6]);
            LCD_DisplayChar(Line5, 319 - 16 * 7, LCD_String_Buffer[7]);
            LCD_SetBackColor(Black);
            LCD_DisplayChar(Line5, 319 - 16 * 12, LCD_String_Buffer[12]);
            LCD_DisplayChar(Line5, 319 - 16 * 13, LCD_String_Buffer[13]);
            break;

        case 1:
            LCD_SetBackColor(Blue);
            LCD_DisplayChar(Line5, 319 - 16 * 9, LCD_String_Buffer[9]);
            LCD_DisplayChar(Line5, 319 - 16 * 10, LCD_String_Buffer[10]);
            LCD_SetBackColor(Black);
            LCD_DisplayChar(Line5, 319 - 16 * 6, LCD_String_Buffer[6]);
            LCD_DisplayChar(Line5, 319 - 16 * 7, LCD_String_Buffer[7]);
            break;

        case 2:
            LCD_SetBackColor(Blue);
            LCD_DisplayChar(Line5, 319 - 16 * 12, LCD_String_Buffer[12]);
            LCD_DisplayChar(Line5, 319 - 16 * 13, LCD_String_Buffer[13]);
            LCD_SetBackColor(Black);
            LCD_DisplayChar(Line5, 319 - 16 * 9, LCD_String_Buffer[9]);
            LCD_DisplayChar(Line5, 319 - 16 * 10, LCD_String_Buffer[10]);
            break;
        }
    }
    else
    {
        LCD_DisplayStringLine(Line5, LCD_String_Buffer);
    }

    switch (Timer_State)
    {
    case StandBy:
        sprintf((char *)LCD_String_Buffer, "       StandBy");
        break;

    case Setting:
        sprintf((char *)LCD_String_Buffer, "       Setting");
        break;

    case Running:
        sprintf((char *)LCD_String_Buffer, "       Running");
        break;

    case Pause:
        sprintf((char *)LCD_String_Buffer, "        Pause ");
        break;
    }
    LCD_DisplayStringLine(Line8, LCD_String_Buffer);
}

void Timer_Task(void)
{
    if (Timer_State == Running)
    {
        Task_Delay(3, 1000);
        Timer_Convert_Sec = ((Timer_Data_Disp[0] * 60 + Timer_Data_Disp[1]) * 60 + Timer_Data_Disp[2]);

        if (Timer_Convert_Sec)
        {
            Timer_Convert_Sec--;

            Timer_Data_Disp[0] = Timer_Convert_Sec / 3600;
            Timer_Data_Disp[1] = Timer_Convert_Sec / 60 % 60;
            Timer_Data_Disp[2] = Timer_Convert_Sec % 60;
        }
        else
        {
            Timer_Set_State(StandBy);
        }
    }
}

void KEY_With_Logic_Task(void)
{
    Task_Delay(1, 50);

    KEY_Val = KEY_Scan();
    KEY_Down = KEY_Val & (KEY_Val ^ KEY_Old);
    KEY_Up = ~KEY_Val & (KEY_Val ^ KEY_Old);
    KEY_Old = KEY_Val;

    if (KEY_Down)
    {
        SW_Timer_Tick[2] = HAL_GetTick() + 800;
    }

    switch (Timer_State)
    {
    case StandBy:
        if (SW_Timer_Tick[2] > HAL_GetTick())  
        {
            switch (KEY_Up) // 短按处理
            {
            case 1:
                if (++Timer_Index == 5)
                    Timer_Index = 0;
                Timer_Set_State(StandBy);
                break;

            case 2:
                Timer_Set_State(Setting);
                break;

            case 4:
                Timer_Set_State(Running);
                break;
            }
        }
        break;

    case Setting:
        if (SW_Timer_Tick[2] > HAL_GetTick()) 
        {
            switch (KEY_Up) // 短按处理
            {
            case 2:
                if (++Setting_Index == 3)
                    Setting_Index = 0;
                break;

            case 3:
                if (Setting_Index == 0)
                {
                    if (++Timer_Data_Disp[Setting_Index] == 24)
                        Timer_Data_Disp[Setting_Index] = 0;
                }
                else
                {
                    if (++Timer_Data_Disp[Setting_Index] == 60)
                        Timer_Data_Disp[Setting_Index] = 0;
                }
                break;

            case 4:
                Timer_Set_State(Running);
                break;
            }
        }
        else
        {
            switch (KEY_Val) // 长按处理
            {
            case 2:
                for (u8 i = 0; i < 3; i++)
                {
                    EEPROM_Write(&Timer_Data_Disp[i], 1, EEPROM_OFFSET + Timer_Index * 3 + i); // ※ 每个字节要单独存储，不易出错
                }
                Timer_Set_State(StandBy);
                break;
                
            case 3:
                if (Setting_Index == 0)
                {
                    if (++Timer_Data_Disp[Setting_Index] >= 24)
                        Timer_Data_Disp[Setting_Index] = 0;
                }
                else
                {
                    if (++Timer_Data_Disp[Setting_Index] >= 60)
                        Timer_Data_Disp[Setting_Index] = 0;
                }
                break;

            case 4:
                Timer_Set_State(StandBy);
                break;
            }
        }
        break;

    case Running:
        if (SW_Timer_Tick[2] > HAL_GetTick()) 
        {
            switch (KEY_Up) // 短按处理
            {
            case 4:
                Timer_Set_State(Pause);
                break;
            }
        }
        else
        {
            switch (KEY_Val) // 长按处理
            {
            case 4:
                Timer_Set_State(StandBy);
                break;
            }
        }
        break;

    case Pause:
        if (SW_Timer_Tick[2] > HAL_GetTick()) 
        {
            switch (KEY_Up) // 短按处理
            {
            case 4:
                Timer_Set_State(Running);
                break;
            }
        }
        else
        {
            switch (KEY_Val) // 长按处理
            {
            case 4:
                Timer_Set_State(StandBy);
                break;
            }
        }
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
