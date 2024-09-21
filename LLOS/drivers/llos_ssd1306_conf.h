/*
 * SSD1306/CH1115以及兼容的屏幕驱动HAL层
 * 作者: LittleLeaf All rights reserved
 * 版本: V1.0.0T
 * 修订日期: 2024 09 12
 * 修订日志:
 * N/A
 */
#ifndef __LLOS_SSD1306_CONF_H
#define __LLOS_SSD1306_CONF_H

#include <llos_ssd1306.h>

#define SSD1306_I2C_ADDR		((0x3C << 1) | 0)

#define DISPLAY1_DC_PORT		devGPIOA
#define DISPLAY1_DC_PIN     	LL_BV(12)

#define SPI_INTERFACE

#ifndef SPI_INTERFACE
#define I2C_INTERFACE
#endif

void LLOS_SSD1306_HAL_Init(void);

#endif
