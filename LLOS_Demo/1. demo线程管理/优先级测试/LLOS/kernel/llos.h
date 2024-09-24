/*
 * LLOS是一款轻量级的可以在支持64bit变量的MCU上运行的非实时资源管理系统，以
 * 事件驱动的方式实现线程管理，目的是取代裸机编程。系统内核提供简单的线程管理、任务间
 * 通信、软定时器、RTC、动态内存管理和设备驱动框架。要注意的是，阻塞会影响调度精度，
 * 所以建议尽可能地使用状态机对阻塞任务进行拆分。RTC闹钟和软件定时器的回调函数里不能
 * 有阻塞。
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.1
 * 修订日期: 2024 09 23
 * 修订日志:
 * V1.0.1 事件的值越小优先级越高表述有误，应为事件的值越大优先级越高
 * N/A
 * 移植步骤:
 * 1) 初始化调用LLOS_Init;
 * 2) while(1)调用LLOS_Loop;
 * 3) 定时器中断函数调用LLOS_Tick_Increase;
 * 基础使用步骤:
 * 1) 使用LLOS_Register_Events创建任务并为其注册事件回调函数;
 * 2) 如果任务ID有效(不是LL_ERR_INVALID)使用LLOS_Start_Event启动事件;
 * 3) ...
 */
#ifndef __LLOS_H
#define __LLOS_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <llos_conf.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define LLOS_VERSION		"V1.0.0"

#define LL_EVENT_MSG		(0x8000)

#define LL_ERR_SUCCESS		(0x00)
#define LL_ERR_OVERFLOW		(0x01)
#define LL_ERR_NULL			(0x02)
#define LL_ERR_VERIFY		(0x03)
#define LL_ERR_FAIL			(0x04)
#define LL_ERR_INVALID		(0xFF)

#define LL_BV(n)      		(1 << (n))
#define LL_BF(x, b, s)  	(((x) & (b)) >> (s))
#define LL_ABS(n)			(((n) < 0) ? -(n) : (n))
#define LL_MIN(a, b)		(((a) < (b)) ? (a) : (b))
#define LL_MAX(a, b)		(((a) > (b)) ? (a) : (b))
#define LL_BUILD_U16(l, h) ((uint16_t)(((l) & 0x00FF) | (((h) & 0x00FF) << 8)))
#define LL_BUILD_U32(b0, b1, b2, b3) \
          ((uint32_t)(((uint32_t)(b0) & 0x00FF) \
          | (((uint32_t)(b1) & 0x00FF) << 8) \
          | (((uint32_t)(b2) & 0x00FF) << 16) \
          | (((uint32_t)(b3) & 0x00FF) << 24)))

#ifndef NULL
#define NULL				((void *)0)
#endif

#ifndef UNUSED_VARIABLE
#define UNUSED_VARIABLE(X)  ((void)(X))
#endif

#define __LL_ALIGNED(n)		__attribute__((aligned(n)))
#define __LL_WEAK			__attribute__((weak))
#define __LL_PACKED			__attribute__((packed))
#define __LL_NOINLINE		__attribute__((noinline))
#define __LL_INLINE			__attribute__((always_inline))

typedef volatile uint32_t 	ll_IO_t;

typedef uint8_t				ll_err_t;
typedef uint8_t 			ll_taskId_t;
typedef uint16_t 			ll_taskEvent_t;
typedef uint64_t			ll_tick_t;

typedef enum
{
	ll_reset,
	ll_set = !ll_reset,
}ll_bit_t;
typedef enum
{
	ll_disable,
	ll_enable = !ll_disable,
}ll_newState_t;
typedef enum
{
	ll_success,
	ll_fail = !ll_success,
}ll_result_t;

struct ll_calendar_t
{
	uint16_t year;
	uint8_t mon;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t week;
};

