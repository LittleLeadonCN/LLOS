 /* 
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.0
 * 修订日期: 2024 09 05
 * 修订日志:
 * N/A
 */
#include <llos_led.h>

struct ledList_t
{
	ll_IO_t port;
	uint32_t ledN;
	uint32_t tick;
	uint16_t ms;
	uint8_t num;
	uint8_t duty;
};

static uint8_t i;
static uint16_t ledTaskPeriod;
static uint8_t ledIndex;
static struct ledList_t ledList[LL_LED_NUM];

static void LLOS_LED_Tick(uint8_t timerN);

void LLOS_LED_Init(ll_ledInit_hook_t ledInit, uint16_t ms, uint8_t timerN)
{
	LOG_D("LLOS LED Init\r\n");
	
	if(ledInit != NULL)ledInit();

	ledTaskPeriod = ms;
	LLOS_Timer_Set(timerN, ll_enable, true, LLOS_Ms_To_Tick(ledTaskPeriod), LLOS_LED_Tick);
}

void LLOS_LED_Set(ll_IO_t port, uint32_t ledN, ll_led_t mode)
{
	if(port == 0)return;
	ll_IO_t *temp = (ll_IO_t *)port;

	/* 关闭正在在闪烁的LED */
	for(i = 0; i < LL_LED_NUM; i++)
	{
		if(ledList[i].port == port && ledList[i].ledN == ledN)
		{
			ledList[i].port = 0;
			ledList[i].ledN = 0;
			ledList[i].num = 0;
		}
	}

	switch(mode)
	{
		case ll_led_off:
		{
			*temp |= ledN;
			break;
		}
		case ll_led_on:
		{
			*temp &= ~ledN;
			break;
		}
		case ll_led_toggle:
		{
			*temp ^= ledN;
			break;
		}
		default:
		{
			*temp |= ledN;
			break;
		}
	}
}

void LLOS_LED_Blink(ll_IO_t port, uint32_t ledN, uint8_t num, uint8_t duty, uint16_t ms)
{
	/* 如果被操作的LED已经在列表中 */
	for(i = 0; i < LL_LED_NUM; i++)
	{
		if(ledList[i].port == port && ledList[i].ledN == ledN)
		{
			ledList[i].port = port;
			ledList[i].ledN = ledN;
			ledList[i].num = num; /* 一亮一灭为一次闪烁 */
			ledList[i].duty = duty;
			ledList[i].ms = ms;
			ledList[i].tick = 0;
			return;
		}
	}
	
	/* 如果被操作的LED没有在列表中则创建一个 */
	if(ledIndex >= LL_LED_NUM)
	{
		LOG_E("> LL_LED_NUM!\r\n");
		return;
	}
	
	ledList[ledIndex].port = port;
	ledList[ledIndex].ledN = ledN;
	ledList[ledIndex].num = num;
	ledList[ledIndex].duty = duty;
	ledList[ledIndex].ms = ms;
	ledList[ledIndex].tick = 0;
	
	ledIndex++;
}

static void LLOS_LED_Tick(uint8_t timerN)
{
	ll_IO_t *temp;
	for(i = 0; i < LL_LED_NUM; i++)
	{
		if(ledList[i].port == 0 || ledList[i].ledN == 0 || ledList[i].num == 0)continue;
		
		temp = (ll_IO_t *)ledList[i].port;
		ledList[i].tick++;

		if(ledList[i].tick * ledTaskPeriod * 100 >= ledList[i].ms * ledList[i].duty)
			*temp |= ledList[i].ledN;
		else
			*temp &= ~ledList[i].ledN;

		if((ledList[i].tick * ledTaskPeriod) >= ledList[i].ms)
		{
			ledList[i].tick = 0;
			if(ledList[i].num < 255)ledList[i].num--;
		}
	}
}
