#include "DAC8563.h"
#include "system.h"

/* CMD */
#define CMD_WR_REG 				(0x00 << 19)
#define CMD_SOFT_RST			(0x05 << 19)
#define CMD_PWR_SET				(0x04 << 19)
#define CMD_REF					(0x07 << 19)
#define CMD_WR_DAC				(0x03 << 19)
#define CMD_UP_DAC				(0x01 << 19)
	
enum DAC8563_gain_t
{
	gain_B2A2 = 0x00,
	gain_B2A1 = 0x01,
	gain_B1A2 = 0x02,
	gain_B1A1 = 0x03,
};

enum DAC8563_power_t
{
	pwr_UP = 0x00,
	pwr_DOWN_1K = 0x08,
	pwr_DOWN_100K = 0x10,
	pwr_DOWN_HIZ = 0x18,
};
enum DAC8563_powerN_t
{
	pwr_DAC_A = 0x01,
	pwr_DAC_B = 0x02,
	pwr_DAC_AB = 0x03,
};

static uint8_t id;
static uint8_t arg;

struct DAC8563_t DAC8563_InitStruct[DAC8563_NUM];

static void DAC8563_Write_Data(uint32_t data)
{
	uint8_t psend[3];
	
	psend[0] = (data >> 16) & 0xFF;
	psend[1] = (data >> 8) & 0xFF;
	psend[2] = (data >> 0) & 0xFF;
	
	arg = ll_reset;
	DAC8563_InitStruct[id].devSPI->ctrlCB(DAC8563_InitStruct[id].devSPI, DAC8563_InitStruct[id].CS_cmd, &arg);
	
	DAC8563_InitStruct[id].devSPI->write_readCB(DAC8563_InitStruct[id].devSPI, psend, NULL, 3);
	
	arg = ll_set;
	DAC8563_InitStruct[id].devSPI->ctrlCB(DAC8563_InitStruct[id].devSPI, DAC8563_InitStruct[id].CS_cmd, &arg);
}

static void DAC8563_Write_REG(enum DAC8563_addr_t redAddr, uint16_t data)
{
	uint32_t cmd;
	cmd = CMD_WR_REG | redAddr | data;
	DAC8563_Write_Data(cmd);
};

static void DAC8563_Reset(void)
{
	uint32_t data;
	data = CMD_SOFT_RST;
	DAC8563_Write_Data(data);
}

static void DAC8563_Set_Gain(enum DAC8563_gain_t gain)
{
	uint32_t data;
	data = CMD_WR_REG | addr_GAIN | gain;
	DAC8563_Write_Data(data);
}
static void DAC8563_Set_PWR(enum DAC8563_power_t mode, enum DAC8563_powerN_t dacN)
{
	uint32_t data;
	data = CMD_PWR_SET | mode | dacN;
	DAC8563_Write_Data(data);
}
static void DAC8563_Set_REF(ll_newState_t newState)
{
	uint32_t data;
	data = CMD_REF | newState;
	DAC8563_Write_Data(data);
};

void DAC8563_SetID(uint8_t idN)
{
	if(idN >= DAC8563_NUM)return;
	id = idN;
}

void DAC8563_Init(void)
{
	DAC8563_Reset();
	
	LLOS_DelayMs(10);
	
	DAC8563_Set_PWR(pwr_UP, pwr_DAC_AB);
	DAC8563_Set_REF(ll_disable);
	DAC8563_Set_Gain(gain_B1A1);
	
	DAC8563_Set_Val(addr_DAC_AB, 0);
}

void DAC8563_Set_Val(enum DAC8563_addr_t ch, uint16_t val)
{
	uint32_t data;
	data = CMD_WR_DAC | ch | val;
	DAC8563_Write_Data(data);
}