/*====================================================================================
 * 函数指针类型: ll_eventCB_t
 * 描述: 事件回调函数
 * 参数:
 * 		taskId: 哪个任务调用该回调函数
 * 		events: 哪个事件要启动，Bitmap，支持15个自定义事件，0x8000保留为任务间通信用
 * 	返回值: 返回哪个事件则清除哪个事件
 ====================================================================================*/
typedef ll_taskEvent_t 		(*ll_eventCB_t)(ll_taskId_t taskId, ll_taskEvent_t events);

/*====================================================================================
 * 函数名: LLOS_Init
 * 描述: 初始化
 * 参数:
 * 		hwTimerInit: OS所使用的硬件定时器的初始化回调函数
 *		userDelayMs: 延时ms函数地址
 *		userDelayUs: 延时us函数地址
 ====================================================================================*/
typedef void 				(*ll_hwTimerInit_hook_t)(void);
typedef void 				(*ll_userDelay_hook_t)(uint32_t time);
void LLOS_Init(ll_hwTimerInit_hook_t hwTimerInit, ll_userDelay_hook_t userDelayMs, ll_userDelay_hook_t userDelayUs);

/*====================================================================================
 * 函数名: LLOS_Register_Events
 * 描述: 创建任务并为该任务注册事件回调函数，最大LL_TASK_NUM个任务，对应taskID为0x00-0xFE，0xFF保留
 * 参数:
 * 		ll_eventCB: 事件回调函数
 * 返回值: 创建的任务ID，任务ID值越小优先级越高，创建失败返回LL_ERR_INVALID
 ====================================================================================*/
ll_taskId_t LLOS_Register_Events(ll_eventCB_t ll_eventCB);

/*====================================================================================
 * 函数名: LLOS_Loop
 * 描述: OS处理函数，在死循环执行
 ====================================================================================*/
void LLOS_Loop(void);

/*====================================================================================
 * 函数名: LLOS_Start_Event
 * 描述: 启动一个事件，重复调用可以刷新事件启动时间(tick)
 * 参数:
 * 		taskId: 启动哪一个任务
 * 		events: 启动该任务的哪一个事件，Bitmap，事件的值越大优先级越高
 * 		tick: 多少个tick后启动
 ====================================================================================*/
void LLOS_Start_Event(ll_taskId_t taskId, ll_taskEvent_t events, ll_tick_t tick);

/*====================================================================================
 * 函数名: LLOS_Stop_Event
 * 描述: 停止一个事件
 * 参数:
 * 		taskId: 停止哪一个任务
 * 		events: 停止该任务的哪一个事件，Bitmap，事件的值越大优先级越高
 ====================================================================================*/
void LLOS_Stop_Event(ll_taskId_t taskId, ll_taskEvent_t events);

/*====================================================================================
 * 函数名: LLOS_Get_TaskNum
 * 描述: 获取OS任务数量
 * 返回值: 任务数量
 ====================================================================================*/
uint8_t LLOS_Get_TaskNum(void);

/*====================================================================================
 * 函数名: LLOS_Get_SysTick
 * 描述: 获取OS节拍
 * 返回值: OS节拍
 ====================================================================================*/
uint64_t LLOS_Get_SysTick(void);

/*====================================================================================
 * 函数名: LLOS_Get_Version
 * 描述: 获取OS版本
 * 返回值: OS版本
 ====================================================================================*/
char *LLOS_Get_Version(void);

/*====================================================================================
 * 函数名: LLOS_Tick_Increase
 * 描述: OS节拍计数，在定时器中断中调用，推荐1ms调用一次
 * 参数:
 * 		ms: 以ms为单位的定时器中断时间
 ====================================================================================*/
void LLOS_Tick_Increase(uint8_t ms);

/*====================================================================================
 * 函数名: LLOS_Msg_Send
 * 描述: 向某个任务发送消息，实际上就是传递指针后立即启动对应任务的LL_EVENT_MSG事件
 * 参数:
 * 		taskId: 向哪一个任务发送消息
 * 		pMsg: 消息数据首地址
 ====================================================================================*/
 void LLOS_Msg_Send(ll_taskId_t taskId, const void *pMsg);

