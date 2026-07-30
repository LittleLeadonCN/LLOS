/*
 * 独立按键驱动，支持无阻塞的任意按键点击次数、长按及长按时间检测， 支持按下/弹起双状态和多按键
 * 同时检测，低电平有效。
 * @author LittleLeaf All rights reserved
 * @version V3.0.0
 * @date 2026/06/10
 * 基础使用步骤:
 * 1) 初始化调用LLOS_Key_Init
 * 2) 在回调函数里读取回调函数传入的按键信息
 * 3) ...
 */
#ifndef LLOS_KEY_H
#define LLOS_KEY_H

#include "llos.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LL_KEY_USE_LLOS 				1

#if LL_KEY_USE_LLOS
#define LL_KEY_MALLOC 					LLOS_malloc
#define ENTER_CRITICAL()				LLOS_Critical(1)
#define EXIT_CRITICAL()					LLOS_Critical(0)
#else
#ifndef ENTER_CRITICAL()
    #error "ENTER_CRITICAL not defined! Please define it (e.g., __disable_irq()) before including this header!"
#endif
#ifndef EXIT_CRITICAL()
    #error "EXIT_CRITICAL not defined! Please define it (e.g., __enable_irq()) before including this header!"
#endif

#define LL_KEY_MALLOC 					malloc
#endif

enum ll_keyEvent_t
{
	ll_key_event_NULL,
	ll_key_event_Click,
	ll_key_event_DoubleClick,
	ll_key_event_TripleClick,
	ll_key_event_LongPress,
};

struct ll_keyConfig_t
{
	ll_IO_t port;	  			/* GPIO存储引脚输入状态的寄存器地址 */
	uint32_t pinMask; 			/* 使用到的Pin掩码 */
};

struct ll_keyWhich_t
{
	uint32_t pin;				/* 哪个按键 */
	enum ll_keyEvent_t event;	/* 按键事件(N击) */
	uint16_t pressTime;		 	/* 长按事件的时间(ms) */
};

/**
 * @brief 按键状态发生变化时被执行的回调函数
 * @param[out] portN: 返回哪组按键发生变化
 * @param[out] isUp: 当前按键是否已经弹起
 * @param[out] keyWhich: 按键信息
 */
typedef void (*ll_keyCB_t)(uint8_t portN, bool isUp, struct ll_keyWhich_t *keyWhich);

/**
 * @brief 初始化
 * @param[in] timerN: 使用的OS定时器ID，如不使用LLOS则忽略
 * @param[in] ms: 毫秒，按键的轮询周期，推荐20ms左右
 * @param[in] overTime: 检测时间阈值，推荐100ms左右
 * @param[in] longPressTime: 长按检测时间阈值，推荐800ms左右
 * @param[in] cfg: 按键配置
 * @param[in] portNum: port数量
 * @param[in] keyCB: 按键发生变化时要执行的回调函数
 */
void LLOS_Key_Init(uint8_t timerN, uint16_t ms, uint16_t overTime, uint16_t longPressTime, struct ll_keyConfig_t *cfg, uint8_t portNum, ll_keyCB_t keyCB);

#if !LL_KEY_USE_LLOS
/**
 * @brief 如果不使用LLOS，则需要按照初始化时指定的ms周期轮询该函数
 * @param[in] timerN: 未使用无需理会
 */
void LLOS_Key_Tick(uint8_t timerN);
#endif

#ifdef __cplusplus
}
#endif

#endif
