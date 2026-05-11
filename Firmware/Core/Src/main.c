/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; you may not use this file except in compliance with the
  * License. You can obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h" 

// LED引脚定义
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

// 串口1缓存（调试）
uint8_t recv_buf;
// 串口2缓存（ESP8266）
uint8_t esp_buf;
volatile char esp_recv_data[128];  // ESP数据接收缓存
volatile uint8_t esp_recv_cnt = 0; // 接收计数

// WiFi配置
#define WIFI_SSID     "iPhone"
#define WIFI_PASSWORD "12345677"
#define TCP_PORT      "8080" // TCP服务器端口

/* USER CODE END Includes */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
// ESP8266函数声明
void ESP8266_SendCmd(char *cmd, char *ack, uint16_t timeout);
void ESP8266_Init(void);
void ESP8266_ParseData(char *data);
/* USER CODE END PFP */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init(); // 新增：初始化ESP8266串口
  MX_I2C1_Init();
	
  // 新增：测试USART2是否正常工作
  HAL_Delay(1000);
//  HAL_UART_Transmit(&huart2, (uint8_t *)"AT\r\n", 4, 100);
//	HAL_UART_Transmit(&huart2, (uint8_t *)"AT\r\n", 4, 100);

  printf("USART2 Test OK\r\n");  // 用USART1打印调试信息

  /* USER CODE BEGIN 2 */
  SSD1306_Init(); 
  SSD1306_Clear();
  SSD1306_DrawString(0,0,"ESP8266 Init...");
  
  // 启动串口中断接收
  HAL_UART_Receive_IT(&huart1, &recv_buf, 1);
//	HAL_UART_Receive_IT(&huart2, &esp_buf, 1);

  // 初始化ESP8266
  ESP8266_Init();
	
//  printf("System Ready!\r\n");
//  printf("TCP Server: IP:172.20.10.4 Port:8080\r\n");
  
  SSD1306_Clear();
  SSD1306_DrawString(0,0,"WiFi Connected");
  SSD1306_DrawString(0,2,"TCP Port:8080");
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
    uint8_t ch;

    // 接收ESP8266返回数据
		if(HAL_UART_Receive(&huart2, &ch, 1, 100) == HAL_OK)
		{
				printf("0x%02X ", ch);
		}
    // 本地串口1控制LED
    if(recv_buf == '1' || recv_buf == 'o' || recv_buf == 'O')
    {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
      SSD1306_Clear();
      SSD1306_DrawString(0,0,"WiFi Connected");
      SSD1306_DrawString(0,4,"LED: ON (Local)");
      printf(" LED ON\r\n");
      recv_buf = 0;
    }
    else if(recv_buf == '0' || recv_buf == 'f' || recv_buf == 'F')
    {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      SSD1306_Clear();
      SSD1306_DrawString(0,0,"WiFi Connected");
      SSD1306_DrawString(0,4,"LED: OFF (Local)");
      printf(" LED OFF\r\n");
      recv_buf = 0;
    }
    HAL_Delay(10);
  }
}

/* USER CODE BEGIN 4 */
// printf重定向（调试用）
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
    return ch;
}

// 串口1接收中断（本地调试）
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        if(recv_buf != '\r' && recv_buf != '\n'){}
        HAL_UART_Receive_IT(&huart1, &recv_buf, 1);
    }
    
//    // 串口2接收中断（ESP8266数据）
//    if(huart->Instance == USART2)
//    {
//        // 拼接接收数据
//        // 保存数据
//				if(esp_recv_cnt < 127)
//				{
//						esp_recv_data[esp_recv_cnt++] = esp_buf;
//						esp_recv_data[esp_recv_cnt] = '\0';
//				}
//				else
//				{
//						memset((char*)esp_recv_data, 0, sizeof(esp_recv_data));
//						esp_recv_cnt = 0;
//				}

//				// 调试打印
////				HAL_UART_Transmit(&huart1, &esp_buf, 1, 10);

//				HAL_UART_Receive_IT(&huart2, &esp_buf, 1);
//    }
}

// ESP8266发送AT指令
void ESP8266_SendCmd(char *cmd, char *ack, uint16_t timeout)
{
    char rxbuf[256] = {0};

    uint8_t ch;

    uint16_t idx = 0;

    uint32_t start_time;

    printf("\r\nSEND: %s\r\n", cmd);

    // 清空串口状态
    __HAL_UART_CLEAR_OREFLAG(&huart2);

    // 发送AT
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)cmd,
                      strlen(cmd),
                      100);

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\n",
                      2,
                      100);

    start_time = HAL_GetTick();

    while((HAL_GetTick() - start_time) < timeout)
    {
        // 每次收1字节
        if(HAL_UART_Receive(&huart2,
                            &ch,
                            1,
                            10) == HAL_OK)
        {
            rxbuf[idx++] = ch;

            // 防止越界
            if(idx >= sizeof(rxbuf)-1)
            {
                idx = 0;
                memset(rxbuf,0,sizeof(rxbuf));
            }

            // 打印调试
            HAL_UART_Transmit(&huart1, &ch, 1, 10);

            // 找到ACK
            if(strstr(rxbuf, ack) != NULL)
            {
                printf("\r\nSUCCESS\r\n");
                return;
            }
        }
    }

    printf("\r\nFAILED\r\n");
    printf("RECV:%s\r\n", rxbuf);
}

// ESP8266初始化（联网+开启TCP服务器）
void ESP8266_Init(void)
{
    ESP8266_SendCmd("AT",              "OK",     1000);  // 测试AT
    ESP8266_SendCmd("AT+RST",          "ready",  3000);  // 复位
    HAL_Delay(1000);
    ESP8266_SendCmd("AT+CWMODE=1",     "OK",     1000);  // Station模式
    ESP8266_SendCmd("AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASSWORD"\"", "WIFI GOT IP", 20000); // 连WiFi
	  HAL_Delay(3000);
		// 关闭旧Server
    ESP8266_SendCmd("AT+CIPSERVER=0", "OK", 3000);
    HAL_Delay(1000);
    ESP8266_SendCmd("AT+CIPMUX=1",      "OK",     1000);  // 开启多连接
    ESP8266_SendCmd("AT+CIPSERVER=1,"TCP_PORT, "OK", 2000); // 开启TCP服务器
		HAL_Delay(1000);
}

// 解析ESP8266网络数据（+IPD指令）
void ESP8266_ParseData(char *data)
{
    // 收到网络指令：+IPD,0,1:1 → 开灯
    if(strstr(data, "+IPD") != NULL)
    {
			if(strstr(data, ":1") != NULL)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
            SSD1306_Clear();
            SSD1306_DrawString(0,0,"WiFi Connected");
            SSD1306_DrawString(0,4,"LED: ON (WiFi)");
            printf("🌐 LED ON by WiFi\r\n");
        }
        if(strstr(data, ":0") != NULL)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
            SSD1306_Clear();
            SSD1306_DrawString(0,0,"WiFi Connected");
            SSD1306_DrawString(0,4,"LED: OFF (WiFi)");
            printf("🌐 LED OFF by WiFi\r\n");
        }
    }
}
/* USER CODE END 4 */

// 以下为原有系统函数，无修改
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */


