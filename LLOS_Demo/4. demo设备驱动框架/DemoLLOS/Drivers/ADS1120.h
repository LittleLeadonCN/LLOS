#ifndef __ADS1120_H
#define __ADS1120_H

#include <stdint.h>
#include "llos.h"
#include "hal_spi1.h"

// configuration begin
#define ADS1120_NUM 		(1)
// configuration end

enum ADS1120_channel_t
{
	ADS1120_CH0 = (0x08 << 4),
	ADS1120_CH1 = (0x09 << 4),
	ADS1120_CH2 = (0x0A << 4),
	ADS1120_CH3 = (0x0B << 4),
};

enum ADS1120_gain_t
{
	ADS1120_GAIN_1 = (0 << 1),
	ADS1120_GAIN_2 = (1 << 1),
	ADS1120_GAIN_4 = (2 << 1),
	ADS1120_GAIN_8 = (3 << 1),
	ADS1120_GAIN_16 = (4 << 1),
	ADS1120_GAIN_32 = (5 << 1),
	ADS1120_GAIN_64 = (6 << 1),
	ADS1120_GAIN_128 = (7 << 1),
	ADS1120_PGA_OFF = (0x01),
};

struct ADS1120_t
{
	ll_device_t *devSPI;
	ll_device_t *devDRGPIO;
	uint32_t devDRPin;
	enum SPI_cmd_CS_t CS_cmd;
};

void ADS1120_Init(void);
void ADS1120_SetID(uint8_t id);
uint16_t ADS1120_ReadData_SingleShot(enum ADS1120_channel_t ch, enum ADS1120_gain_t gain);

extern struct ADS1120_t ADS1120_InitStruct[ADS1120_NUM];

#endif
