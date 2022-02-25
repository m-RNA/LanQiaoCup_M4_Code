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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_led_key.h"
#include "lcd.h"

#include "stdbool.h" // bool数据类型
#include "stdio.h"   // 标准输入输出库
#include "string.h"  // 字符串相关标准库
#include "ctype.h"   // 判断 ASCII码 是否为 "数字"
#include "time.h"    // 计算时间相关标准库


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	u8 state;   // 车位是否占用 1-占用 0-空闲
	u8 type;    // 车的类型 'C' / 'V'
	u8 id[5];   // 车的编号 (多余一位存放 0 即'\0',方便打印id)
	time_t sec; // 入车库时刻
} Data_Type;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 软件定时器Tick （MutilTimer 简易版）
__IO u32 KEY_Timer_Tick = 0;
__IO u32 LED_Timer_Tick = 0;
__IO u32 LCD_Timer_Tick = 0;
__IO u32 UART_Timer_Tick = 0;


// 外设相关变量
u8 ucLED = 0x01;

u8 KEY_Val, KEY_Down, KEY_Old = 0;

u8 LCD_String_Buffer[21] = {0};

u8 Uart_Rx_Pointer = 0;
u8 Uart_Rx_Buffer = 0;
u8 Uart_Rx[150] = {0};

u8 Uart_Tx_Buffer[150] = {0};


//题目算法相关变量
bool Menu_State = 0;
bool PWM_Ctrl = 0;

Data_Type Data[8] = {0}; // 车位数据

u8 No_Use_Num = 8;   // 空闲车位
u8 CNBR_Use_Num = 0; // C类占用车位数 
u8 VNBR_Use_Num = 0; // V类占用车位数

u16 CNBR_Fee_X10 = 35; // C类停车费用
u16 VNBR_Fee_X10 = 20; // V类停车费用

char *CNBR_String = "CNBR"; // 方便打印等
char *VNBR_String = "VNBR"; // 方便打印等


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void KEY_Proc(void);
void LED_Proc(void);
void LCD_Proc(void);
void UART_Proc(void);


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
  MX_TIM17_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	LCD_Init();
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	LED_Disp(ucLED);
	

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
	HAL_UART_Receive_IT(&huart1, &Uart_Rx_Buffer, 1);
	
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		KEY_Proc();
		LED_Proc();
		LCD_Proc();
		UART_Proc();

		
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
void KEY_Proc(void)
{
	if(KEY_Timer_Tick > HAL_GetTick()) return; 
	KEY_Timer_Tick = HAL_GetTick() + 50;
	
	KEY_Val = KEY_Scan();
	KEY_Down = KEY_Val & (KEY_Val ^ KEY_Old);
	KEY_Old = KEY_Val;
	
	switch(KEY_Down)
	{
		case 1:
			Menu_State ^= 1;
			LCD_Clear(Black);
			break;
		
		case 2:
			if(Menu_State)
			{
				CNBR_Fee_X10 += 5;
				VNBR_Fee_X10 += 5;
			}
			break;

		case 3:
			if(Menu_State)
			{
				CNBR_Fee_X10 -= 5;
				VNBR_Fee_X10 -= 5;
				if(VNBR_Fee_X10 == 0)
				{
					CNBR_Fee_X10 = 20; //35
					VNBR_Fee_X10 = 5;  //20
				}
			}
			break;

		case 4:
			PWM_Ctrl ^= 1;
			if(PWM_Ctrl)
			{
				__HAL_TIM_SetCompare(&htim17, TIM_CHANNEL_1, 100);
				ucLED |= 0x02;
			}
			else
			{
				__HAL_TIM_SetCompare(&htim17, TIM_CHANNEL_1, 0);
				ucLED &= ~0x02;
			}
			break;		
	}
}

void LED_Proc(void)
{
	if(LED_Timer_Tick > HAL_GetTick()) return;
	LED_Timer_Tick = HAL_GetTick() + 150;
	
	if(No_Use_Num)
	{
		ucLED |= 0x01;
	}
	else
	{
		ucLED &= ~0x01;
	}
	
	LED_Disp(ucLED);
}
 
