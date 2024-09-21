/*
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.0T
 * 修订日期: 2024 09 12
 * 修订日志:
 * N/A
 */
#include <llos_ssd1306_conf.h>

#include "hal_spi1.h"
#include "hal_i2c2.h"

static uint8_t arg;
static ll_device_t *devGPIOA, *devSPI1, *devI2C2;

static void I8080_WriteByteCB(uint8_t data, enum ll_SSD1306_cmdType_t cmd);
static void I8080_DMAWriteCB(const uint8_t *pic, uint32_t len);

void LLOS_SSD1306_HAL_Init(void)
{
	ll_SSD1306_screenConfig_t SSD1306_screenConfig = {0};
	
	devGPIOA = LLOS_Device_Find("GPIOA");
	if(devGPIOA == NULL)
	{
    	LOG_E("GPIOA Not Found!\r\n");
		while(1);
	}
	
	devSPI1 = LLOS_Device_Find("SPI1");
	if(devSPI1 == NULL)
	{
    	LOG_E("SPI1 Not Found!\r\n");
		while(1);
	}
	
	devI2C2 = LLOS_Device_Find("I2C2");
	if(devI2C2 == NULL)
	{
    	LOG_E("I2C2 Not Found!\r\n");
		while(1);
	}
	
	SSD1306_screenConfig.type = ll_SSD1306_screenType_128x64;
	SSD1306_screenConfig.xOffset = ll_SSD1306_screen_xOffset_0_96;
	SSD1306_screenConfig.isMirrot = false;
	SSD1306_screenConfig.isInvert = false;
	SSD1306_screenConfig.isInvertPhase = false;
	SSD1306_screenConfig.brightness = 255;
	LLOS_SSD1306_Init(I8080_WriteByteCB, NULL, &SSD1306_screenConfig);
//	LLOS_SSD1306_Init(I8080_WriteByteCB, I8080_DMAWriteCB, &SSD1306_screenConfig);
}

static void I8080_WriteByteCB(uint8_t data, enum ll_SSD1306_cmdType_t cmd)
{
#ifdef SPI_INTERFACE
	arg = ll_reset;
	LLOS_Device_Ctrl(devSPI1, device_DISPLAY1_Cmd_CS, &arg);

	LLOS_Device_WritePin(DISPLAY1_DC_PORT, DISPLAY1_DC_PIN, (uint8_t)cmd);
	LLOS_Device_Write(devSPI1, 0, 0, &data, 1);
	
	arg = ll_set;
	LLOS_Device_Ctrl(devSPI1, device_DISPLAY1_Cmd_CS, &arg);
#else
	uint8_t buffer[2];
	
	buffer[0] = cmd ? 0x40:0x00;
	buffer[1] = data;
	
	LLOS_Device_Write(devI2C2, SSD1306_I2C_ADDR, 0, buffer, 2);
#endif
}

static void I8080_DMAWriteCB(const uint8_t *pic, uint32_t len)
{
#ifdef SPI_INTERFACE	
	arg = ll_reset;
	LLOS_Device_Ctrl(devSPI1, device_DISPLAY1_Cmd_CS, &arg);
	
	LLOS_Device_WritePin(DISPLAY1_DC_PORT, DISPLAY1_DC_PIN, (uint8_t)ll_SSD1306_cmd_Data);
	LLOS_Device_DMAWrite(devSPI1, 0, 0, pic, len);
	
	LLOS_DelayMs(1);
	arg = ll_set;
	LLOS_Device_Ctrl(devSPI1, device_DISPLAY1_Cmd_CS, &arg);
#endif
}