/*====================================================================================
 * 函数名: LLOS_Msg_Receive
 * 描述: 某个任务收到消息后，使用该函数接收消息
 * 返回值: 消息数据首地址
 ====================================================================================*/
const void *LLOS_Msg_Receive(void);

/*====================================================================================
 * 函数名: LLOS_Msg_Clear
 * 描述: 某个任务收到消息后，使用该函数清除消息
 ====================================================================================*/
void LLOS_Msg_Clear(void);

/*====================================================================================
 * 函数名: LLOS_Ms_To_Tick
 * 描述: 将ms转换成tick
 * 参数:
 * 		ms: 毫秒
 * 返回值: tick
 ====================================================================================*/
ll_tick_t LLOS_Ms_To_Tick(uint32_t ms);

/*====================================================================================
 * 函数名: LLOS_Random
 * 描述: 利用系统节拍为种子产生伪随机数
 * 返回值: 伪随机数
 ====================================================================================*/
uint32_t LLOS_Random(void);

/*====================================================================================
 * 函数名: LLOS_DelayMs
 * 描述: 需要在LLOS_Init时注册才可使用
 * 参数:
 * 		time: 延时多少Ms
 ====================================================================================*/
void LLOS_DelayMs(uint32_t time);

/*====================================================================================
 * 函数名: LLOS_DelayUs
 * 描述: 需要在LLOS_Init时注册才可使用
 * 参数:
 * 		time: 延时多少Us
 ====================================================================================*/
void LLOS_DelayUs(uint32_t time);

/*====================================================================================
 * 函数名: LLOS_Timer_Set
 * 描述: OS提供的软件定时器，在LLOS_Tick_Increase()中执行，实时性相对较高
 * 参数:
 * 		timerN: 定时器ID，范围 0 - (LL_TIMER_NUM - 1)
 * 		newState: 是否启用该定时器
 * 		mode: 是否循环执行，ll_false为只执行一次，ll_true为重复执行
 * 		tick: 多少个tick执行一次
 * 		timerCB: 被执行的回调函数，该函数不要有阻塞
 *		timerN: OS传入哪个定时器调用了该回调函数(回调函数)
 ====================================================================================*/
typedef void 				(*ll_timerCB_t)(uint8_t timerN);
ll_err_t LLOS_Timer_Set(uint8_t timerN, ll_newState_t newState, bool mode, ll_tick_t tick, ll_timerCB_t timerCB);

/*====================================================================================
 * 函数名: LLOS_RTC_SetDate
 * 描述: 设置RTC时间
 * 参数:
 * 		year: 范围2000-2099
 ====================================================================================*/
void LLOS_RTC_SetDate(uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);

/*====================================================================================
 * 函数名: LLOS_RTC_GetDate
 * 描述: 计算并返回RTC时间，时间被保存在ll_calendar结构体
 ====================================================================================*/
extern struct ll_calendar_t ll_calendar;
void LLOS_RTC_GetDate(void);

/*====================================================================================
 * 函数名: LLOS_RTC_SetAlarm
 * 描述: 设置RTC闹钟
 * 参数:
 * 		year: 范围2000-2099
 * 		rtcCB: 被执行的回调函数，传入NULL可以关闭闹钟，在LLOS_Tick_Increase()中执
 * 		行，实时性相对较高，该函数不要有阻塞
 * 		alarmN: 闹钟ID，范围 0 - (LL_ALARM_NUM - 1)
 *		alarmN: OS传入哪个闹钟调用了该回调函数(回调函数)
 ====================================================================================*/
