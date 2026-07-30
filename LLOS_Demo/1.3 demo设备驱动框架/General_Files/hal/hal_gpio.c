#include <llos.h>
#include "hal_gpio.h"
#include "CH58x_common.h"

static ll_err_t HW_GPIOB_Init(struct ll_device_t *dev, void *arg);
static ll_err_t HW_GPIOB_WritePin(struct ll_device_t *dev, uint32_t pin, ll_bit_t newState);
static uint32_t HW_GPIOB_ReadPin(struct ll_device_t *dev, uint32_t pin);

struct ll_deviceOps_t opsGPIOB =
{
	.initCB = HW_GPIOB_Init,
	.writePinCB = HW_GPIOB_WritePin,
	.readPinCB = HW_GPIOB_ReadPin,
};
void LLOS_Device_Register_GPIO(void)
{
	struct ll_device_t dev = {0};

	dev.name = "GPIOB";
	dev.ops = &opsGPIOB;
	
	if(LLOS_Register_Device(&dev) == LL_ERR_INVALID)
	{
		LL_LOG_E("%s ", "%s register failed!\r\n", __FUNCTION__, dev.name);
		while(1);
	}

	HW_GPIOB_Init(NULL, NULL);
}

static ll_err_t HW_GPIOB_Init(struct ll_device_t *dev, void *arg)
{
	GPIOB_SetBits(PIN_LEDR | PIN_LEDG | PIN_LEDB);
	GPIOB_ModeCfg(PIN_LEDR | PIN_LEDG | PIN_LEDB, GPIO_ModeOut_PP_20mA);
	return LL_ERR_SUCCESS;
}
static ll_err_t HW_GPIOB_WritePin(struct ll_device_t *dev, uint32_t pin, ll_bit_t newState)
{
	newState ? GPIOB_SetBits(pin) : GPIOB_ResetBits(pin);
	return LL_ERR_SUCCESS;
}
static uint32_t HW_GPIOB_ReadPin(struct ll_device_t *dev, uint32_t pin)
{
	return GPIOB_ReadPortPin(pin);
}
