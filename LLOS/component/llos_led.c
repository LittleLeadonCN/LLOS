/*
 * @author LittleLeaf All rights reserved
 */
#include "llos_led.h"

static uint16_t ledTaskPeriod;
static uint8_t ll_ledNum;
static bool ready;

static struct ll_led_config_t *ll_led_config;
void LLOS_LED_Tick(uint8_t timerN);

void LLOS_LED_Init(uint8_t timerN, uint16_t ms, struct ll_led_config_t *cfg, uint8_t ledNum)
{
	uint32_t size;

	if (ms == 0 || ledNum == 0 || cfg == NULL)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		while (1);
	}

	ledTaskPeriod = ms;
	ll_ledNum = ledNum;

	size = sizeof(struct ll_led_config_t) * ledNum;
	ll_led_config = LL_LED_MALLOC(size);
	if (ll_led_config == NULL)
	{
		LL_LOG_E("%s ", "cfg malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	memcpy(ll_led_config, cfg, size);

#if LL_LED_USE_LLOS
	LLOS_Timer_Set(timerN, ll_enable, true, LLOS_Ms_To_Tick(ledTaskPeriod), LLOS_LED_Tick);
#endif

	ready = true;
}

void LLOS_LED_Set(uint8_t LEDN, enum ll_led_t mode)
{
	if (LEDN >= ll_ledNum || ll_led_config[LEDN].port == 0 || ll_led_config[LEDN].pinMask == 0)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return;
	}
	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return;
	}
	
	ENTER_CRITICAL();

	ll_IO_t *temp = (ll_IO_t *)ll_led_config[LEDN].port;

	ll_led_config[LEDN].ledBlink.num = 0; /* 关闭正在闪烁的LED */

	switch (mode)
	{
		case ll_led_off:
		{
			if (ll_led_config[LEDN].isActiveHigh)
				*temp &= ~ll_led_config[LEDN].pinMask;
			else
				*temp |= ll_led_config[LEDN].pinMask;
			break;
		}
		case ll_led_on:
		{
			if (ll_led_config[LEDN].isActiveHigh)
				*temp |= ll_led_config[LEDN].pinMask;
			else
				*temp &= ~ll_led_config[LEDN].pinMask;
			break;
		}
		case ll_led_toggle:
		{
			*temp ^= ll_led_config[LEDN].pinMask;
			break;
		}
		default:
		{
			if (ll_led_config[LEDN].isActiveHigh)
				*temp &= ~ll_led_config[LEDN].pinMask;
			else
				*temp |= ll_led_config[LEDN].pinMask;
			break;
		}
	}

	EXIT_CRITICAL();
}

void LLOS_LED_Blink(uint8_t LEDN, uint8_t num, uint8_t duty, uint16_t ms)
{
	if (LEDN >= ll_ledNum || ll_led_config[LEDN].port == 0 || ll_led_config[LEDN].pinMask == 0)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return;
	}
	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return;
	}

	LL_LIMIT_MAX(duty, 100);

	ENTER_CRITICAL();

	ll_led_config[LEDN].ledBlink.num = num;
	ll_led_config[LEDN].ledBlink.duty = duty;
	ll_led_config[LEDN].ledBlink.ms = ms;
	ll_led_config[LEDN].ledBlink.tick = 0;

	EXIT_CRITICAL();
}

void LLOS_LED_Tick(uint8_t timerN)
{
	LL_UNUSED(timerN);

	uint8_t i;

	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return;
	}
	
	for (i = 0; i < ll_ledNum; i++)
	{
		if (ll_led_config[i].ledBlink.num == 0) continue;

		ENTER_CRITICAL();
		
		ll_IO_t *temp = (ll_IO_t *)ll_led_config[i].port; // 获取端口状态
		uint32_t mask = ll_led_config[i].pinMask;
		bool isHighActive = ll_led_config[i].isActiveHigh;
		uint32_t tick = ll_led_config[i].ledBlink.tick;
		uint16_t period = ledTaskPeriod;
		uint16_t blink_ms = ll_led_config[i].ledBlink.ms;
		uint8_t duty = ll_led_config[i].ledBlink.duty;

		uint64_t elapsed_ms = (uint64_t)tick * period;			 // 换算成毫秒
		uint64_t high_time_ms = (uint64_t)blink_ms * duty / 100; // 高电平所占时间

		// 更新 tick 计数器
		tick++;
		ll_led_config[i].ledBlink.tick = tick;

		if (elapsed_ms < high_time_ms)
		{
			if (isHighActive) *temp |= mask;
			else *temp &= ~mask;
		}
		else
		{
			if (isHighActive) *temp &= ~mask;
			else *temp |= mask;
		}

		if (elapsed_ms >= blink_ms)
		{
			ll_led_config[i].ledBlink.tick = 0;
			if (ll_led_config[i].ledBlink.num < 255)
				ll_led_config[i].ledBlink.num--;
		}

		EXIT_CRITICAL();
	}
}
