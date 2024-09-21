 /* 
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.0
 * 修订日期: 2024 09 13
 * 修订日志:
 * N/A
 */
#include <llos.h>

struct ll_eventCB_list_t
{
	ll_tick_t startTick[16];
	bool oldActivation[16];
	bool newActivation[16];
	ll_taskEvent_t oldEvents;
	ll_taskEvent_t newEvents;
	ll_eventCB_t eventCB;
};
struct ll_timerCB_list_t
{
	ll_tick_t initTick;
	ll_tick_t tick;
	ll_newState_t newState;
	bool mode;
	ll_timerCB_t timerCB;
};

struct ll_rtc_t
{
	ll_tick_t initTick;
	volatile uint32_t sec;
};
struct ll_alarm_t
{
	uint32_t Sec;
	ll_alarmCB_t CB;
};

static volatile ll_tick_t sysTick;
static volatile uint8_t tickMultiple = 1;
static ll_taskId_t taskIndex;
static const uint8_t *msg;

static struct ll_eventCB_list_t eventCB_list[LL_TASK_NUM]; /* 0xFF保留不可用 */
static struct ll_timerCB_list_t timerCB_list[LL_TIMER_NUM];
static struct ll_rtc_t rtc;
static struct ll_alarm_t alarm[LL_ALARM_NUM];

struct ll_calendar_t ll_calendar;

static uint32_t heap[LL_HEAP_SIZE / 4 + 1] __LL_ALIGNED(4);

static ll_userDelay_hook_t ll_userDelayMs, ll_userDelayUs;
static ll_errHandler_hook_t errHandler_hook;
static ll_system_reset_hook_t system_reset_hook;

void LLOS_Init(ll_hwTimerInit_hook_t hwTimerInit, ll_userDelay_hook_t userDelayMs, ll_userDelay_hook_t userDelayUs)
{
	ll_userDelayMs = userDelayMs;
	ll_userDelayUs = userDelayUs;
	
	if(hwTimerInit != NULL)hwTimerInit();

	LOG_I("H  e  l  l  o   --     --  \r\n");
	LOG_I("|      |      |    | |     \r\n");
	LOG_I("|      |      |    |  --   \r\n");
	LOG_I("|      |      |    |     | \r\n");
	LOG_I(" ----   ----    --    --   \r\n");
	LOG_I("LLOS version: %s\r\n", LLOS_VERSION);
}

ll_taskId_t LLOS_Register_Events(ll_eventCB_t ll_eventCB)
{
	if(taskIndex >= LL_TASK_NUM)return LL_ERR_INVALID;

	eventCB_list[taskIndex].eventCB = ll_eventCB;
	taskIndex++;

	return taskIndex - 1;
}

void LLOS_Loop(void)
{
	int8_t i, j;
	for(i = 0; i < taskIndex; i++) /* 任务轮询 */
	{
		for(j = 15; j >= 0; j--) /* 事件轮询，从0x8000开始保证消息事件的优先级最高  */
		{
			/* 更新事件状态 */
			eventCB_list[i].oldEvents |= eventCB_list[i].newEvents;
			if(eventCB_list[i].newActivation[j])
			{
				eventCB_list[i].oldActivation[j] = eventCB_list[i].newActivation[j];
			}
			eventCB_list[i].newEvents = 0;
			eventCB_list[i].newActivation[j] = 0;

			/* 如果事件已激活&&系统节拍大于该事件的启动节拍则执行该事件 */
			if(
					eventCB_list[i].oldActivation[j] &&
					(sysTick >= eventCB_list[i].startTick[j]) &&
					eventCB_list[i].eventCB != NULL)
			{
				/* 启动对应的事件并且清除返回的事件 */
				eventCB_list[i].oldEvents ^= eventCB_list[i].eventCB(i, eventCB_list[i].oldEvents & (0x0001 << j));
				/* 如果事件标志已被清除则取消激活该事件 */
				if((eventCB_list[i].oldEvents & (0x0001 << j)) == 0x0000)
				{
					eventCB_list[i].oldActivation[j] = false;
				}
			}
		}
	}
}

void LLOS_Start_Event(ll_taskId_t taskId, ll_taskEvent_t events, ll_tick_t tick)
{
	uint8_t i;

	if(taskId >= LL_TASK_NUM)return;
	eventCB_list[taskId].newEvents |= events; /* 将要启动的事件添加到新事件列表 */
	for(i = 0; i < 16; i++)
	{
		if((events >> i) & 0x0001) /* 操作对应事件 */
		{
			eventCB_list[taskId].startTick[i] = sysTick + tick; /* 为新事件设定启动时间 */
			eventCB_list[taskId].newActivation[i] = true; /* 激活新事件 */
		}
	}
}
void LLOS_Stop_Event(ll_taskId_t taskId, ll_taskEvent_t events)
{
	uint8_t i;

	if(taskId >= LL_TASK_NUM)return;
	for(i = 0; i < 16; i++)
	{
		if((events >> i) & 0x0001) /* 操作对应事件 */
		{
			eventCB_list[taskId].oldActivation[i] = false; /* 取消激活待执行事件 */
			eventCB_list[taskId].newActivation[i] = false; /* 取消激活新事件 */
		}
	}
}

