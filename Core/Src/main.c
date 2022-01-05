/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "stdio.h"
#include "motor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

extern int flag;
extern int count;
extern int cnt[3];
float RGB_Scale[3];		//储存3个RGB比例因子
float distance_forward = 0;
float distance_right = 0;
extern uint16_t Ultrasonic_us1;
extern uint16_t Ultrasonic_us2;
int Front_flag = 0;
int Right_flag = 0;
int KEY1; 
int KEY2;
int KEY3;
int KEY4;
int KEY5;
int turn_flag = 0; // straight is 0, turn is 1
int turn_dir = 0; // left is even, right is odd
int turn_cnt = 0; // turn counter
int black_cnt = 0; //全黑计数
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
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
	MX_TIM5_Init();
    MX_TIM6_Init();
    CarGoStr();
//	printf("ok");
  /* USER CODE BEGIN 2 */
	
	
	HAL_TIM_Base_Start_IT(&htim1);	//使能定时器1
	HAL_TIM_Base_Start(&htim2);	//使能定时器2
//	HAL_TIM_Base_Start_IT(&htim5);
//		printf("ok");
	HAL_TIM_IC_Start_IT(&htim5,TIM_CHANNEL_1);   //开始捕获TIM5的通道1
	HAL_TIM_IC_Start_IT(&htim5,TIM_CHANNEL_2);   //开始捕获TIM5的通道1
	__HAL_TIM_ENABLE_IT(&htim5,TIM_IT_UPDATE);   //使能更新中断

	//选择2%的输出比例因子
	S0_L;
	S1_H;
	LED_ON;		//打开四个白色LED，进行白平衡
	HAL_Delay(3000);		//延时三秒，等待识别
	//通过白平衡测试，计算得到白色物的RGB值255与0.5秒内三色光脉冲数的RGB比例因子
	for(int i=0;i<3;i++)
	{
		RGB_Scale[i] = 255.0/cnt[i];
		printf("Scaler: %5lf  \r\n", RGB_Scale[i]);
	}
	//红绿蓝三色光分别对应的0.5s内TCS3200输出脉冲数，乘以相应的比例因子就是我们所谓的RGB标准值
	//打印被测物体的RGB值
	
	for(int i=0; i<3; i++)
	{
		printf("%d \r\n", (int) (cnt[i]*RGB_Scale[i]));
	}
	printf("White Balance Done!\r\n");
	//白平衡结束
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		flag = 0;
		count = 0;
//		printf("while loop is running!\r\n");
		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
	
		for(int i=0; i<3; i++)
		{
			if(i==0)
				printf("RGB = (");
			if(i==2)
				printf("%d)\r\n",(int) (cnt[i]*RGB_Scale[i]));
			else
				printf("%d, ", (int) (cnt[i]*RGB_Scale[i]));
		}
        KEY1 = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13);//左1
        KEY2 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_6);//左2
        KEY3 = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14);//中
        KEY4 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7);//右2
        KEY5 = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15);//右1
        printf("%d, %d, %d, %d, %d\r\n",KEY1, KEY2, KEY3, KEY4, KEY5);
        Sonic_Trig(20);

        if(KEY1 !=0 & KEY2 !=0 & KEY3 == 0 & KEY4 != 0 & KEY5 !=0){//中间灯检测到黑线,其余检测到白线，最正中情况
            CarGo();
        }
        else if(KEY1 !=0 & KEY2 ==0 & KEY3 == 0 & KEY4 != 0 & KEY5 !=0){//左二和中间灯检测到黑线,其余检测到白线
            CarGo();
        }
        else if(KEY1 !=0 & KEY2 !=0 & KEY3 == 0 & KEY4 == 0 & KEY5 !=0){//右二和中间灯检测到黑线,其余检测到白线
            CarGo();
        }
        else if(KEY1 !=0 & KEY2 !=0 & KEY3 == 0 & KEY4 == 0 & KEY5 ==0){//左边两灯检测到白线，中间，右2和右1检测到黑线
            TurnRightB();//大右转
        }
        else if(KEY1 !=0 & KEY2 !=0 & KEY3 != 0 & KEY4 == 0 & KEY5 ==0){//左边两灯以及中间检测到白线，右2和右1检测到黑线
            TurnRightB();//大右转
        }
        else if(KEY1 != 0 & KEY2!=0 & KEY3 !=0 & KEY4 ==0 & KEY5 !=0){//左侧三灯以及右1检测到白线，右2灯检测到黑线
            TurnRight();//小右转
        }
        else if(KEY1 != 0 & KEY2!=0 & KEY3 !=0 & KEY4 !=0 & KEY5 ==0){//左侧四灯检测到白线，右侧两灯检测到黑线
            TurnRightB();//大右转
        }
        else if (KEY1==0 & KEY2 ==0 & KEY3 == 0 & KEY4 != 0 & KEY5 !=0){//右边两灯检测到白线，中间以及左2，左1检测到黑线
            TurnLeftB();//大左转
        }
        else if(KEY1==0 & KEY2 ==0 & KEY3 != 0 & KEY4 != 0 & KEY5 !=0){//左1左2检测到黑线，其余检测到白线
            TurnLeftB();//大左转
        }
        else if (KEY1 !=0 & KEY2 ==0 & KEY3 != 0 & KEY4 != 0 & KEY5 !=0){//左二检测到黑线，其余检测到白线
            TurnLeft();//小左转
        }
        else if (KEY1 ==0 & KEY2 !=0 & KEY3 != 0 & KEY4 != 0 & KEY5 !=0){//左1检测到黑线，其余检测到白线
            TurnLeftB();//大左转
        }
        else if(KEY1 !=0 & KEY2 !=0 & KEY3 != 0 & KEY4 != 0 & KEY5 !=0){//全部检测为白色
            GoBack();//倒车
        }
        else if(KEY1 ==0 & KEY2 ==0 & KEY3 == 0 & KEY4 == 0 & KEY5 ==0){//检测到全为黑线
            black_cnt += 1;
            if(black_cnt >= 16){//通过黑线计数大于临界值停车
                Stop();
            }
            else{
                CarGo();
            }
        }
        else{
            Stop();
        }




		
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
