#include "taskUART.h"
#include "system.h"
#include "llos_led.h"

#define EVENT_UART_LOOP		LL_EVENT(0)
#define EVENT_UART_LED		LL_EVENT(1)

static uint8_t bufferUART[1024];
llos_fifo_t fifoUART;

static ll_taskId_t taskUART = LL_ERR_INVALID;

ll_taskEvent_t Task_UART_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & EVENT_UART_LOOP)
	{
		printf("Used Size = %d\r\n", LLOS_FIFO_Get_UsedSize(&fifoUART));
		printf("Available Size = %d\r\n", LLOS_FIFO_Get_AvailableSize(&fifoUART));
		
		LLOS_Start_Event(taskUART, EVENT_UART_LOOP, LLOS_Ms_To_Tick(2000));
		return EVENT_UART_LOOP;
	}
	
	if(events & EVENT_UART_LED)
	{
		uint32_t i;
		i = LLOS_FIFO_Get_AvailableSize(&fifoUART);
		if(i < 999)LLOS_LED_Set(0, ll_led_on);
		else LLOS_LED_Set(0, ll_led_off);

		LLOS_Start_Event(taskUART, EVENT_UART_LED, LLOS_Ms_To_Tick(10));
		return EVENT_UART_LED;
	}

	return 0xFFFF;
}

void Task_UART_Init(void)
{
	LLOS_FIFO_Init(&fifoUART, bufferUART, sizeof(bufferUART));
	
    taskUART = LLOS_Register_Events(Task_UART_Events);
    if(taskUART == LL_ERR_INVALID)
    {
    	LL_LOG_E("%s ", "init failed!\r\n", __FUNCTION__);
		while(1);
    }
	
	LLOS_Start_Event(taskUART, EVENT_UART_LOOP, taskUART);
	LLOS_Start_Event(taskUART, EVENT_UART_LED, taskUART);
}
