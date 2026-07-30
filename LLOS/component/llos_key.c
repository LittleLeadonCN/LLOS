/*
 * @author LittleLeaf All rights reserved
 */
#include "llos_key.h"

#define IO_STATUS			(*((ll_IO_t *)ll_keyConfig[i].port) & ll_keyConfig[i].pinMask)

typedef enum
{
	FSM_keyUp,
	FSM_keyDown,
} FSM_state_t;

struct FSM_value_t
{
	FSM_state_t state;
	enum ll_keyEvent_t event;
	uint32_t pinState;
	uint16_t pressTime;
	uint8_t flag;
};

static uint8_t ll_portNum;
static uint16_t ll_overTime, ll_longPressTime;
static uint16_t keyTaskPeriod;
static ll_keyCB_t keyChangeCB;
static bool ready;

static struct ll_keyWhich_t *ll_keyWhich;
static struct ll_keyConfig_t *ll_keyConfig;
static struct FSM_value_t *FSM_value;

void LLOS_Key_Tick(uint8_t timerN);

void LLOS_Key_Init(uint8_t timerN, uint16_t ms, uint16_t overTime, uint16_t longPressTime, struct ll_keyConfig_t *cfg, uint8_t portNum, ll_keyCB_t keyCB)
{
	uint32_t size;

	keyTaskPeriod = ms;
	keyChangeCB = keyCB;

	if (portNum == 0 || overTime == 0 || longPressTime == 0 || cfg == NULL)
	{
		LL_LOG_E("%s ", "para error!\r\n", __FUNCTION__);
		while (1);
	}

	ll_portNum = portNum;
	ll_overTime = overTime;
	ll_longPressTime = longPressTime;

	size = sizeof(struct ll_keyWhich_t) * ll_portNum;
	ll_keyWhich = LL_KEY_MALLOC(size);
	if (ll_keyWhich == NULL)
	{
		LL_LOG_E("%s ", "keyWhich malloc failed!\r\n", __FUNCTION__);
		while (1);
	}

	size = sizeof(struct ll_keyConfig_t) * ll_portNum;
	ll_keyConfig = LL_KEY_MALLOC(size);
	if (ll_keyConfig == NULL)
	{
		LL_LOG_E("%s ", "keyConfig malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	memcpy(ll_keyConfig, cfg, size);

	size = sizeof(struct FSM_value_t) * ll_portNum;
	FSM_value = LL_KEY_MALLOC(size);
	if (FSM_value == NULL)
	{
		LL_LOG_E("%s ", "FSM_value malloc null!\r\n", __FUNCTION__);
		while (1);
	}
	memset(FSM_value, 0, size);

#if LL_KEY_USE_LLOS
	LLOS_Timer_Set(timerN, ll_enable, true, LLOS_Ms_To_Tick(keyTaskPeriod), LLOS_Key_Tick);
#endif

	ready = true;
}

void LLOS_Key_Tick(uint8_t timerN)
{
	uint8_t i;

	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return;
	}

	for (i = 0; i < ll_portNum; i++)
	{
		bool need_callback = false;
		bool callback_isUp = false;

		if (ll_keyConfig[i].port == 0 || ll_keyConfig[i].pinMask == 0)
			continue;

		ENTER_CRITICAL();

		switch (FSM_value[i].state)
		{
			case FSM_keyUp:
			{
				FSM_value[i].flag = 0;
				FSM_value[i].event = ll_key_event_NULL;
				FSM_value[i].pinState = IO_STATUS; 							/* 保存IO电平 */
				if (FSM_value[i].pinState != ll_keyConfig[i].pinMask)		/* IO电平发生变化 */
				{
					FSM_value[i].state = FSM_keyDown;		 				/* 状态机进入按键按下状态 */
					FSM_value[i].event = ll_key_event_Click; 				/* 默认为单击 */
				}
				break;
			}
			case FSM_keyDown:
			{
				FSM_value[i].pressTime += keyTaskPeriod;					/* 统计按下时间 */

				if (IO_STATUS == ll_keyConfig[i].pinMask) 					/* 按下后按键弹起 */
				{
					FSM_value[i].flag = 1;

					/* 弹起超过pressTime则认为按键检测结束 */
					if (FSM_value[i].pressTime >= ll_keyWhich[i].pressTime + ll_overTime)
					{
						FSM_value[i].flag = 0;
						FSM_value[i].state = FSM_keyUp;			   			/* 状态机进入按键弹起状态 */
						ll_keyWhich[i].event = FSM_value[i].event; 			/* 保存事件 */
						if (ll_keyWhich[i].pressTime > ll_longPressTime)
						{
							ll_keyWhich[i].event = ll_key_event_LongPress; 	/* 保存按键长按事件 */
						}

						need_callback = true;
						callback_isUp = true;
						FSM_value[i].pressTime = 0;
						break;
					}
				}
				else	/* 按下后按键未弹起 */
				{
					ll_keyWhich[i].pin = ~IO_STATUS; 						/* 保存键值 */
					ll_keyWhich[i].pressTime = FSM_value[i].pressTime;		/* 保存长按时间 */
					need_callback = true;
					callback_isUp = false;
				}

				if (IO_STATUS == FSM_value[i].pinState && FSM_value[i].flag == 1)
				{
					FSM_value[i].event++;
					FSM_value[i].flag = 0; 									/* 该标志保证只有弹起了才能进行事件增加 */
				}
				break;
			}
			default:
			{
				FSM_value[i].state = FSM_keyUp; 							/* 状态机切换到按键弹起状态 */
				break;
			}
		}

		EXIT_CRITICAL();

		if (need_callback && keyChangeCB != NULL)
		{
			keyChangeCB(i, callback_isUp, &ll_keyWhich[i]);
			need_callback = false;
		}
	}
}
