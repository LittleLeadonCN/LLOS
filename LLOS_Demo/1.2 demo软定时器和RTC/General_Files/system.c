#include "system.h"
#include "CH58x_common.h"
#include "llos.h"

static uint32_t pMemPool[1024];

static void timerCB(uint8_t timerN);
static void alarmCB(uint8_t alarmN);

static void delayms(uint32_t time)
{
    DelayMs(time);
}
static void delayus(uint32_t time)
{
    DelayUs(time);
}

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
	
	/* OS定时器初始化 */
	LLOS_Timer_Set(0, ll_enable, true, LLOS_Ms_To_Tick(100), timerCB);
	
	/* RTC初始化 */
	LLOS_RTC_SetDate(2026, 07, 29, 23, 59, 50);
	LLOS_RTC_SetAlarm(2026, 07, 30, 00, 00, 05, alarmCB, 0);

	LL_LOG_I("Task Num: %d\r\n", LLOS_Get_TaskNum());
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

static void timerCB(uint8_t timerN)
{
	static uint8_t sec = 0;

	struct ll_calendar_t calenda;
	LLOS_RTC_GetDate(&calenda);
	
	if(sec != calenda.sec)
	{
		LL_LOG_I("Timer ID: %d --- %04d-%02d-%02d %02d:%02d:%02d %d\r\n", timerN,
			calenda.year, calenda.mon, calenda.day, calenda.hour, calenda.min, calenda.sec, calenda.week);
		sec = calenda.sec;
	}
}

static void alarmCB(uint8_t alarmN)
{
	LL_LOG_W("%s ", "ALARM!!! ID: %d\r\n", __FUNCTION__, alarmN);
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
