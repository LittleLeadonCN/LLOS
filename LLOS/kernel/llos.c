/*
 * @author LittleLeaf All rights reserved
 */
#include "llos.h"

struct ll_eventCB_list_t
{
	ll_tick_t startTick[32];
	ll_taskEvent_t oldEvents;
	ll_taskEvent_t newEvents;
	ll_eventCB_t eventCB;
};
struct ll_timerCB_list_t
{
	ll_tick_t initTick;
	ll_tick_t tick;
	ll_tick_t nextTick;
	ll_newState_t newState;
	bool isPeriodic;
	ll_timerCB_t timerCB;
};
struct ll_alarm_list_t
{
	uint32_t sec;
	ll_alarmCB_t CB;
};
struct ll_rtc_t
{
	volatile uint32_t sec;
};

static volatile ll_tick_t sysTick;
static ll_taskId_t taskIndex; // 0xFF保留不可用
static ll_deviceId_t deviceIndex;
static struct ll_rtc_t rtc;

static struct ll_init_CBs_t ll_init_CBs;
static struct ll_init_cfgs_t ll_init_cfgs;
static uint8_t tickPeriod = 1;
static ll_LP_hook_t ll_LP_CB;

static struct ll_eventCB_list_t *eventCB_list;
static void const **msg_list;
static struct ll_timerCB_list_t *timerCB_list;
static struct ll_alarm_list_t *alarm_list;
static struct ll_device_t *device_list;
static struct ll_calendar_t ll_calendar;

// CPU占用率统计
static uint16_t ticksAll = 1;
static uint16_t ticksBusy;
static uint8_t cpuUsagePercent;
static volatile uint16_t cpuUsageTick;

#if LL_COROUTINES_MAX > 0
static struct ll_coroutine_task_t ll_coroutine_tasks[LL_COROUTINES_MAX];
static uint8_t ll_coroutine_count;
static uint32_t ll_coroutine_events;

void LLOS_Coroutine_Register(struct ll_coroutine_ctx_t *ctx, ll_coroutine_CB CB, void *arg)
{
    if (ll_coroutine_count < LL_COROUTINES_MAX)
    {
        ll_coroutine_tasks[ll_coroutine_count].ctx = ctx;
        ll_coroutine_tasks[ll_coroutine_count].CB = CB;
        ll_coroutine_tasks[ll_coroutine_count].arg = arg;
        ll_coroutine_count++;
        ctx->state = 0;
    }
}
void LLOS_Coroutine_SetEvents(uint32_t events)
{
    ll_coroutine_events |= events;
}
uint32_t LLOS_Coroutine_GetEvents(void)
{
    return ll_coroutine_events;
}
#endif

