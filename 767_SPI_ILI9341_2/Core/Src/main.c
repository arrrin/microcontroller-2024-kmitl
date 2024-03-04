/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "ILI9341_Touchscreen.h"
#include "am2030.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#include "group.h"
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
uint8_t redVal = 0;
uint8_t greenVal = 0;
uint8_t blueVal = 0;
uint8_t redDecimal = 0;
uint8_t blueDecimal = 0;
uint8_t greenDecimal = 0;
uint8_t someVal = 0;
uint16_t h = 0; //color up to chart
char someValToString[50] = "";
char redPercent[50] = "";
char greenPercent[50] = "";
char bluePercent[50] = "";
uint16_t result = 0;
char resultHex[100] = "";
uint8_t onPage = 0;
uint8_t isRenderImage = 0;
uint8_t step = 0;
HAL_StatusTypeDef status;


/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

uint16_t CRC16_2(uint8_t *ptr, uint8_t length) {
	uint16_t crc = 0xFFFF;
	uint8_t s = 0x00;
	while (length--) {
		crc ^= *ptr++;
		for (s = 0; s < 8; s++) {
			if ((crc & 0x01) != 0) {
				crc >>= 1;
				crc ^= 0xA001;
			} else
				crc >>= 1;
		}
	}

	return crc;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t rgb888_to_rgb565(uint8_t red8, uint8_t green8, uint8_t blue8) {
// Convert 8-bit red to 5-bit red.
	uint8_t red5 = (uint8_t) ((red8 / 255.0) * 31);
// Convert 8-bit green to 6-bit green.
	uint8_t green6 = (uint8_t) ((green8 / 255.0) * 63);
// Convert 8-bit blue to 5-bit blue.
	uint8_t blue5 = (uint8_t) ((blue8 / 255.0) * 31);

// Shift the red value to the left by 11 bits.
	uint16_t red5_shifted = (uint16_t) (red5) << 11;
// Shift the green value to the left by 5 bits.
	uint16_t green6_shifted = (uint16_t) (green6) << 5;

// Combine the red, green, and blue values.
	uint16_t rgb565 = red5_shifted | green6_shifted | blue5;

	return rgb565;
}

void vprint(const char *fmt, va_list argp) {
	char string[200];
	if (0 < vsprintf(string, fmt, argp)) // build string
			{
		HAL_UART_Transmit(&huart3, (uint8_t*) string, strlen(string), 0xffffff); // send message via UART
	}
}

void my_printf(const char *fmt, ...) // custom printf() function
{
	va_list argp;
	va_start(argp, fmt);
	vprint(fmt, argp);
	va_end(argp);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	char str[100];
	uint8_t cmdBuffer[3];
	uint8_t dataBuffer[8];
	char humidVal[100];
	char tempVal[100];
	/* USER CODE END 1 */
	/* Enable the CPU Cache */

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

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
	MX_SPI5_Init();
	MX_TIM1_Init();
	MX_RNG_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_I2C1_Init();
	MX_USART3_UART_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	cmdBuffer[0] = 0x03;
	cmdBuffer[1] = 0x00;
	cmdBuffer[2] = 0x04;

	ILI9341_Init(); //initial driver setup to drive ili9341
	Am2320_HandleTypeDef Am2320_;
	Am2320_ = am2320_Init(&hi2c1,  0x01 << 1);
	float temperature, humidity;
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//Send Temp & Humid via UART2
		//Wake up sensor

		if (onPage == 0) {
			ILI9341_Fill_Screen(WHITE);
			onPage = 2;
		}
		am2320_GetTemperatureAndHumidity(&Am2320_, &temperature, &humidity);
		ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
		sprintf(tempVal, "%.1f C", temperature);
		sprintf(humidVal, "%.1f%%RH", humidity);
		ILI9341_Draw_Text(tempVal, 30, 30, BLACK, 2, WHITE);
		ILI9341_Draw_Text(humidVal, 180, 30, BLACK, 2, WHITE);
		ILI9341_Draw_Filled_Circle(160, 30, 15, h); // x , y , r ,color
		ILI9341_Draw_Filled_Circle(30, 80, 15, 0XF800);
		ILI9341_Draw_Rectangle(50, 70, 100, 20, 0XF81F); //x , y , lx, ly ,color
		if (redVal <= 10) {
			ILI9341_Draw_Rectangle(50, 70, redVal * 10, 20, 0XF800); //x , y , lx, ly ,color
			sprintf(redPercent, "%d%%    ", redVal * 10);
			ILI9341_Draw_Text(redPercent, 180, 70, BLACK, 2, WHITE);

		}
		////////////////////////////////////green///////////////////////////////////
		ILI9341_Draw_Filled_Circle(30, 130, 15, 0X07E0);
		ILI9341_Draw_Rectangle(50, 120, 100, 20, 0XC618); //x , y , lx, ly ,color
		if (greenVal <= 10) {
			ILI9341_Draw_Rectangle(50, 120, greenVal * 10, 20, 0X07E0);	//x , y , lx, ly ,color
			sprintf(greenPercent, "%d%%    ", greenVal * 10);
			ILI9341_Draw_Text(greenPercent, 180, 120, BLACK, 2, WHITE);

		}
		////////////////////////////////////blue///////////////////////////////////
		ILI9341_Draw_Filled_Circle(30, 180, 15, 0X001F);
		ILI9341_Draw_Rectangle(50, 170, 100, 20, 0X07FF);//x , y , lx, ly ,color
		if (blueVal <= 10) {
			ILI9341_Draw_Rectangle(50, 170, blueVal * 10, 20, 0X001F);//x , y , lx, ly ,color
			sprintf(bluePercent, "%d%%    ", blueVal * 10);
			ILI9341_Draw_Text(bluePercent, 180, 170, BLACK, 2, WHITE);
			sprintf(someValToString, "%d", blueDecimal);

		}

		result = rgb888_to_rgb565(redDecimal, greenDecimal, blueDecimal);
		sprintf(resultHex, "0X%04X", result);
		ILI9341_Draw_Text(resultHex, 180, 210, BLACK, 2, WHITE);
		sscanf(resultHex, "%x", &h);
		ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
		ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
		if (TP_Touchpad_Pressed()) {

			uint16_t x_pos = 0;
			uint16_t y_pos = 0;

			HAL_GPIO_WritePin(GPIOB, LD3_Pin | LD2_Pin, GPIO_PIN_SET);

			uint16_t position_array[2];

			if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK) {
				x_pos = position_array[0];
				y_pos = position_array[1];
				if ((x_pos >= 146 && x_pos <= 170)
						&& (y_pos >= 6 && y_pos <= 34))	//RedCheck
						{
					redVal += 1;
					redDecimal += 25;
					if (redVal == 11) {
						redVal = 0;
						redDecimal = 0;
					}
				} else if ((x_pos >= 90 && x_pos <= 119)
						&& (y_pos >= 9 && y_pos <= 60))	//GreenCheck
						{
					greenVal += 1;
					greenDecimal += 25;
					if (greenVal == 11) {
						greenVal = 0;
						greenDecimal = 0;
					}
				} else if ((x_pos >= 44 && x_pos <= 69)
						&& (y_pos >= 9 && y_pos <= 42))	//GreenCheck
						{
					blueVal += 1;
					blueDecimal += 25;
					if (blueVal == 11) {
						blueVal = 0;
						blueDecimal = 0;
					}
				} else if ((x_pos >= 200 && x_pos <= 231)
						&& (y_pos >= 145 && y_pos <= 176))	//Center Top
						{
					isRenderImage = 0;
					onPage = 1;
				}
				ILI9341_Draw_Filled_Circle(x_pos, y_pos, 2, BLACK);
				ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
				ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
			}

		} else {
			HAL_GPIO_WritePin(GPIOB, LD3_Pin | LD2_Pin, GPIO_PIN_RESET);
		}
		while (onPage == 1) {
			if (isRenderImage == 0) {
				HAL_TIM_Base_Start_IT(&htim3);
				ILI9341_Fill_Screen(WHITE);
				ILI9341_Draw_Image((const char*) image_data_Image,
				SCREEN_HORIZONTAL_1);
				isRenderImage = 1;

			}
			ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
			ILI9341_Draw_Text("Group No.19", 135, 40, h, 2, WHITE);
			ILI9341_Draw_Text("Arin 1063", 135, 80, h, 2, WHITE);
			ILI9341_Draw_Text("Peet 1052", 135, 120, h, 2, WHITE);
			ILI9341_Draw_Text("pond 1050", 135, 160, h, 2, WHITE);
			ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
			if (TP_Touchpad_Pressed()) {

				uint16_t x_pos = 0;
				uint16_t y_pos = 0;
				uint16_t position_array[2];
				if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK) {
					x_pos = position_array[0];
					y_pos = position_array[1];
					if ((x_pos >= 83 && x_pos <= 178)
							&& (y_pos >= 8 && y_pos <= 99))	//IMG check
							{
						onPage = 0;
					}

				}
			}
		}
		HAL_TIM_Base_Stop_IT(&htim3);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 200;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
