/*
 * @author LittleLeaf All rights reserved
 */
#ifndef LLOS_CONF_H
#define LLOS_CONF_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ==========================[kernel]========================== */
#define LL_COROUTINES_MAX           2

#define LL_USE_CMD_SHELL            1
#define ll_cmd_printf(x...)         printf(x) /* 指令解析输出映射 */

#define LL_LOG_LEVEL (3) /* 打印日志等级 */

#if (LL_LOG_LEVEL > 3)
#ifndef LL_LOG_D
#define LL_LOG_D(X, x...)           printf("[LLOS DEBUG] " X x)
#endif
#else
#ifndef LL_LOG_D
#define LL_LOG_D(X, x...) ((void)0)
#endif
#endif

#if (LL_LOG_LEVEL > 2)
#ifndef LL_LOG_W
#define LL_LOG_W(X, x...)           printf("[LLOS WARNING] " X x)
#endif
#else
#ifndef LL_LOG_W
#define LL_LOG_W(X, x...) ((void)0)
#endif
#endif

#if (LL_LOG_LEVEL > 1)
#ifndef LL_LOG_E
#define LL_LOG_E(X, x...)           printf("[LLOS ERROR] " X x)
#endif
#else
#ifndef LL_LOG_E
#define LL_LOG_E(X, x...) ((void)0)
#endif
#endif

#if (LL_LOG_LEVEL > 0)
#ifndef LL_LOG_I
#define LL_LOG_I(x...)              printf("[LLOS INFO] " x)
#endif
#else
#ifndef LL_LOG_I
#define LL_LOG_I(X, x...) ((void)0)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* LLOS_CONF_H */
