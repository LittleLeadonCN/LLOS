#include "system.h"
#include "main.h"
#include "usart.h"

/* ========================[变量声明]========================= */
/* 串口缓冲区 */
#define UART_BUFFER_LEN		(255)
struct usartBuf_t
{
	uint16_t len;
	uint8_t data[UART_BUFFER_LEN];
	volatile bool rxOK;
}usart1_recBuf;

/* ========================[函数声明]========================= */
static void timerCB(uint8_t timerN);
static void alarmCB(uint8_t alarmN);

void System_Init(void)
{
	/* 串口初始化*/
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart1, usart1_recBuf.data, UART_BUFFER_LEN);
	usart1_recBuf.rxOK = false;
	
	/* LLOS初始化 */
	LLOS_Init(NULL, HAL_Delay, NULL);
	LLOS_Register_System_Reset(NVIC_SystemReset);
	
	/* OS定时器初始化 */
	LLOS_Timer_Set(0, ll_enable, true, LLOS_Ms_To_Tick(100), timerCB);
	
	/* RTC初始化 */
	LLOS_RTC_SetDate(2024, 12, 31, 23, 59, 50);
	LLOS_RTC_SetAlarm(2025, 01, 01, 00, 00, 05, alarmCB, 0);
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

static void timerCB(uint8_t timerN)
{
	uint8_t sec;
	
	sec = ll_calendar.sec;
	
	LLOS_RTC_GetDate();
	
	if(sec != ll_calendar.sec)
	{
		LOG_I("Timer ID: %d --- %04d-%02d-%02d %02d:%02d:%02d %d\r\n", timerN,
			ll_calendar.year, ll_calendar.mon, ll_calendar.day, ll_calendar.hour, ll_calendar.min, ll_calendar.sec, ll_calendar.week);
	}
}

static void alarmCB(uint8_t alarmN)
{
	LOG_W("ALARM!!! ID: %d\r\n", alarmN);
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
