#include "system.h"
#include "main.h"
#include "usart.h"
#include "llos_led.h"

#include "task0.h"

/* ========================[变量声明]========================= */
/* LED */
#define GPIO_ODR_OFFSET			(12)
#define PORT_LED				GPIOB_BASE + GPIO_ODR_OFFSET
#define PIN_LED0				LL_LEDn(6)
#define PIN_LED1				LL_LEDn(7)
#define PIN_LED2				LL_LEDn(8)
#define PIN_LED3				LL_LEDn(9)

/* 串口缓冲区 */
#define UART_BUFFER_LEN		(255)
struct usartBuf_t
{
	uint16_t len;
	uint8_t data[UART_BUFFER_LEN];
	volatile bool rxOK;
}usart1_recBuf;

/* ========================[函数声明]========================= */

void System_Init(void)
{
	/* 串口初始化*/
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart1, usart1_recBuf.data, UART_BUFFER_LEN);
	usart1_recBuf.rxOK = false;
	
	/* LLOS初始化 */
	LLOS_Init(NULL, HAL_Delay, NULL);
	LLOS_Register_System_Reset(NVIC_SystemReset);
	
	/* LLOS LED模块初始化 */
	LLOS_LED_Init(NULL, 10, 0);
	LLOS_LED_Blink(PORT_LED, PIN_LED0, 255, 10, 500);
	
	/* Task初始化 */
	Task0_Init();
	
	printf("Task Num: %d\r\n", LLOS_Get_TaskNum());
}

void System_Loop(void)
{
	LLOS_Loop();
	if(usart1_recBuf.rxOK)
	{
		usart1_recBuf.rxOK = false;
		printf("%s\r\n", usart1_recBuf.data);
	}
}

int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

void HAL_UART_IdleCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE)) 
		{
			__HAL_UART_CLEAR_OREFLAG(&huart1);
			__HAL_UART_CLEAR_IDLEFLAG(&huart1);
			__HAL_UART_CLEAR_NEFLAG(&huart1);
			HAL_UART_DMAStop(&huart1);
			
			HAL_UART_Receive_DMA(&huart1, usart1_recBuf.data, UART_BUFFER_LEN);
			
			return;	
		}
		if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != 0x00U)
		{
			__HAL_UART_CLEAR_IDLEFLAG(&huart1);
			HAL_UART_DMAStop(&huart1);	
		
			extern DMA_HandleTypeDef hdma_usart1_rx;
			usart1_recBuf.len = UART_BUFFER_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
			usart1_recBuf.rxOK = true;
			HAL_UART_Receive_DMA(&huart1, usart1_recBuf.data, UART_BUFFER_LEN);
		}
	}
}
