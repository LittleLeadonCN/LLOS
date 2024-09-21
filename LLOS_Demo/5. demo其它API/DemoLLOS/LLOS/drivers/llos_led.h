/*
 * LED(GPIO输出)驱动，支持无阻塞的LED开关，特定闪烁次数、占空比和时长的LED控制，
 * 默认低电平有效。
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.1T
 * 修订日期: 2024 09 05
 * 修订日志:
 * N/A
 * 基础使用步骤:
 * 1) 初始化调用LLOS_LED_Init注册LED的GPIO初始化回调函数和设定LED的轮询周期
 * 随后系统会注册并启动相关任务
 * 2) 使用相关API控制LED
 * 3) ...
 */
#ifndef __LLOS_LED_H
#define __LLOS_LED_H

#include <llos.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define LL_LEDn(n)			LL_BV(n)

typedef enum
{
	ll_led_off,
	ll_led_on,
	ll_led_toggle,
}ll_led_t;

/*====================================================================================
 * 函数名: LLOS_LED_Init
 * 描述: 初始化
 * 参数:
 * 		ledInit: LED的GPIO初始化回调函数
 * 		ms: 毫秒，LED的轮询周期，一般为10
 *		timerN: 使用的OS定时器ID
 ====================================================================================*/
typedef void 				(*ll_ledInit_hook_t)(void);
void LLOS_LED_Init(ll_ledInit_hook_t ledInit, uint16_t ms, uint8_t timerN);

/*====================================================================================
 * 函数名: LLOS_LED_Set
 * 描述: 设置LED状态
 * 参数:
 * 		port: GPIO控制整组复位置位的寄存器地址
 * 		ledN: 哪一个GPIO，Bitmap，LL_LEDn(n)
 * 		mode: 要设置的LED状态
 ====================================================================================*/
void LLOS_LED_Set(ll_IO_t port, uint32_t ledN, ll_led_t mode);

/*====================================================================================
 * 函数名: LLOS_LED_Blink
 * 描述: 设置LED闪烁
 * 参数:
 * 		port: GPIO控制整组复位置位的寄存器地址
 * 		ledN: 哪一个GPIO，Bitmap，LL_LEDn(n)
 * 		num: 闪烁的次数，num = 255时永久闪烁
 * 		duty: 闪烁的占空比，受到闪烁周期和LED的轮询周期的影响
 * 		ms: 闪烁的周期
 ====================================================================================*/
void LLOS_LED_Blink(ll_IO_t port, uint32_t ledN, uint8_t num, uint8_t duty, uint16_t ms);

#ifdef __cplusplus
 }
#endif

#endif /* __LLOS_LED_H */
