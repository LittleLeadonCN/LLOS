#include "system.h"
#include "CH58x_common.h"
#include "llos_led.h"
#include "llos_key.h"

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

/* ========================[变量声明]========================= */
/* 串口缓冲区 */
#define UART_BUFFER_LEN		(255)
struct usartBuf_t
{
	uint16_t len;
	uint8_t data[UART_BUFFER_LEN];
	volatile bool rxOK;
}usart1_recBuf;

static uint32_t pMemPool[1024];

static void delayms(uint32_t time)
{
    DelayMs(time);
}
static void delayus(uint32_t time)
{
    DelayUs(time);
}

static void KeyCB(uint8_t portN, bool isUp, struct ll_keyWhich_t *keyWhich);;

void System_Init(void)
{
    SysTick_Config(GetSysClock() / 1000);

    GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeIN_PU);

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
	
	LLOS_LED_Blink(0, 10, 10, 500);
	LLOS_LED_Blink(1, 255, 15, 100);
	LLOS_LED_Set(2, ll_led_on);
	
	/* LLOS KEY模块初始化 */
	GPIOB_ModeCfg(MASK_KEY, GPIO_ModeIN_PU);

	struct ll_keyConfig_t keyConfig = {0};
	keyConfig.port = PORT_KEY;
	keyConfig.pinMask = MASK_KEY;

	LLOS_Key_Init(1, 20, 200, 800, &keyConfig, 1, KeyCB);
	
	LL_LOG_I("Pool get used size: %d\r\n", LLOS_Pool_GetUsedSize());
}

void System_Loop(void)
{
	LLOS_Loop();

    if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET)
    {
        DelayMs(1000);
        if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET) Jump2BOOT();
    }
}

static void KeyCB(uint8_t portN, bool isUp, struct ll_keyWhich_t *keyWhich)
{
	uint32_t key = keyWhich->pin & MASK_KEY;
	if(isUp)
	{
		printf("Mask: %08X Event: %d Time: %d\r\n", key, keyWhich->event, keyWhich->pressTime);
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