void LCD_Proc(void)
{
	if(HAL_GetTick() < LCD_Timer_Tick) return;
	LCD_Timer_Tick = HAL_GetTick() + 100;
	
	if(Menu_State == 0)
	{
		sprintf((char *)LCD_String_Buffer, "       Data");
		LCD_DisplayStringLine(Line1, LCD_String_Buffer);
		sprintf((char *)LCD_String_Buffer, "   CNBR:%d", CNBR_Use_Num);
		LCD_DisplayStringLine(Line3, LCD_String_Buffer);	
		sprintf((char *)LCD_String_Buffer, "   VNBR:%d", VNBR_Use_Num);
		LCD_DisplayStringLine(Line5, LCD_String_Buffer);	
		sprintf((char *)LCD_String_Buffer, "   IDLE:%d", No_Use_Num);
		LCD_DisplayStringLine(Line7, LCD_String_Buffer);	
	}
	else
	{
		sprintf((char *)LCD_String_Buffer, "       Para");
		LCD_DisplayStringLine(Line1, LCD_String_Buffer);
		sprintf((char *)LCD_String_Buffer, "   CNBR:%4.2f", CNBR_Fee_X10 / 10.0f);
		LCD_DisplayStringLine(Line3, LCD_String_Buffer);	
		sprintf((char *)LCD_String_Buffer, "   VNBR:%4.2f", VNBR_Fee_X10 / 10.0f);
		LCD_DisplayStringLine(Line5, LCD_String_Buffer);		
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// 每进一次中断就复位软件定时器
	UART_Timer_Tick = HAL_GetTick() + 20;
	
	Uart_Rx[Uart_Rx_Pointer] = Uart_Rx_Buffer;
	Uart_Rx_Pointer++;
	
	HAL_UART_Receive_IT(&huart1, &Uart_Rx_Buffer, 1);
}

// 检查命令格式函数
// 若命令格式正确，则将对应日期以1970-1-1 00:00:00为起点转换为秒
// 若命令格式错误，则返回0
time_t Command_Check(u8 * command)
{
	u8 i;
	struct tm info ={0}; // "time.h" 标准库 结构体
	
	if(command[4] == ':' && command[9] == ':')
	{
		for(i = 10; i < 22; i++)
		{
			if(isdigit(command[i])) // 调用"ctype.h"标准库 判断 ASCII码 是否为 "数字"
				command[i] -= '0';
			else
				return 0;
		}
		
		// 将时间转换为tm结构体格式
		info.tm_year = command[10] * 10 + command[11] + 2000 - 1900; /* years since 1900 */
		info.tm_mon  = command[12] * 10 + command[13] - 1; /* months since January, 0 to 11 */
		info.tm_mday = command[14] * 10 + command[15]; 
		info.tm_hour = command[16] * 10 + command[17]; 
		info.tm_min  = command[18] * 10 + command[19]; 
		info.tm_sec  = command[20] * 10 + command[21];
		
		if(info.tm_mon  < 12 && (info.tm_mday > 0  && info.tm_mday < 32) && \
			 info.tm_hour < 24 &&  info.tm_min  < 60 && info.tm_sec  < 60)
		{
			//调用"time.h" 标准库，将日期以1970-1-1 00:00:00为起点转换为秒
			return mktime(&info);  
		}
	}
	return 0;
}



void UART_Proc(void)
{
	u8 Postion; // 车库位置
	time_t dt_hour = 0; // 存放时间差
	__IO bool dir = 1;  // 出入车库方向 1-In  0-Out
	Data_Type Data_Temp = {0}; //存放临时数据
	
	if(Uart_Rx_Pointer)
	{
		if(UART_Timer_Tick > HAL_GetTick()) return;
		if(Uart_Rx_Pointer != 22) goto SEND_ERROR;
		
		Data_Temp.sec = Command_Check(Uart_Rx);
		if(Data_Temp.sec == 0) goto SEND_ERROR;
		
		// 检查车的类型是否正确
		if(memcmp(CNBR_String, Uart_Rx, 4) == 0)
			Data_Temp.type = 'C';
		else if(memcmp(VNBR_String, Uart_Rx, 4) == 0)
			Data_Temp.type = 'V';
		else
		 goto SEND_ERROR;	
		
		// 拷贝id，方便后续判断与入车库等操作
		memcpy(Data_Temp.id, &Uart_Rx[5], 4);
		
		//....right 命令格式正确
		// 判断是入车库还是出车库 以及找出对应车位
		for(Postion = 0; Postion < 8; Postion++)
		{
			if(Data[Postion].state)
			{
				if(memcmp(Data_Temp.id, Data[Postion].id, 4) == 0)
				{
					dir = 0; // Out-出车库
					break;
				}
			}
		}
		if(Postion == 8)
		{
			if(No_Use_Num == 0) goto SEND_ERROR;	
			for(Postion = 0; Postion < 8; Postion++)
			{
				if(Data[Postion].state == 0)
				{
					dir = 1; // In-入车库
					break;
				}
			}
		}
		
		//....有车位
		if(dir) // 1-In
		{
			Data[Postion] = Data_Temp;
			
			Data[Postion].state = 1;
			
			No_Use_Num--;
			if(Data[Postion].type == 'C')
				CNBR_Use_Num++;
			else if(Data[Postion].type == 'V')
				VNBR_Use_Num++;
		}
		else // 0-Out
		{
			if(Data[Postion].type != Data_Temp.type) goto SEND_ERROR; // id一样，看出入车库前后的type(车型)是否一致
			if(Data[Postion].sec >= Data_Temp.sec) goto SEND_ERROR;   // 入车库时间 比 出车库时间 早

			dt_hour =  (Data_Temp.sec - Data[Postion].sec) /3600.0f + 1; // 不满一小时按一小时算
			
			Data[Postion].state = 0;
			
			No_Use_Num++;
			if(Data[Postion].type == 'C')
			{
				CNBR_Use_Num--;
				sprintf((char *)Uart_Tx_Buffer, "%s:%s:%d:%.2f\n",CNBR_String, Data[Postion].id, dt_hour,dt_hour * CNBR_Fee_X10 / 10.0f);
			}
			else if(Data[Postion].type == 'V')
			{
				VNBR_Use_Num--;		
				sprintf((char *)Uart_Tx_Buffer, "%s:%s:%d:%.2f\n",VNBR_String, Data[Postion].id, dt_hour,dt_hour * VNBR_Fee_X10 / 10.0f);
			}
			HAL_UART_Transmit(&huart1, Uart_Tx_Buffer, strlen((const char *)Uart_Tx_Buffer),50);			
		}

		goto PASS;
		SEND_ERROR:
			sprintf((char *)Uart_Tx_Buffer, "Error\n");
			HAL_UART_Transmit(&huart1, Uart_Tx_Buffer, strlen((const char *)Uart_Tx_Buffer),50);
		PASS:
		
		memset(Uart_Rx, 0, 22);
		Uart_Rx_Pointer = 0;
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
