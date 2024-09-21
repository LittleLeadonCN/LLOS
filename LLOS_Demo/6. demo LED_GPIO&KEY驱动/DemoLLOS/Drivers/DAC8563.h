#ifndef __DAC8563_H
#define __DAC8563_H

#include <stdint.h> 
#include "llos.h"
#include "hal_spi1.h"

// configuration begin
#define DAC8563_NUM 			(1)
// configuration

enum DAC8563_addr_t
{
	addr_DAC_A = (0 << 16),
	addr_DAC_B = (1 << 16),
	addr_DAC_AB = (7 << 16),
	addr_GAIN = (2 << 16),
};

struct DAC8563_t
{
	ll_device_t *devSPI;
	enum SPI_cmd_CS_t CS_cmd;
};

void DAC8563_Init(void);
void DAC8563_SetID(uint8_t idN);
void DAC8563_Set_Val(enum DAC8563_addr_t ch, uint16_t val);

extern struct DAC8563_t DAC8563_InitStruct[DAC8563_NUM];

#endif