typedef void 				(*ll_alarmCB_t)(uint8_t alarmN);
void LLOS_RTC_SetAlarm(uint16_t year, uint8_t mon, uint8_t day,
		uint8_t hour, uint8_t min, uint8_t sec, ll_alarmCB_t alarmCB,
		uint8_t alarmN);

/*====================================================================================
 * 函数名: ll_malloc
 * 描述: 申请动态内存，最大值为LL_HEAP_SIZE
 * 参数:
 * 		size: 要申请的动态内存大小
 * 返回值: 申请到的动态内存首地址，失败返回NULL
 ====================================================================================*/
void *LLOS_malloc(uint16_t size_t);

/*====================================================================================
 * 函数名: ll_free
 * 描述: 释放动态内存
 * 参数:
 * 		p: 要释放的内存首地址
 * 返回值: 是否成功释放
 ====================================================================================*/
void LLOS_free(void *p);

/*====================================================================================
 * 函数名: LLOS_Register_ErrorHandler
 * 描述: 注册错误处理回调函数
 * 参数:
 * 		ll_errHandler_hook: 错误处理回调函数
 ====================================================================================*/
typedef void 				(*ll_errHandler_hook_t)(uint8_t errCode);
void LLOS_Register_ErrorHandler(ll_errHandler_hook_t ll_errHandler_hook);

/*====================================================================================
 * 函数名: LLOS_ErrorHandler
 * 描述: 错误处理
 * 参数:
 * 		errCode: 错误处理回调函数
 ====================================================================================*/
void LLOS_ErrorHandler(uint8_t errCode);

/*====================================================================================
 * 函数名: LLOS_System_Reset
 * 描述: 注册系统复位回调函数
 * 参数:
 * 		ll_system_reset_hook: 系统复位回调函数
 ====================================================================================*/
typedef void 				(*ll_system_reset_hook_t)(void);
void LLOS_Register_System_Reset(ll_system_reset_hook_t ll_system_reset_hook);

/*====================================================================================
 * 函数名: LLOS_System_Reset
 * 描述: 系统复位
 ====================================================================================*/
void LLOS_System_Reset(void);

/* =====================================[设备驱动框架]====================================== */
typedef uint8_t 					ll_deviceId_t;

typedef struct ll_device
{
	const char *name;							/* 设备名称 */
    bool isOpen;								/* 设备打开标志 */
    ll_deviceId_t deviceId;						/* 设备ID, 0 - 254 */

	ll_err_t (*initCB)   	(struct ll_device *dev, void *arg);
	ll_err_t (*openCB)   	(struct ll_device *dev, uint32_t cmd);
	ll_err_t (*closeCB)  	(struct ll_device *dev);
	ll_err_t (*readCB)   	(struct ll_device *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len);
	ll_err_t (*writeCB)  	(struct ll_device *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len);
	ll_err_t (*write_readCB)(struct ll_device *dev, uint32_t address, uint32_t offset, const void *writeData, void *readData, uint32_t len);
	uint32_t (*readPinCB)   (struct ll_device *dev, uint32_t pin);
	ll_err_t (*writePinCB)  (struct ll_device *dev, uint32_t pin, ll_bit_t newState);
	ll_err_t (*DMA_readCB)	(struct ll_device *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len);
	ll_err_t (*DMA_writeCB)	(struct ll_device *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len);
	ll_err_t (*ctrlCB)		(struct ll_device *dev, uint32_t cmd, void *args);
}ll_device_t;

/*====================================================================================
 * 函数名: LLOS_Register_Device
 * 描述: 注册设备
 * 参数:
 * 		dev: 设备句柄
 * 	返回值: 设备ID，注册失败返回LL_ERR_INVALID
 ====================================================================================*/
ll_deviceId_t LLOS_Register_Device(ll_device_t *dev);

/*====================================================================================
 * 函数名: LLOS_Device_GetNum
 * 描述: 获取设备数量
 * 返回值: 设备数量
 ====================================================================================*/
uint8_t LLOS_Device_GetNum(void);