void LLOS_Init(struct ll_init_CBs_t *osCB, struct ll_init_cfgs_t *cfgs)
{
	uint32_t size;

	ll_init_CBs.systemReset = osCB->systemReset;
	ll_init_CBs.DelayMs = osCB->DelayMs;
	ll_init_CBs.DelayUs = osCB->DelayUs;
	ll_init_CBs.errHandle = osCB->errHandle;
	ll_init_CBs.criticalEnter = osCB->criticalEnter;
	ll_init_CBs.criticalEXIT = osCB->criticalEXIT;

	ll_init_cfgs.TIMER_EN = cfgs->TIMER_EN;
	ll_init_cfgs.RTC_EN = cfgs->RTC_EN;
	ll_init_cfgs.taskNum = cfgs->taskNum;
	ll_init_cfgs.timerNum = cfgs->timerNum;
	ll_init_cfgs.alarmNum = cfgs->alarmNum;
	ll_init_cfgs.deviceNum = cfgs->deviceNum;

	ll_init_cfgs.pPool = cfgs->pPool;
	ll_init_cfgs.poolSize = cfgs->poolSize;
	if (ll_init_cfgs.pPool == NULL)
	{
		LL_LOG_E("%s ", "pPool NULL!\r\n", __FUNCTION__);
		while (1);
	}
	if (ll_init_cfgs.poolSize <= sizeof(uint32_t))
	{
		LL_LOG_E("%s ", "poolSize <= sizeof(uint32_t)!\r\n", __FUNCTION__);
		while (1);
	}
	memset(ll_init_cfgs.pPool, 0, ll_init_cfgs.poolSize);

	size = sizeof(struct ll_eventCB_list_t) * ll_init_cfgs.taskNum;
	eventCB_list = LLOS_malloc(size);
	if (eventCB_list == NULL)
	{
		LL_LOG_E("%s ", "eventCB list malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	memset(eventCB_list, 0, size);

	size = sizeof(void *) * ll_init_cfgs.taskNum;
	msg_list = LLOS_malloc(size);
	if (msg_list == NULL)
	{
		LL_LOG_E("%s ", "msg list malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	memset(msg_list, 0, size);

	size = sizeof(struct ll_timerCB_list_t) * ll_init_cfgs.timerNum;
	timerCB_list = LLOS_malloc(size);
	if (timerCB_list == NULL && ll_init_cfgs.timerNum != 0)
	{
		LL_LOG_E("%s ", "timerCB list malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	memset(timerCB_list, 0, size);

	size = sizeof(struct ll_alarm_list_t) * ll_init_cfgs.alarmNum;
	alarm_list = LLOS_malloc(size);
	if (alarm_list == NULL && ll_init_cfgs.alarmNum != 0)
	{
		LL_LOG_E("%s ", "alarm list malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	memset(alarm_list, 0, size);

	size = sizeof(struct ll_device_t) * ll_init_cfgs.deviceNum;
	device_list = LLOS_malloc(size);
	if (device_list == NULL && ll_init_cfgs.deviceNum != 0)
	{
		LL_LOG_E("%s ", "device list malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	memset(device_list, 0, size);

	LL_LOG_D("", "H  e  l  l  o   --     --  \r\n");
	LL_LOG_D("", "|      |      |    | |     \r\n");
	LL_LOG_D("", "|      |      |    |  --   \r\n");
	LL_LOG_D("", "|      |      |    |     | \r\n");
	LL_LOG_D("", " ----   ----    --    --   \r\n");
	LL_LOG_D("", "LLOS version: %s\r\n", LLOS_VERSION);
}

ll_taskId_t LLOS_Register_Events(ll_eventCB_t eventCB)
{
	if (taskIndex >= ll_init_cfgs.taskNum)
	{
		LL_LOG_E("%s ", "> taskNum!\r\n", __FUNCTION__);
		return LL_ERR_INVALID;
	}
	if(eventCB == NULL)
	{
		LL_LOG_E("%s ", "CB NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}

	eventCB_list[taskIndex].eventCB = eventCB;
	taskIndex++;

	return taskIndex - 1;
}

void LLOS_Loop(void)
{
	int8_t i, j;
	ll_taskEvent_t event;

	// CPU统计相关
	uint64_t tempAll = sysTick;
    uint64_t tempBusy;

	tempBusy = sysTick;
	for (i = 0; i < taskIndex; i++)
	{
		if (eventCB_list[i].eventCB == NULL) continue;

		LL_BIT_SET(eventCB_list[i].oldEvents, eventCB_list[i].newEvents);
		eventCB_list[i].newEvents = 0;

		for (j = 31; j >= 0; j--) // 从0x80000000开始保证消息事件的优先级最高
		{
			event = LL_BIT_READ(eventCB_list[i].oldEvents, LL_EVENT(j));

			if (event && tempBusy >= eventCB_list[i].startTick[j])
			{
				if (ll_LP_CB != NULL) ll_LP_CB(i, event, false);
				eventCB_list[i].oldEvents &= ~eventCB_list[i].eventCB(i, event);
				ticksBusy += sysTick - tempBusy;
				if (ll_LP_CB != NULL) ll_LP_CB(i, event, true);
			}
		}
	}

#if LL_COROUTINES_MAX > 0
	// 协程处理
	for (int i = 0; i < ll_coroutine_count; i++)
    {
        struct ll_coroutine_task_t *task = &ll_coroutine_tasks[i];
        struct ll_coroutine_ctx_t *ctx = task->ctx;
        if (LLOS_CR_IsFinished(ctx)) continue;
		tempBusy = sysTick;
        task->CB(ctx, task->arg);
		ticksBusy += sysTick - tempBusy;
    }
#endif

	ticksAll += sysTick - tempAll;
	if(cpuUsageTick > 1000)
	{
		cpuUsagePercent = (ticksBusy * 100 + ticksAll / 2) / ticksAll;
		cpuUsageTick = 0;
		ticksAll = 1;
		ticksBusy = 0;
	}
}

void LLOS_Start_Event(ll_taskId_t taskId, ll_taskEvent_t events, ll_tick_t tick)
{
	uint8_t i;
	
	if (taskId >= ll_init_cfgs.taskNum)
	{
		LL_LOG_E("%s ", "> taskNum!\r\n", __FUNCTION__);
		return;
	}
	
	LL_BIT_SET(eventCB_list[taskId].newEvents, events);

	for (i = 0; i < 32; i++)
	{
		if ((events >> i) & 0x00000001)
		{
			eventCB_list[taskId].startTick[i] = sysTick + tick;
		}
	}
}
void LLOS_Stop_Event(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if (taskId >= ll_init_cfgs.taskNum)
	{
		LL_LOG_E("%s ", "> taskNum!\r\n", __FUNCTION__);
		return;
	}

	LL_BIT_CLEAR(eventCB_list[taskId].newEvents, events);
	LL_BIT_CLEAR(eventCB_list[taskId].oldEvents, events);
}

uint8_t LLOS_Get_TaskNum(void)
{
	return taskIndex;
}
uint64_t LLOS_Get_SysTick(void)
{
	uint64_t tick;

    tick = sysTick;

	return tick;
}
uint8_t LLOS_Get_CPU_Usage(void)
{
    return cpuUsagePercent;
}
const char *LLOS_Get_Version(void)
{
	return LLOS_VERSION;
}

void LLOS_Tick_Increase(void)
{
	uint8_t i;

	sysTick++;
	cpuUsageTick++;

	// RTC
	static uint16_t rtc_counter = 0;
	rtc_counter++;
	if (ll_init_cfgs.RTC_EN && rtc_counter >= (1000 / tickPeriod))
	{
		rtc_counter = 0;
		rtc.sec++;
		
		for (i = 0; i < ll_init_cfgs.alarmNum; i++)
		{
			if (rtc.sec >= alarm_list[i].sec && alarm_list[i].CB != NULL)
			{
				alarm_list[i].CB(i);
				alarm_list[i].CB = NULL;
			}
		}
	}

	// timer
	if (ll_init_cfgs.TIMER_EN)
	{
		for (i = 0; i < ll_init_cfgs.timerNum; i++)
		{
			if (timerCB_list[i].newState && timerCB_list[i].timerCB != NULL && sysTick >= timerCB_list[i].nextTick)
			{
				timerCB_list[i].timerCB(i);
				if (timerCB_list[i].isPeriodic == false)
					timerCB_list[i].newState = ll_disable;
				else
					timerCB_list[i].nextTick = sysTick + timerCB_list[i].tick;
			}
		}
	}
}

ll_err_t LLOS_Msg_Send(ll_taskId_t taskId, const void *pMsg)
{
	if (taskId >= ll_init_cfgs.taskNum)
	{
		LL_LOG_E("%s ", "> taskNum!\r\n", __FUNCTION__);
		return LL_ERR_INVALID;
	}

	msg_list[taskId] = pMsg;

	LLOS_Start_Event(taskId, LL_EVENT_MSG, 0);

	return LL_ERR_SUCCESS;
}
const void *LLOS_Msg_Receive(ll_taskId_t taskId)
{
    const void *msg;
    msg = msg_list[taskId];
	return msg;
}
ll_err_t LLOS_Msg_Clear(ll_taskId_t taskId)
{
	if (taskId >= ll_init_cfgs.taskNum)
	{
		LL_LOG_E("%s ", "> taskNum!\r\n", __FUNCTION__);
		return LL_ERR_INVALID;
	}
	msg_list[taskId] = NULL;
	return LL_ERR_SUCCESS;
}

ll_tick_t LLOS_Ms_To_Tick(uint32_t ms)
{
	return (ms / tickPeriod);
}

void LLOS_DelayMs(uint32_t time)
{
	if (ll_init_CBs.DelayMs == NULL)
	{
		LL_LOG_E("%s ", "CB NULL!\r\n", __FUNCTION__);
		return;
	}
	ll_init_CBs.DelayMs(time);
}
void LLOS_DelayUs(uint32_t time)
{
	if (ll_init_CBs.DelayUs == NULL)
	{
		LL_LOG_E("%s ", "CB NULL!\r\n", __FUNCTION__);
		return;
	}
	ll_init_CBs.DelayUs(time);
}

ll_err_t LLOS_Timer_Set(uint8_t timerN, ll_newState_t newState, bool isPeriodic, ll_tick_t tick, ll_timerCB_t timerCB)
{
	if (timerN >= ll_init_cfgs.timerNum || !ll_init_cfgs.TIMER_EN)
	{
		LL_LOG_E("%s ", "> timerNum or timer disabled!\r\n", __FUNCTION__);
		return LL_ERR_INVALID;
	}
	if (tick == 0) return LL_ERR_PARA;

	timerCB_list[timerN].initTick = sysTick;
	timerCB_list[timerN].newState = newState;
	timerCB_list[timerN].isPeriodic = isPeriodic;
	timerCB_list[timerN].tick = tick;
	timerCB_list[timerN].timerCB = timerCB;
	timerCB_list[timerN].nextTick = sysTick + tick;

	return LL_ERR_SUCCESS;
}

static const uint8_t table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
static const uint8_t table_mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static bool RTC_Is_Leap_Year(uint16_t year);
static uint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day);

ll_err_t LLOS_RTC_SetDate(uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
	uint16_t t;
	uint32_t toSec = 0;

	if (year < 2000 || year > 2099)
		return LL_ERR_PARA;
	for (t = 2000; t < year; t++)
	{
		if (RTC_Is_Leap_Year(t))
			toSec += 31622400; // 366*24*60*60
		else
			toSec += 31536000; // 365*24*60*60
	}

	mon -= 1;
	for (t = 0; t < mon; t++)
	{
		toSec += table_mon[t] * 86400; // 60*60*24
		if (RTC_Is_Leap_Year(year) && t == 1)
			toSec += 86400; // 闰年二月多一天
	}

	toSec += (day - 1) * 86400;
	toSec += hour * 3600;
	toSec += min * 60;
	toSec += sec;

	rtc.sec = toSec;

	return LL_ERR_SUCCESS;
}
void LLOS_RTC_GetDate(struct ll_calendar_t *calenda)
{
	static uint16_t daycnt = 0;
	uint32_t temp = 0;
	uint16_t temp1 = 0;
	uint32_t toTime = rtc.sec;

	temp = toTime / 86400; // 计算出天数

	if (daycnt != temp) // 如果天数已更新
	{
		daycnt = temp;
		temp1 = 2000;
		while (temp >= 365)
		{
			if (RTC_Is_Leap_Year(temp1))
			{
				if (temp >= 366)
					temp -= 366;
				else
					break;
			}
			else
			{
				temp -= 365;
			}
			temp1++;
		}
		ll_calendar.year = temp1;

		temp1 = 0;
		while (temp >= 28) // 超过一个月，此时temp为一年中的第几天
		{
			if (RTC_Is_Leap_Year(ll_calendar.year) && temp1 == 1) // 闰年的2月份
			{
				if (temp >= 29)
					temp -= 29;
				else
					break;
			}
			else
			{
				if (temp >= table_mon[temp1])
					temp -= table_mon[temp1];
				else
					break;
			}
			temp1++;
		}

		ll_calendar.mon = temp1 + 1;
		ll_calendar.day = temp + 1;
	}

	toTime %= 86400;
	ll_calendar.hour = toTime / 3600;
	ll_calendar.min = (toTime % 3600) / 60;
	ll_calendar.sec = (toTime % 3600) % 60;

	ll_calendar.week = RTC_Get_Week(ll_calendar.year, ll_calendar.mon, ll_calendar.day);

	*calenda = ll_calendar;
}
ll_err_t LLOS_RTC_SetAlarm(uint16_t year, uint8_t mon, uint8_t day,
						   uint8_t hour, uint8_t min, uint8_t sec, ll_alarmCB_t alarmCB,
						   uint8_t alarmN)
{
	uint16_t t;
	uint32_t toSec = 0;

	if (alarmN >= ll_init_cfgs.alarmNum || !ll_init_cfgs.RTC_EN)
	{
		LL_LOG_E("%s ", "> alarmNum or RTC disabled!\r\n", __FUNCTION__);
		return LL_ERR_INVALID;
	}

	if (year < 2000 || year > 2099)
		return LL_ERR_PARA;
	for (t = 2000; t < year; t++)
	{
		if (RTC_Is_Leap_Year(t))
			toSec += 31622400;
		else
			toSec += 31536000;
	}

	mon -= 1;
	for (t = 0; t < mon; t++)
	{
		toSec += table_mon[t] * 86400;
		if (RTC_Is_Leap_Year(year) && t == 1)
			toSec += 86400;
	}

	toSec += (day - 1) * 86400;
	toSec += hour * 3600;
	toSec += min * 60;
	toSec += sec;

	alarm_list[alarmN].sec = toSec;
	alarm_list[alarmN].CB = alarmCB;

	return LL_ERR_SUCCESS;
}

static uint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day)
{
	/* 蔡勒公式 */
	uint16_t temp;
	uint8_t yearH, yearL;

	yearH = year / 100;
	yearL = year % 100;
	if (yearH > 19)
		yearL += 100;
	temp = yearL + (yearL >> 2);
	temp = temp % 7;						   // 得到年份代码
	temp = temp + day + table_week[month - 1]; // 年份代码+日期代码+月份代码
	if ((yearL & 3) == 0 && month < 3)
		temp--; // 考虑闰年的情况

	return temp % 7;
}
static bool RTC_Is_Leap_Year(uint16_t year)
{
	return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

void *LLOS_malloc(size_t size)
{
	uint32_t *head; // 表头格式为0xcdxxxxac，xxxx代表已经被分配的内存长度
	uint32_t *nextHead;
	size_t i, s;

	if (size > ll_init_cfgs.poolSize || size < 1)
		return NULL;

	head = ll_init_cfgs.pPool;
	s = (size + 3) / 4;

	do
	{
		if ((head + s) > ll_init_cfgs.pPool + (ll_init_cfgs.poolSize >> 2)) // 内存不足，分配失败
		{
			return NULL;
			//			break;
		}
		if ((head[0] >> 24) == 0xcd && (head[0] & 0xff) == 0xac) // 内存已被使用
		{
			head += ((head[0] >> 8) & 0xffff) + 1;
		}
		else // 内存没有被使用
		{
			nextHead = head + 1;
			for (i = 0; i < s; i++)
			{
				// 寻找下一个表头
				if ((nextHead[0] >> 24) == 0xcd && (nextHead[0] & 0xff) == 0xac)
				{
					head = nextHead;
					break;
				}
				else
				{
					nextHead++;
				}
			}
			if (i >= s)
				break; // 找到足够大的未被分配的空间
		}
	} while (1);

	head[0] = (0xcd0000ac | (s << 8));

	return (void *)(head + 1);
}
void LLOS_free(void *p)
{
	uint32_t *head;
    uint16_t i, size;
    uint32_t *pool_start = ll_init_cfgs.pPool;
    uint32_t *pool_end = pool_start + (ll_init_cfgs.poolSize >> 2);

    if (p == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return;
	}

    head = (uint32_t *)p - 1;

    if ((uint32_t *)head < pool_start || (uint32_t *)head >= pool_end)
	{
        LL_LOG_E("%s ", "parameter Invalid!\r\n", __FUNCTION__);
        return;
    }

    if (((head[0] >> 24) == 0xcd) && ((head[0] & 0xff) == 0xac))
	{
        size = (head[0] >> 8) & 0xffff;

        if (head + size + 1 > pool_end)
		{
            LL_LOG_E("%s ", "parameter VERIFY!\r\n", __FUNCTION__);
            return;
        }

        for (i = 0; i <= size; i++)
            head[i] = 0;
    }
    // 魔数不匹配：可能是重复释放或非法指针，可选择忽略或报错
    else
	{
        LL_LOG_E("%s ", "parameter VERIFY!\r\n", __FUNCTION__);
    }
}
uint16_t LLOS_Pool_GetUsedSize(void)
{
	uint32_t *head; // 表头指针
	uint32_t *end;	// 内存池末尾

	head = ll_init_cfgs.pPool;								 // 内存池起始位置
	end = ll_init_cfgs.pPool + (ll_init_cfgs.poolSize >> 2); // 内存池末尾位置

	while (head < end)
	{
		if ((head[0] >> 24) == 0xcd && (head[0] & 0xff) == 0xac) // 表头标记为已分配内存
		{
			uint16_t blockSize = (head[0] >> 8) & 0xffff; // 获取已分配的内存块大小
			head += blockSize + 1;						  // 跳过当前已分配的内存块
		}
		else
		{
			break;
		}
	}

	return head - ll_init_cfgs.pPool;
}

void LLOS_ErrorHandler(uint8_t errCode)
{
	if (ll_init_CBs.errHandle == NULL)
	{
		LL_LOG_E("%s ", "CB NULL!\r\n", __FUNCTION__);
		return;
	}
	ll_init_CBs.errHandle(errCode);
}
void LLOS_System_Reset(void)
{
	if (ll_init_CBs.systemReset == NULL)
	{
		LL_LOG_E("%s ", "CB NULL!\r\n", __FUNCTION__);
		return;
	}
	ll_init_CBs.systemReset();
}

void LLOS_Register_LP(ll_LP_hook_t LP_CB)
{
	ll_LP_CB = LP_CB;
}

void LLOS_Critical(bool isEnter)
{
	if(isEnter)
	{
		if (ll_init_CBs.criticalEnter == NULL)
		{
			return;
		}
		ll_init_CBs.criticalEnter();
	}
	else
	{
		if (ll_init_CBs.criticalEXIT == NULL)
		{
			return;
		}
		ll_init_CBs.criticalEXIT();
	}
}
/* =====================================[设备驱动框架]====================================== */
ll_deviceId_t LLOS_Register_Device(struct ll_device_t *dev)
{
	if (deviceIndex >= ll_init_cfgs.deviceNum)
	{
		LL_LOG_E("%s ", "> deviceNum!\r\n", __FUNCTION__);
		return LL_ERR_INVALID;
	}
	if (dev == NULL || dev->name == NULL || dev->ops == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}

	device_list[deviceIndex].deviceId = deviceIndex;
	device_list[deviceIndex].name = dev->name;
	device_list[deviceIndex].ops = dev->ops;
	device_list[deviceIndex].priv = dev->priv;

	deviceIndex++;

	return deviceIndex - 1;
}

uint8_t LLOS_Device_GetNum(void)
{
	return deviceIndex;
}

struct ll_device_t *LLOS_Device_Find(const char *name)
{
	for (ll_deviceId_t i = 0; i < ll_init_cfgs.deviceNum; i++)
	{
		if (device_list[i].name != NULL && strcmp(name, device_list[i].name) == 0)
		{
			return &device_list[i];
		}
	}

	return NULL;
}

void LLOS_Device_EnumAll(void)
{
	for (ll_deviceId_t i = 0; i < ll_init_cfgs.deviceNum; i++)
	{
		if (device_list[i].name)
			LL_LOG_I("Device: %u -> %s\r\n", i, device_list[i].name);
	}
}

ll_err_t LLOS_Device_Init(struct ll_device_t *dev, void *arg)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->initCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->initCB(dev, arg);
}
ll_err_t LLOS_Device_DeInit(struct ll_device_t *dev, void *arg)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->deInitCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->deInitCB(dev, arg);
}
ll_err_t LLOS_Device_Open(struct ll_device_t *dev, uint32_t cmd)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->openCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	dev->isOpen = true;
	return dev->ops->openCB(dev, cmd);
}
ll_err_t LLOS_Device_Close(struct ll_device_t *dev)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->closeCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	dev->isOpen = false;
	return dev->ops->closeCB(dev);
}
ll_err_t LLOS_Device_Read(struct ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->readCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->readCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_Write(struct ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->writeCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->writeCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_WriteRead(struct ll_device_t *dev, uint32_t address, uint32_t offset,
							   const void *writeData, void *readData, uint32_t wlen, uint32_t rlen)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->write_readCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->write_readCB(dev, address, offset, writeData, readData, wlen, rlen);
}
uint32_t LLOS_Device_ReadPin(struct ll_device_t *dev, uint32_t pin)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->readPinCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return 0xFFFFFFFF;
	}
	return dev->ops->readPinCB(dev, pin);
}
ll_err_t LLOS_Device_WritePin(struct ll_device_t *dev, uint32_t pin, ll_bit_t newState)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->writePinCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->writePinCB(dev, pin, newState);
}
ll_err_t LLOS_Device_DMARead(struct ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->DMA_readCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->DMA_readCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_DMAWrite(struct ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->DMA_writeCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->DMA_writeCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_Ctrl(struct ll_device_t *dev, uint32_t cmd, void *arg)
{
	if (dev == NULL || dev->ops == NULL || dev->ops->ctrlCB == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	return dev->ops->ctrlCB(dev, cmd, arg);
}

/* =====================================[指令解析]====================================== */
#if LL_USE_CMD_SHELL
static uint16_t ll_cmd_bufSize;
static ll_cmd_t context;
static const char *pvid, *ppid, *pversion, *psn;

static const ll_unit_entry_t builtin_units[] =
{
	// 长度4
	{"mohm", 0.001, 4},
	{"kohm", 1000.0, 4},
	{"Mohm", 1000000.0, 4},

	// 长度3
	{"kHz", 1000.0, 3},
	{"MHz", 1000000.0, 3},
	{"ohm", 1.0, 3},

	// 长度2
	{"mA", 0.001, 2},
	{"mV", 0.001, 2},
	{"ms", 0.001, 2},
	{"mm", 0.001, 2},
	{"mF", 0.001, 2},
	{"mH", 0.001, 2},
	{"mW", 0.001, 2},
	{"kA", 1000.0, 2},
	{"kV", 1000.0, 2},
	{"km", 1000.0, 2},
	{"kW", 1000.0, 2},
	{"Hz", 1.0, 2},

	// 长度1
	{"A", 1.0, 1},
	{"V", 1.0, 1},
	{"s", 1.0, 1},
	{"m", 1.0, 1},
	{"F", 1.0, 1},
	{"H", 1.0, 1},
	{"W", 1.0, 1},
};
/* 尝试解析并转换单位，返回是否成功转换，并更新 endptr */
static bool convert_unit(double *value, char **endptr)
{
    char *p = *endptr;

    while (*p == ' ' || *p == ',') p++;

    int best_len = 0;
    double best_mult = 1.0;

    // 遍历内置单位表，寻找最长匹配
    for (size_t i = 0; i < sizeof(builtin_units) / sizeof(builtin_units[0]); i++)
	{
        const ll_unit_entry_t *unit = &builtin_units[i];

        if (strncmp(p, unit->suffix, unit->len) == 0)
		{
            if (unit->len > best_len)
			{
                best_len = unit->len;
                best_mult = unit->multiplier;
            }
        }
    }

    if (best_len > 0)
	{
        *value *= best_mult;
        *endptr = p + best_len;
        return true;
    }
    return false;
}

ll_err_t LLOS_Cmd_IDN(ll_cmd_t *context)
{
	ll_cmd_printf("%s,%s,%s,%s\r\n", pvid, ppid, pversion, psn);
	return LL_ERR_SUCCESS;
}
ll_err_t LLOS_Cmd_RST(ll_cmd_t *context)
{
	LLOS_System_Reset();
	return LL_ERR_SUCCESS;
}

extern struct cmdList_t cmdList[];

static int compare(const void *a, const void *b)
{
	const struct cmdList_t *sa = (const struct cmdList_t *)a;
	const struct cmdList_t *sb = (const struct cmdList_t *)b;
	if (strlen(sb->pattern) > strlen(sa->pattern)) return 1;
	else if (strlen(sb->pattern) < strlen(sa->pattern)) return -1;
	else return 0;
}
void LLOS_Cmd_Init(uint16_t bufSize, const char *vid, const char *pid, const char *version, const char *sn)
{
	uint32_t size;

	pvid = vid;
	ppid = pid;
	pversion = version;
	psn = sn;

	ll_cmd_bufSize = bufSize;
	if (ll_cmd_bufSize <= sizeof(char))
	{
		LL_LOG_E("%s ", "bufSize <= sizeof(char)!", __FUNCTION__);
		while (1);
	}

	size = sizeof(char) * bufSize;
	context.buffer = LLOS_malloc(size);
	if (context.buffer == NULL)
	{
		LL_LOG_E("%s ", "context.buffer malloc failed!\r\n", __FUNCTION__);
		while (1);
	}
	context.buffer[0] = '\0';

	int i;
	for (i = 0; cmdList[i].callback != NULL; i++);
	qsort(cmdList, i, sizeof(cmdList[0]), compare);
}

bool LLOS_Cmd_Input(const char *data, uint32_t len)
{
	for (int i = 0; cmdList[i].callback != NULL; i++)
	{
		uint32_t patternLen = strlen(cmdList[i].pattern);
		
		if (len >= patternLen && strncasecmp(cmdList[i].pattern, data, patternLen) == 0)
		{
			context.len = len - patternLen; // 获取除去指令后字符串数据长度
			if (context.len >= ll_cmd_bufSize) context.len = ll_cmd_bufSize - 1;  // 留出结尾
			strncpy(context.buffer, data + patternLen, context.len); // 获取除去指令后字符串数据
			context.buffer[context.len] = '\0';   // 强制加结尾
			if (cmdList[i].callback != NULL)
			{
				ll_err_t errCode = cmdList[i].callback(&context); // 执行回调
				if (errCode) LL_LOG_E("LLOS CMD ERR ", "0x%02X\r\n", errCode);
				return true;
			}
			break;
		}
	}

	LL_LOG_E("", "LLOS CMD not found!\r\n");
	return false;
}

// 跳过space和,
static void Trim(char *str)
{
	if (str == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return;
	}

	char *p = str;
	while (*p == ' ' || *p == ',') p++;
	if (*p == '\0')
	{
		*str = '\0';
		return;
	}

	memmove(str, p, strlen(p) + 1);
}

ll_err_t LLOS_Cmd_ParamBool(ll_cmd_t *context, bool *val)
{
	if (context == NULL || val == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}

	char *endptr;
	endptr = context->buffer;

	Trim(context->buffer);

	if (*context->buffer == '\0')
	{
		LL_LOG_E("%s ", "failed!\r\n", __FUNCTION__);
		return LL_ERR_FAILED;
	}

	if ((context->buffer[0] == '0' || context->buffer[0] == '1'))
	{
		*val = (context->buffer[0] == '1') ? true : false;
		endptr += 1;
		goto label;
	}
	else if (strncasecmp(context->buffer, "ON", 2) == 0)
	{
		*val = true;
		endptr += 2;
		goto label;
	}
	else if (strncasecmp(context->buffer, "OFF", 3) == 0)
	{
		*val = false;
		endptr += 3;
		goto label;
	}
	else if (strncasecmp(context->buffer, "TRUE", 4) == 0)
	{
		*val = true;
		endptr += 4;
		goto label;
	}
	else if (strncasecmp(context->buffer, "FALSE", 5) == 0)
	{
		*val = false;
		endptr += 5;
		goto label;
	}

	LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
	return LL_ERR_FAILED;

label:
	memmove(context->buffer, endptr, strlen(endptr) + 1);
	return LL_ERR_SUCCESS;
}
ll_err_t LLOS_Cmd_ParamFloat(ll_cmd_t *context, float *val)
{
	if (context == NULL || val == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}

	double temp;
	char *endptr;

	Trim(context->buffer);

	if (*context->buffer == '\0')
	{
		LL_LOG_E("%s ", "failed!\r\n", __FUNCTION__);
		return LL_ERR_FAILED;
	}

	temp = strtof(context->buffer, &endptr);

	if (endptr == context->buffer)
	{
		LL_LOG_E("%s ", "failed!\r\n", __FUNCTION__);
		return LL_ERR_FAILED;
	}

	convert_unit(&temp, &endptr);

	memmove(context->buffer, endptr, strlen(endptr) + 1);

	*val = temp;

	return LL_ERR_SUCCESS;
}
ll_err_t LLOS_Cmd_ParamInt32(ll_cmd_t *context, int32_t *val)
{
	if (context == NULL || val == NULL)
	{
		LL_LOG_E("%s ", "parameter NULL!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}

	int32_t temp;
	char *endptr;

	Trim(context->buffer);

	if (*context->buffer == '\0')
	{
		LL_LOG_E("%s ", "failed!\r\n", __FUNCTION__);
		return LL_ERR_FAILED;
	}

	temp = strtol(context->buffer, &endptr, 10);

	if (endptr == context->buffer)
	{
		LL_LOG_E("%s ", "failed!\r\n", __FUNCTION__);
		return LL_ERR_FAILED;
	}

	double ftemp = temp;
	convert_unit(&ftemp, &endptr);

	memmove(context->buffer, endptr, strlen(endptr) + 1);

	*val = ftemp;

	return LL_ERR_SUCCESS;
}
ll_err_t LLOS_Cmd_ParamCopyText(ll_cmd_t *context, char *text, uint32_t copyLen)
{
	if (context == NULL || text == NULL || copyLen == 0)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}

	Trim(context->buffer);

	char *start = strchr(context->buffer, '"');
	char *end;

	if (start == NULL)
	{
		text[0] = '\0';
		LL_LOG_E("%s ", "parameter format error!\r\n", __FUNCTION__);
		return LL_ERR_FAILED;
	}

	start++;
	end = strchr(start, '"');

	if (end == NULL)
	{
		text[0] = '\0';
		LL_LOG_E("%s ", "parameter format error!\r\n", __FUNCTION__);
		return LL_ERR_FAILED;
	}

	uint32_t textLen = (uint32_t)(end - start);

	LL_LIMIT_MAX(textLen, context->len);
	LL_LIMIT_MAX(textLen, copyLen);

	strncpy(text, start, textLen);
	text[textLen] = '\0';

	return LL_ERR_SUCCESS;
}

void LLOS_Cmd_ResultBool(bool val)
{
	if (val) ll_cmd_printf("true\r\n");
	else ll_cmd_printf("false\r\n");
}
void LLOS_Cmd_ResultFloat(float val)
{
	ll_cmd_printf("%f\r\n", val);
}
void LLOS_Cmd_ResultInt32(int32_t val)
{
	ll_cmd_printf("%d\r\n", val);
}
void LLOS_Cmd_ResultUInt32(uint32_t val)
{
	ll_cmd_printf("%u\r\n", val);
}
void LLOS_Cmd_ResultText(char *val)
{
	ll_cmd_printf("%s\r\n", val);
}
#endif
