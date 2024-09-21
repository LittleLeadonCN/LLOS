#include "llos.h"
#include "hal_gpio.h"

#include "gpio.h"

static ll_err_t HW_GPIOA_WritePin(struct ll_device *dev, uint32_t pin, ll_bit_t newState);
static uint32_t HW_GPIOA_ReadPin(struct ll_device *dev, uint32_t pin);

void LLOS_Device_Register_GPIO(void)
{
	ll_device_t dev = {0};

	dev.name = "GPIOA";
	dev.writePinCB = HW_GPIOA_WritePin;
	dev.readPinCB = HW_GPIOA_ReadPin;
	
	if(LLOS_Register_Device(&dev) == LL_ERR_INVALID)
	{
		LOG_E("%s register failed!\r\n", dev.name);
		while(1);
	}
}

static ll_err_t HW_GPIOA_WritePin(struct ll_device *dev, uint32_t pin, ll_bit_t newState)
{
	HAL_GPIO_WritePin(GPIOA, pin, (GPIO_PinState)newState);
	return LL_ERR_SUCCESS;
}
static uint32_t HW_GPIOA_ReadPin(struct ll_device *dev, uint32_t pin)
{
	return HAL_GPIO_ReadPin(GPIOA, pin);
}
