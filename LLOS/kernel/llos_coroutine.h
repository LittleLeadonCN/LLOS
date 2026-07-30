/*
 * @author LittleLeaf All rights reserved
 * 协程内禁止使用: goto/return/break/continue/
 * switch/case/default/局部变量(使用ctx->priv替代)
 */
#ifndef LLOS_COROUTINE_H
#define LLOS_COROUTINE_H

#include "llos_conf.h"

#if LL_COROUTINES_MAX > 0

struct ll_coroutine_ctx_t
{
    int state;
    uint64_t wakeTick;
    uint32_t eventMask; 
    void *priv;
};

typedef void (*ll_coroutine_CB)(struct ll_coroutine_ctx_t *ctx, void *arg);

struct ll_coroutine_task_t
{
    struct ll_coroutine_ctx_t *ctx;
    ll_coroutine_CB CB;
    void *arg;
};

void LLOS_Coroutine_Register(struct ll_coroutine_ctx_t *ctx, ll_coroutine_CB CB, void *arg);
void LLOS_Coroutine_SetEvents(uint32_t events);
uint32_t LLOS_Coroutine_GetEvents(void);

// ---- 协程控制宏 ----
#define LLOS_CR_BEGIN(ctx) \
    enum { __llos_cr_begin_dummy = __COUNTER__ }; \
    switch((ctx)->state) { \
        case 0:

#define LLOS_CR_END() \
    } \
    (ctx)->state = -1; \
    return;

#define LLOS_CR_Yield(ctx) \
    do { \
		enum { __cr_line = __COUNTER__ }; \
        (ctx)->state = __cr_line; \
        return; \
        case __cr_line:; \
    } while(0)

#define LLOS_CR_Sleep(ctx, ticks) \
    do { \
		enum { __cr_line = __COUNTER__ }; \
        (ctx)->wakeTick = LLOS_Get_SysTick() + (ticks); \
        (ctx)->state = __cr_line; \
        return; \
        case __cr_line: \
            if (LLOS_Get_SysTick() < (ctx)->wakeTick) return; \
    } while(0)

#define LLOS_CR_WaitEvent(ctx, eventMask) \
    do { \
		enum { __cr_line = __COUNTER__ }; \
        (ctx)->eventMask = (eventMask); \
        (ctx)->state = __cr_line; \
        return; \
        case __cr_line: \
            if (!((ctx)->eventMask & LLOS_Coroutine_GetEvents())) return; \
    } while(0)

#define LLOS_CR_WaitUntil(ctx, condition) \
    do { \
		enum { __cr_line = __COUNTER__ }; \
        (ctx)->state = __cr_line; \
        return; \
        case __cr_line: \
            if (!(condition)) return; \
    } while(0)

#define LLOS_CR_WaitTimeout(ctx, condition, ticks) \
    do { \
		enum { __cr_line = __COUNTER__ }; \
        (ctx)->wakeTick = LLOS_Get_SysTick() + (ticks); \
        (ctx)->state = __cr_line; \
        return; \
        case __cr_line: \
            if (LLOS_Get_SysTick() >= (ctx)->wakeTick) { \
                /* 超时，继续执行 */ \
            } else if (!(condition)) { \
                return; \
            } \
    } while(0)

#define LLOS_CR_Reset(ctx) do { (ctx)->state = 0; } while(0)
#define LLOS_CR_IsFinished(ctx) ((ctx)->state == -1)

#endif

#endif