/*====================================================================================
 * 函数名: LLOS_Device_Find
 * 描述: 按照设备名称查找设备
 * 参数:
 * 		name: 设备名称
 * 返回值: 设备句柄
 ====================================================================================*/
ll_device_t *LLOS_Device_Find(const char *name);

/*====================================================================================
 * 函数名: LLOS_Device_Init
 * 描述: 初始化设备
 * 参数:
 * 		dev: 设备句柄
 * 		arg: 参数
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_Init(ll_device_t *dev, void *arg);

/*====================================================================================
 * 函数名: LLOS_Device_Open
 * 描述: 打开设备
 * 参数:
 * 		dev: 设备句柄
 * 		cmd: 命令
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_Open(ll_device_t *dev, uint32_t cmd);

/*====================================================================================
 * 函数名: LLOS_Device_Close
 * 描述: 关闭设备
 * 参数:
 * 		dev: 设备句柄
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_Close(ll_device_t *dev);

/*====================================================================================
 * 函数名: LLOS_Device_Read
 * 描述: 从设备读数据
 * 参数:
 * 		dev: 设备句柄
 *		address：地址
 * 		offset: 偏移量
 * 		buffer: 缓冲区首地址
 * 		len: 读取的长度
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_Read(ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len);

/*====================================================================================
 * 函数名: LLOS_Device_Write
 * 描述: 写数据到设备
 * 参数:
 * 		dev: 设备句柄
 *		address：地址
 * 		offset: 偏移量
 * 		buffer: 缓冲区首地址
 * 		len: 写入的长度
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_Write(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len);

/*====================================================================================
 * 函数名: LLOS_Device_WriteRead
 * 描述: 读写数据到设备
 * 参数:
 * 		dev: 设备句柄
 *		address：地址
 * 		offset: 偏移量
 * 		writeData: 写缓冲区首地址
 * 		readData: 读缓冲区首地址
 * 		len: 读写的长度
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_WriteRead(ll_device_t *dev, uint32_t address, uint32_t offset,
	const void *writeData, void *readData, uint32_t len);

/*====================================================================================
 * 函数名: LLOS_Device_ReadPin
 * 描述: 读引脚数据
 * 参数:
 * 		dev: 设备句柄
 * 		pin: 引脚
 * 返回值: 读取结果
 ====================================================================================*/
uint32_t LLOS_Device_ReadPin(struct ll_device *dev, uint32_t pin);

/*====================================================================================
 * 函数名: LLOS_Device_WritePin
 * 描述: 写数据到引脚
 * 参数:
 * 		dev: 设备句柄
 * 		pin: 引脚
 * 		newState: 引脚状态
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_WritePin(struct ll_device *dev, uint32_t pin, ll_bit_t newState);

/*====================================================================================
 * 函数名: LLOS_Device_DMARead
 * 描述: 通过DMA方式从设备读数据
 * 参数:
 * 		dev: 设备句柄
 *		address：地址
 * 		offset: 偏移量
 * 		buffer: 缓冲区首地址
 * 		len: 读取的长度
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_DMARead(ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len);

/*====================================================================================
 * 函数名: LLOS_Device_DMAWrite
 * 描述: 通过DMA方式写数据到设备
 * 参数:
 * 		dev: 设备句柄
 *		address：地址
 * 		offset: 偏移量
 * 		buffer: 缓冲区首地址
 * 		len: 写入的长度
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_DMAWrite(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len);

/*====================================================================================
 * 函数名: LLOS_Device_Ctrl
 * 描述: 控制设备
 * 参数:
 * 		dev: 设备句柄
 * 		cmd: 命令
 * 		arg: 参数
 * 返回值: 错误码
 ====================================================================================*/
ll_err_t LLOS_Device_Ctrl(ll_device_t *dev, uint32_t cmd, void *arg);
	
#ifdef __cplusplus
 }
#endif

#endif /* __LLOS_H */