uint8_t LLOS_Get_TaskNum(void)
{
	return taskIndex;
}
uint64_t LLOS_Get_SysTick(void)
{
	return sysTick;
}
char *LLOS_Get_Version(void)
{
	return LLOS_VERSION;
}

void LLOS_Tick_Increase(uint8_t ms)
{
	uint8_t i;
	tickMultiple = ms;

	sysTick++;

	/* RTC */
	if((sysTick - rtc.initTick) % LLOS_Ms_To_Tick(1000) == 0)
	{
		rtc.sec++;

		for(i = 0; i < LL_ALARM_NUM; i++)
		{
			if(rtc.sec == alarm[i].Sec && alarm[i].CB != NULL)
			{
				alarm[i].CB(i);
				alarm[i].CB = NULL;
			}
		}
	}
	/* 软件定时器 */
	for(i = 0; i < LL_TIMER_NUM; i++)
	{
		if(
				timerCB_list[i].newState &&
				((sysTick - timerCB_list[i].initTick) % timerCB_list[i].tick) == 0 &&
				timerCB_list[i].timerCB != NULL)
		{
			timerCB_list[i].timerCB(i);
			if(timerCB_list[i].mode == false)
			{
				timerCB_list[i].newState = ll_disable;
			}
		}
	}
}

void LLOS_Msg_Send(ll_taskId_t taskId, const uint8_t *pMsg)
{
	msg = pMsg;
	LLOS_Start_Event(taskId, LL_EVENT_MSG, 0);
}
const uint8_t *LLOS_Msg_Receive(void)
{
	return msg;
}
void LLOS_Msg_Clear(void)
{
	msg = NULL;
}

ll_tick_t LLOS_Ms_To_Tick(uint32_t ms)
{
	return (ms / tickMultiple);
}

uint32_t LLOS_Random(void)
{
	/* 线性同余算法 */
	uint32_t state = sysTick * 7; /* 状态 */
	uint32_t a = 1664525;     /* 乘数 */
	uint32_t c = 1013904223;  /* 增量 */
	uint32_t m = 0xFFFFFFFF;  /* 模数 */

	state = (a * state + c) % m;

	return state;
}

void LLOS_DelayMs(uint32_t time)
{
	if(ll_userDelayMs != NULL)ll_userDelayMs(time);
}
void LLOS_DelayUs(uint32_t time)
{
	if(ll_userDelayUs != NULL)ll_userDelayUs(time);
}

ll_err_t LLOS_Timer_Set(uint8_t timerN, ll_newState_t newState, bool mode, ll_tick_t tick, ll_timerCB_t timerCB)
{
	if(timerN >= LL_TIMER_NUM)
	{
		LOG_E("> LL_TIMER_NUM!\r\n");
		return LL_ERR_INVALID;
	}

	timerCB_list[timerN].initTick = sysTick;
	timerCB_list[timerN].newState = newState;
	timerCB_list[timerN].mode = mode;
	timerCB_list[timerN].tick = tick;
	timerCB_list[timerN].timerCB = timerCB;
	
	return LL_ERR_SUCCESS;
}

static const uint8_t table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
static const uint8_t table_mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static bool RTC_Is_Leap_Year(uint16_t year);
static uint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day);

