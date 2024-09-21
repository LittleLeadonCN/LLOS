/*
 * 独立按键驱动，支持无阻塞的任意按键点击次数、长按及长按时间检测， 支持按下/弹起双状态和多按键
 * 同时检测，默认低电平有效。
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.1T
 * 修订日期: 2024 09 05
 * 修订日志:
 * N/A
 * 基础使用步骤:
 * 1) 初始化调用LLOS_Key_Init注册Key的GPIO初始化回调函数、配置IO输入状态寄存器地
 * 址和设定Key的轮询周期，随后系统会注册并启动相关任务
 * 2) 使用LLOS_Key_Register注册当按键状态变化时要调用的回调函数
 * 3) 在回调函数里读取回调函数传入的参数和ll_keyWhich结构体内的按键信息
 * 4) ...
 */
#ifndef __LLOS_KEY_H
#define __LLOS_KEY_H

#include <llos.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define LL_KEYn(n)				LL_BV(n)

typedef enum
{
	ll_key_event_NULL,
	ll_key_event_Click,
	ll_key_event_DoubleClick,
	ll_key_event_TripleClick,
	ll_key_event_LongPress,
}ll_keyEvent_t;

typedef struct
{
	ll_IO_t port;		/* GPIO存储引脚输入状态的寄存器地址 */
	uint32_t pinMask;	/* 使用到的Pin掩码 */
}ll_keyConfig_t;

struct ll_keyWhich_t
{
	uint32_t pin;		/* 哪个按键 */
	ll_keyEvent_t event;/* 按键事件(N击) */
    uint16_t pressTime;	/* 长按事件的时间(ms) */
};

/*====================================================================================
 * 函数指针类型: ll_keyCB_t
 * 描述: 按键状态发生变化时被执行的回调函数
 * 参数:
 * 		portN: 返回哪组按键发生变化
 * 		isUp: 当前按键是否已经弹起
 ====================================================================================*/
typedef void (*ll_keyCB_t)(uint8_t portN, bool isUp);

/*====================================================================================
 * 函数名: LLOS_Key_Init
 * 描述: 初始化
 * 参数:
 * 		keyInit: 按键的GPIO初始化回调函数
 * 		keyConfig: 配置使用到的GPIO组地址和Pin掩码
 * 		keyCB: 当有按键按下时执行的回调函数
 * 		ms: 毫秒，按键的轮询周期，一般为20
 *		timerN: 使用的OS定时器ID
 ====================================================================================*/
extern ll_keyConfig_t ll_keyConfig[LL_KEY_PORT_NUM];
typedef void (*ll_keyInit_hook_t)(void);
void LLOS_Key_Init(ll_keyInit_hook_t keyInit, ll_keyConfig_t *keyConfig, uint16_t ms, uint8_t timerN);

/*====================================================================================
 * 函数名: LLOS_Key_Register
 * 描述: 注册按键发生变化时要执行的回调函数，按键信息被保存在ll_keyWhich[LL_KEY_PORT_NUM]结构体数组
 * 参数:
 * 		keyCB: 按键发生变化时要执行的回调函数
 ====================================================================================*/
extern struct ll_keyWhich_t ll_keyWhich[LL_KEY_PORT_NUM];
void LLOS_Key_Register(ll_keyCB_t keyCB);

#ifdef __cplusplus
 }
#endif

#endif /* __LLOS_KEY_H */
