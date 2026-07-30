#include "system.h"
#include "CH58x_common.h"
#include "llos.h"
#include "llos_led.h"
#include "llos_key.h"
#include "taskUART.h"

/* LED */
#define PORT_LED				(uint32_t)&R32_PB_OUT
#define PIN_LEDB				LL_BV(0)
#define PIN_LEDR				LL_BV(1)
#define PIN_LEDG				LL_BV(2)

/* KEY */
#define PORT_KEY				(uint32_t)&R32_PB_PIN
#define MASK_KEY_1 				LL_BV(3)
#define MASK_KEY_2 				LL_BV(5)
#define MASK_KEY_3 				LL_BV(19)
#define MASK_KEY 				(uint32_t)(MASK_KEY_1 | MASK_KEY_2 | MASK_KEY_3)

/* 串口缓冲区 */
#define UART_BUFFER_LEN		(255)
struct usartBuf_t
{
	uint16_t len;
	uint8_t data[UART_BUFFER_LEN];
	volatile bool rxOK;
}usart_recBuf;

static ll_taskId_t taskUARTRec = LL_ERR_INVALID;
static uint32_t pMemPool[1024];

static void KeyCB(uint8_t portN, bool isUp, struct ll_keyWhich_t *keyWhich);

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

	/* LLOS LED模块初始化 */
	GPIOB_SetBits(PIN_LEDR | PIN_LEDG | PIN_LEDB);
	GPIOB_ModeCfg(PIN_LEDR | PIN_LEDG | PIN_LEDB, GPIO_ModeOut_PP_20mA);
	
	struct ll_led_config_t led_config[4] = {0};
	
	led_config[0].port = PORT_LED;
	led_config[0].pinMask = PIN_LEDR;
	led_config[0].isActiveHigh = false;
	
	led_config[1].port = PORT_LED;
	led_config[1].pinMask = PIN_LEDG;
	led_config[1].isActiveHigh = false;
	
	led_config[2].port = PORT_LED;
	led_config[2].pinMask = PIN_LEDB;
	led_config[2].isActiveHigh = false;
	
	LLOS_LED_Init(0, 10, led_config, 3);
	
	/* LLOS KEY模块初始化 */
	GPIOB_ModeCfg(MASK_KEY, GPIO_ModeIN_PU);

	struct ll_keyConfig_t keyConfig = {0};
	keyConfig.port = PORT_KEY;
	keyConfig.pinMask = MASK_KEY;

	LLOS_Key_Init(1, 20, 200, 800, &keyConfig, 1, KeyCB);

	/* Task初始化 */
	taskUARTRec = LLOS_Register_Events(TaskUART_Events);
    if(taskUARTRec == LL_ERR_INVALID)
    {
    	LL_LOG_E("%s ", "init failed!\r\n", __FUNCTION__);
		while(1);
    }
	Task_UART_Init();

	printf("init complete!\r\n");
}

void System_Loop(void)
{
	LLOS_Loop();
	if(usart_recBuf.rxOK)
	{
		printf("%s", usart_recBuf.data);
		LLOS_FIFO_Input(&fifoUART, (const uint8_t *)usart_recBuf.data, usart_recBuf.len);
		memset(&usart_recBuf, 0, sizeof(usart_recBuf));
	}

    if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET)
    {
        DelayMs(1000);
        if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET) Jump2BOOT();
    }
}

static void KeyCB(uint8_t portN, bool isUp, struct ll_keyWhich_t *keyWhich)
{
	uint32_t i;
	uint8_t data[2] = {0};
	
	i = LLOS_FIFO_Output(&fifoUART, data, 1);
	if(i > 0)printf("%s\r\n", (char *)data);
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
			LLOS_Start_Event(taskUARTRec, LL_EVENT(1), LLOS_Ms_To_Tick(10));
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