void LLOS_RTC_SetDate(uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
	uint16_t t;
	uint32_t toSec = 0;

    if(year < 2000 || year > 2099)return;
    for(t = 2000; t < year; t++)
    {
        if(RTC_Is_Leap_Year(t))toSec += 31622400; /* 366*24*60*60 */
        else toSec += 31536000; /* 365*24*60*60 */
    }

    mon -= 1;
    for(t = 0; t < mon; t++)
    {
    	toSec += table_mon[t] * 86400; /* 60*60*24 */
        if(RTC_Is_Leap_Year(year) && t == 1)toSec += 86400; /* 闰年二月多一天 */
    }

	toSec += (day - 1) * 86400;
	toSec += hour * 3600;
	toSec += min * 60;
	toSec += sec;
	rtc.sec = toSec;
	rtc.initTick = sysTick;
}
void LLOS_RTC_GetDate(void)
{
    static uint16_t daycnt = 0;
    uint32_t temp = 0;
    uint16_t temp1 = 0;
	uint32_t toTime = rtc.sec;

	temp = toTime / 86400; /* 计算出天数 */

	if(daycnt != temp) /* 如果天数已更新 */
	{
		daycnt = temp;
		temp1 = 2000;
		while(temp >= 365)
		{
			if(RTC_Is_Leap_Year(temp1))
			{
				if(temp >= 366)temp -= 366;
				else break;
			}
			else
			{
				temp -= 365;
			}
			temp1++;
		}
		ll_calendar.year = temp1;

		temp1 = 0;
		while(temp >= 28) /* 超过一个月，此时temp为一年中的第几天 */
		{
			if(RTC_Is_Leap_Year(ll_calendar.year) && temp1 == 1) /* 闰年的2月份 */
			{
				if(temp >= 29)temp -= 29;
				else break;
			}
			else
			{
				if(temp >= table_mon[temp1])temp -= table_mon[temp1];
				else break;
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
}
void LLOS_RTC_SetAlarm(uint16_t year, uint8_t mon, uint8_t day,
		uint8_t hour, uint8_t min, uint8_t sec, ll_alarmCB_t alarmCB,
		uint8_t alarmN)
{
	uint16_t t;
	uint32_t toSec = 0;

	if(alarmN >= LL_ALARM_NUM)
	{
		LOG_E("> LL_ALARM_NUM!\r\n");
		return;
	}

    if(year < 2000 || year > 2099)return;
    for(t = 2000; t < year; t++)
    {
        if(RTC_Is_Leap_Year(t))toSec += 31622400; /* 366*24*60*60 */
        else toSec += 31536000; /* 365*24*60*60 */
    }

    mon -= 1;
    for(t = 0; t < mon; t++)
    {
    	toSec += table_mon[t] * 86400; /* 60*60*24 */
        if(RTC_Is_Leap_Year(year) && t == 1)toSec += 86400; /* 闰年二月多一天 */
    }

	toSec += (day - 1) * 86400;
	toSec += hour * 3600;
	toSec += min * 60;
	toSec += sec;
	alarm[alarmN].Sec = toSec;
	alarm[alarmN].CB = alarmCB;
}

static uint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day)
{
    /* 蔡勒公式 */
	uint16_t temp;
	uint8_t  yearH, yearL;

    yearH = year / 100;
    yearL = year % 100;
    if(yearH > 19)yearL += 100;
    temp = yearL + (yearL >> 2);
    temp = temp % 7; /* 得到年份代码 */
    temp = temp + day + table_week[month - 1]; /* 年份代码+日期代码+月份代码 */
    if((yearL & 3) == 0 && month < 3)temp--; /* 考虑闰年的情况 */

    return temp % 7;
}
static bool RTC_Is_Leap_Year(uint16_t year)
{
    if((year & 3) == 0)
    {
        if(year % 100 == 0)
        {
            if(year % 400 == 0)return true; /* 能被400整除为闰年 */
            else return false;
        }
        else
            return true; /* 能被4整除但是不能被100整除为闰年 */
    }
    else
    {
        return false;
    }
}

void *LLOS_malloc(uint16_t size_t)
{
	uint32_t *head; /* 表头格式为0xcdxxxxac，xxxx代表已经被分配的内存长度 */
	uint32_t *nextHead;
	uint16_t i, size;

	if(size_t > sizeof(heap) || size_t < 1)return NULL;

	head = heap;
	size =  size_t >> 2;
	if((size_t & 3) != 0)size += 1;

	do
	{
		if((head + size) >= heap + (sizeof(heap) >> 2)) /* 内存不足，分配失败 */
		{
			return NULL;
//			break;
		}
		if((head[0] >> 24) == 0xcd && (head[0] & 0xff) == 0xac) /* 内存已被使用 */
		{
			head += ((head[0] >> 8) & 0xffff) + 1;
		}
		else /* 内存没有被使用 */
		{
			nextHead = head + 1;
			for(i = 0; i < size; i++)
			{
				/* 寻找下一个表头 */
				if((nextHead[0] >> 24) == 0xcd && (nextHead[0] & 0xff) == 0xac)
				{
					head = nextHead;
					break;
				}
				else
				{
					nextHead++;
				}
			}
			if(i >= size)break; /* 找到足够大的未被分配的空间 */
		}
	}while(1);

	head[0] =(0xcd0000ac | (size << 8));

	return (void *)(head + 1);
}
void LLOS_free(void *p)
{
	uint32_t *head;
	uint16_t i, size;

	head = (uint32_t *)p;
	head -= 1;

	if((head[0] >> 24) == 0xcd && (head[0] & 0xff) == 0xac)
	{
		size = (head[0] >> 8) & 0xffff;
		for(i = 0; i <= size; i++)head[i] = 0;
	}
}

void LLOS_Register_ErrorHandler(ll_errHandler_hook_t ll_errHandler_hook)
{
	errHandler_hook = ll_errHandler_hook;
}
void LLOS_ErrorHandler(uint8_t errCode)
{
	if(errHandler_hook != NULL)errHandler_hook(errCode);
}

void LLOS_Register_System_Reset(ll_system_reset_hook_t ll_system_reset_hook)
{
	system_reset_hook = ll_system_reset_hook;
}
void LLOS_System_Reset(void)
{
	if(system_reset_hook != NULL)system_reset_hook();
}

/* =====================================[设备驱动框架]====================================== */
static ll_deviceId_t deviceIndex;
static ll_device_t ll_deviceList[LL_DEV_MAX_NUM];

ll_deviceId_t LLOS_Register_Device(ll_device_t *dev)
{
	if(deviceIndex >= LL_DEV_MAX_NUM)return LL_ERR_INVALID;

	ll_deviceList[deviceIndex].deviceId = deviceIndex;
	ll_deviceList[deviceIndex].name = dev->name;

	ll_deviceList[deviceIndex].initCB = dev->initCB;
	ll_deviceList[deviceIndex].openCB = dev->openCB;
	ll_deviceList[deviceIndex].closeCB = dev->closeCB;
	ll_deviceList[deviceIndex].readCB = dev->readCB;
	ll_deviceList[deviceIndex].writeCB = dev->writeCB;
	ll_deviceList[deviceIndex].write_readCB = dev->write_readCB;
	ll_deviceList[deviceIndex].readPinCB = dev->readPinCB;
	ll_deviceList[deviceIndex].writePinCB = dev->writePinCB;
	ll_deviceList[deviceIndex].DMA_readCB = dev->DMA_readCB;
	ll_deviceList[deviceIndex].DMA_writeCB = dev->DMA_writeCB;
	ll_deviceList[deviceIndex].ctrlCB = dev->ctrlCB;

	deviceIndex++;

	return deviceIndex - 1;
}

uint8_t LLOS_Device_GetNum(void)
{
	return deviceIndex;
}

ll_device_t *LLOS_Device_Find(const char *name)
{
	for(ll_deviceId_t i = 0; i < LL_DEV_MAX_NUM; i++)
	{
		if(strcmp(name, ll_deviceList[i].name) == 0)
		{
			return &ll_deviceList[i];
		}
	}
	return NULL;
}

ll_err_t LLOS_Device_Init(ll_device_t *dev, void *arg)
{
	if(dev == NULL || dev->initCB == NULL)return LL_ERR_NULL;
	return dev->initCB(dev, arg);
}
ll_err_t LLOS_Device_Open(ll_device_t *dev, uint32_t cmd)
{
	if(dev == NULL || dev->openCB == NULL)return LL_ERR_NULL;
	dev->isOpen  = true;
	return dev->openCB(dev, cmd);
}
ll_err_t LLOS_Device_Close(ll_device_t *dev)
{
	if(dev == NULL || dev->closeCB == NULL)return LL_ERR_NULL;
	dev->isOpen  = false;
	return dev->closeCB(dev);
}
ll_err_t LLOS_Device_Read(ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len)
{
	if(dev == NULL || dev->readCB == NULL)return LL_ERR_NULL;
	return dev->readCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_Write(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len)
{
	if(dev == NULL || dev->writeCB == NULL)return LL_ERR_NULL;
	return dev->writeCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_WriteRead(ll_device_t *dev, uint32_t address, uint32_t offset, const void *writeData, void *readData, uint32_t len)
{
	if(dev == NULL || dev->write_readCB == NULL)return LL_ERR_NULL;
	return dev->write_readCB(dev, address, offset, writeData, readData, len);
}
uint32_t LLOS_Device_ReadPin(struct ll_device *dev, uint32_t pin)
{
	if(dev == NULL || dev->readPinCB == NULL)return 0xFFFFFFFF;
	return dev->readPinCB(dev, pin);
}
ll_err_t LLOS_Device_WritePin(struct ll_device *dev, uint32_t pin, ll_bit_t newState)
{
	if(dev == NULL || dev->writePinCB == NULL)return LL_ERR_NULL;
	return dev->writePinCB(dev, pin, newState);
}
ll_err_t LLOS_Device_DMARead(ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len)
{
	if(dev == NULL || dev->DMA_readCB == NULL)return LL_ERR_NULL;
	return dev->DMA_readCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_DMAWrite(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len)
{
	if(dev == NULL || dev->DMA_writeCB == NULL)return LL_ERR_NULL;
	return dev->DMA_writeCB(dev, address, offset, buffer, len);
}
ll_err_t LLOS_Device_Ctrl(ll_device_t *dev, uint32_t cmd, void *arg)
{
	if(dev == NULL || dev->ctrlCB == NULL)return LL_ERR_NULL;
	return dev->ctrlCB(dev, cmd, arg);
}
