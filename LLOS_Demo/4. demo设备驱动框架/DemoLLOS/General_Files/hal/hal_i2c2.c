#include "system.h"
#include "hal_i2c2.h"

#include "i2c.h"

static ll_err_t HW_I2C2_Write(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len);
static ll_err_t HW_I2C2_Read(ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len);
static ll_err_t HW_I2C2_DMAWrite(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len);
static ll_err_t HW_I2C2_Ctrl(ll_device_t *dev, uint32_t cmd, void *arg);

void LLOS_Device_Register_I2C2(void)
{
	ll_device_t dev = {0};

	dev.name = "I2C2";
	dev.writeCB = HW_I2C2_Write;
	dev.readCB = HW_I2C2_Read;
	dev.DMA_writeCB = HW_I2C2_DMAWrite;
	dev.ctrlCB = HW_I2C2_Ctrl;
	if(LLOS_Register_Device(&dev) == LL_ERR_INVALID)
	{
		LOG_E("%s register failed!\r\n", dev.name);
		while(1);
	}
}

static ll_err_t HW_I2C2_Write(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len)
{
	HAL_I2C_Master_Transmit(&hi2c2, address, (uint8_t *)buffer + offset, len, 100);
    return LL_ERR_SUCCESS;
}
static ll_err_t HW_I2C2_Read(ll_device_t *dev, uint32_t address, uint32_t offset, void *buffer, uint32_t len)
{
	HAL_I2C_Master_Receive(&hi2c2, address, (uint8_t *)buffer + offset, len, 100);
    return LL_ERR_SUCCESS;
}
static ll_err_t HW_I2C2_DMAWrite(ll_device_t *dev, uint32_t address, uint32_t offset, const void *buffer, uint32_t len)
{
	HAL_I2C_Master_Transmit_DMA(&hi2c2, address, (uint8_t *)buffer + offset, len);
	return LL_ERR_SUCCESS;
}
static ll_err_t HW_I2C2_Ctrl(ll_device_t *dev, uint32_t cmd, void *arg)
{
	return LL_ERR_SUCCESS;
}
