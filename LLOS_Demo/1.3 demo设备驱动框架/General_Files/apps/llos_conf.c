/*
 * 作者: LittleLeaf All rights reserved
 */
#include "llos.h"
#include "app_LED.h"

/* =====================================[指令解析框架]====================================== */
extern ll_err_t LLOS_Cmd_IDN(ll_cmd_t *context);
extern ll_err_t LLOS_Cmd_RST(ll_cmd_t *context);

ll_err_t LLOS_Cmd_Test(ll_cmd_t *context)
{
	/* 测试指令 *Test FALSE,2.66mV,-2kA,"Hello world!" */
	bool a;
	float b;
	int32_t c;
	char s[11];

	if (LLOS_Cmd_ParamBool(context, &a))
		return LL_ERR_FAILED;
	if (LLOS_Cmd_ParamFloat(context, &b))
		return LL_ERR_FAILED;
	if (LLOS_Cmd_ParamInt32(context, &c))
		return LL_ERR_FAILED;
	if (LLOS_Cmd_ParamCopyText(context, s, 10))
		return LL_ERR_FAILED;

	ll_cmd_printf("*Test -> %d, %f, %d, %s\r\n", a, b, c, s);

	return LL_ERR_SUCCESS;
}

ll_err_t LLOS_Cmd_Factory(ll_cmd_t *context)
{
	// user code

	return LL_ERR_SUCCESS;
}

// user code begin
ll_err_t LEDR(ll_cmd_t *context)
{
	/* 测试指令 *Test FALSE,2.66mV,-2kA,"Hello world!" */
	bool param;

	if (LLOS_Cmd_ParamBool(context, &param))
		return LL_ERR_FAILED;

	App_LED_Ctrl(App_LED_color_R, param);

	return LL_ERR_SUCCESS;
}
ll_err_t LEDG(ll_cmd_t *context)
{
	/* 测试指令 *Test FALSE,2.66mV,-2kA,"Hello world!" */
	bool param;

	if (LLOS_Cmd_ParamBool(context, &param))
		return LL_ERR_FAILED;

	App_LED_Ctrl(App_LED_color_G, param);

	return LL_ERR_SUCCESS;
}
ll_err_t LEDB(ll_cmd_t *context)
{
	/* 测试指令 *Test FALSE,2.66mV,-2kA,"Hello world!" */
	bool param;

	if (LLOS_Cmd_ParamBool(context, &param))
		return LL_ERR_FAILED;

	App_LED_Ctrl(App_LED_color_B, param);

	return LL_ERR_SUCCESS;
}
// user code over

struct cmdList_t cmdList[] =
{
		/* 指令不区分大小写，参数用 , 分隔，字符串用 "" 包括 */
		/* 部分单位可自动转换，单位区分大小写 */

		{.pattern = "*IDN?", .callback = LLOS_Cmd_IDN},		   /* 查询ID信息 */
		{.pattern = "SYStem:RESet", .callback = LLOS_Cmd_RST}, /* 系统复位 */
		{.pattern = "*Test", .callback = LLOS_Cmd_Test},	   /* 测试 */
		{.pattern = "*RST", .callback = LLOS_Cmd_Factory},	   /* 恢复出厂设置 */

		// user code begin
		{.pattern = "LEDR", .callback = LEDR},
		{.pattern = "LEDG", .callback = LEDG},
		{.pattern = "LEDB", .callback = LEDB},
		// user code over

		{.pattern = "", .callback = NULL}, 						/* 结束标志 */
};
