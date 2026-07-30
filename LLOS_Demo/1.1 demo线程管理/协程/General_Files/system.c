#include "system.h"
#include "CH58x_common.h"
#include "llos.h"

static struct ll_coroutine_ctx_t ctx1, ctx2;
static void CB1(struct ll_coroutine_ctx_t *ctx, void *arg)
{
	LLOS_CR_BEGIN(ctx);

	while(1)
	{
		printf("%s - 1\r\n", __FUNCTION__);
		LLOS_CR_Sleep(ctx, LLOS_Ms_To_Tick(500));
		printf("%s - 2\r\n", __FUNCTION__);
		LLOS_CR_Sleep(ctx, LLOS_Ms_To_Tick(1000));
		printf("%s - 3\r\n", __FUNCTION__);
		LLOS_CR_Sleep(ctx, LLOS_Ms_To_Tick(2000));
	}

	LLOS_CR_END();
}
static void CB2(struct ll_coroutine_ctx_t *ctx, void *arg)
{
	LLOS_CR_BEGIN(ctx);
	
	while(1)
	{
		printf("CPU:%d%%\r\n", LLOS_Get_CPU_Usage());
		LLOS_CR_Sleep(ctx, LLOS_Ms_To_Tick(1000));
	}

	LLOS_CR_END();
}

void System_Init(void)
{
    SysTick_Config(GetSysClock() / 1000);

    GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeIN_PU);

	LLOS_Coroutine_Register(&ctx1, CB1, NULL);
	LLOS_Coroutine_Register(&ctx2, CB2, NULL);
}

void System_Loop(void)
{
	LLOS_Loop();

    if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET)
    {
        DelayMs(1000);
        if(GPIOB_ReadPortPin(GPIO_Pin_22) == RESET) Jump2BOOT();
    }
}

__HIGH_CODE
void Jump2BOOT(void)
{
	FLASH_ROM_ERASE(0, EEPROM_BLOCK_SIZE);
	FLASH_ROM_SW_RESET();
	sys_safe_access_enable();
	R16_INT32K_TUNE = 0xFFFF;
	SYS_ResetExecute();
    sys_safe_access_disable();
    while(1);
}

__INTERRUPT
__HIGH_CODE 
void SysTick_Handler(void)
{
    LLOS_Tick_Increase();
    SysTick->SR = 0; 
}
