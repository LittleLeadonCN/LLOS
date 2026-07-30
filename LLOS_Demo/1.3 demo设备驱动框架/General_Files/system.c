#include "system.h"
#include "CH58x_common.h"
#include "llos.h"
#include "hal_gpio.h"
#include "app_LED.h"

/* 串口缓冲区 */
#define UART_BUFFER_LEN		(255)
struct usartBuf_t
{
	uint16_t len;
	uint8_t data[UART_BUFFER_LEN];
	volatile bool rxOK;
}usart_recBuf;

static ll_taskId_t taskUART = LL_ERR_INVALID;
static uint32_t pMemPool[1024];

static void delayms(uint32_t time)
{
    DelayMs(time);
}
static void delayus(uint32_t time)
{
    DelayUs(time);
}

static ll_taskEvent_t TaskUART_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & LL_EVENT(1))
	{
		usart_recBuf.rxOK = true;
		return LL_EVENT(1);
	}

	return 0xFFFF;
}

void System_Init(void)
{
    SysTick_Config(GetSysClock() / 1000);

    GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeIN_PU);

	memset(&usart_recBuf, 0, sizeof(usart_recBuf));
	UART0_ByteTrigCfg(UART_1BYTE_TRIG);
    UART0_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART0_IRQn);

	/* LLOS初始化 */
	struct ll_init_CBs_t CBs = {0};
	struct ll_init_cfgs_t cfgs = {0};
	
	CBs.DelayMs = delayms;
	CBs.systemReset = SYS_ResetExecute;
    CBs.DelayUs = delayus;

	cfgs.taskNum = 10;
	cfgs.timerNum = 5;
    cfgs.TIMER_EN = true;
    cfgs.RTC_EN = true;
	cfgs.alarmNum = 3;
	cfgs.deviceNum = 10;
	cfgs.pPool = pMemPool;
	cfgs.poolSize = sizeof(pMemPool);
	LLOS_Init(&CBs, &cfgs);
	LLOS_Cmd_Init(255, "LittleLeaf", "LLOS", LLOS_VERSION, "00000001");
	
	/* Task初始化 */
	taskUART = LLOS_Register_Events(TaskUART_Events);
    if(taskUART == LL_ERR_INVALID)
    {
    	LL_LOG_E("%s ", "init failed!\r\n", __FUNCTION__);
		while(1);
    }

	LLOS_Device_Register_GPIO();

	App_LED_Init();
	
	App_LED_Ctrl(App_LED_color_R, ll_set);
	LLOS_DelayMs(1000);
	App_LED_Ctrl(App_LED_color_G, ll_set);
	LLOS_DelayMs(1000);
	App_LED_Ctrl(App_LED_color_B, ll_set);

	LLOS_Device_EnumAll();
}

void System_Loop(void)
{
	LLOS_Loop();
	if(usart_recBuf.rxOK)
	{
		printf("%s", usart_recBuf.data);
		LLOS_Cmd_Input((const char *)usart_recBuf.data, usart_recBuf.len);
		memset(&usart_recBuf, 0, sizeof(usart_recBuf));
	}

    if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET)
    {
        DelayMs(1000);
        if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET) Jump2BOOT();
    }
}

__HIGH_CODE
void Jump2BOOT(void)
{
	FLASH_ROM_ERASE(0, EEPROM_BLOCK_SIZE);
	FLASH_ROM_SW_RESET();
	sys_safe_access_enable();
	R16_INT32K_TUNE = 0xFFFF;
	SYS_ResetExecute();
    sys_safe_access_disable();
    while(1);
}

__INTERRUPT
__HIGH_CODE 
void SysTick_Handler(void)
{
    LLOS_Tick_Increase();
    SysTick->SR = 0; 
}

__INTERRUPT
__HIGH_CODE
void UART0_IRQHandler(void)
{
    switch(UART0_GetITFlag())
    {
        case UART_II_LINE_STAT:
        {
            UART0_GetLinSTA();
            break;
        }

        case UART_II_RECV_RDY:
			if(usart_recBuf.len < UART_BUFFER_LEN)
			{
            	usart_recBuf.data[usart_recBuf.len++] = UART0_RecvByte();
			}
			LLOS_Start_Event(taskUART, LL_EVENT(1), LLOS_Ms_To_Tick(10));
            break;

        case UART_II_RECV_TOUT:
            break;

        case UART_II_THR_EMPTY:
            break;

        case UART_II_MODEM_CHG:
            break;

        default:
            break;
    }
}
