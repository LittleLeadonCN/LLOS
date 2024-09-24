#include "ADS1120.h"
#include "system.h"

#define ADS1120_CMD_PWRDOWN		(0x02)
#define ADS1120_CMD_RESET		(0x06)
#define ADS1120_CMD_START_SYNC	(0x08)
#define ADS1120_CMD_RDATA		(0x10)
#define ADS1120_CMD_RREG		(0x20)
#define ADS1120_CMD_WREG		(0x40)

enum ADS1120_addrREG_t
{
	ADS1120_REG_MUX = (0x00 << 2),
	ADS1120_REG_CONFIG1 = (0x01 << 2),
	ADS1120_REG_CONFIG2 = (0x02 << 2),
	ADS1120_REG_CONFIG3 = (0x03 << 2),
};

static uint8_t id;
static uint8_t arg;

struct ADS1120_t ADS1120_InitStruct[ADS1120_NUM];

static void ADS1120_Write_Data(uint8_t *data, uint32_t len)
{
	arg = ll_reset;
	ADS1120_InitStruct[id].devSPI->ctrlCB(ADS1120_InitStruct[id].devSPI, ADS1120_InitStruct[id].CS_cmd, &arg);
	
	ADS1120_InitStruct[id].devSPI->write_readCB(ADS1120_InitStruct[id].devSPI, data, NULL, len);
	
	arg = ll_set;
	ADS1120_InitStruct[id].devSPI->ctrlCB(ADS1120_InitStruct[id].devSPI, ADS1120_InitStruct[id].CS_cmd, &arg);
}
static void ADS1120_Read_Data(const uint8_t *wdata, uint8_t *rdata, uint32_t len)
{
	arg = ll_reset;
	ADS1120_InitStruct[id].devSPI->ctrlCB(ADS1120_InitStruct[id].devSPI, ADS1120_InitStruct[id].CS_cmd, &arg);
	
	ADS1120_InitStruct[id].devSPI->write_readCB(ADS1120_InitStruct[id].devSPI, wdata, rdata, len);
	
	arg = ll_set;
	ADS1120_InitStruct[id].devSPI->ctrlCB(ADS1120_InitStruct[id].devSPI, ADS1120_InitStruct[id].CS_cmd, &arg);
}
static ll_err_t ADS1120_Wait(uint32_t time)
{
	uint32_t i = 0;
	while(ADS1120_InitStruct[id].devDRGPIO->readPinCB(ADS1120_InitStruct[id].devDRGPIO, ADS1120_InitStruct[id].devDRPin))
	{
		i++;
		if(i > time)return LL_ERR_FAIL;
	}
	return LL_ERR_SUCCESS;
}

static void ADS1120_WriteREG(enum ADS1120_addrREG_t regAddr, uint8_t val)
{
	uint8_t psend[2];
	
	psend[0] = ADS1120_CMD_WREG | regAddr;
	psend[1] = val;
	
	ADS1120_Write_Data(psend, 2);
}
static uint8_t ADS1120_ReadREG(enum ADS1120_addrREG_t regAddr)
{
	uint8_t prev[2];
	uint8_t psend[2];
	
	psend[0] = ADS1120_CMD_RREG | regAddr;
	psend[1] = 0x00;

	ADS1120_Read_Data(psend, prev, 2);
	
	return prev[1];
}
static void ADS1120_Reset(void)
{
	uint8_t cmd = ADS1120_CMD_RESET;
	ADS1120_Write_Data(&cmd, 1);
}
static void ADS1120_Start(void)
{
	uint8_t cmd = ADS1120_CMD_START_SYNC;
	ADS1120_Write_Data(&cmd, 1);
}

void ADS1120_SetID(uint8_t idN)
{
	if(idN >= ADS1120_NUM)return;
	id = idN;
}

void ADS1120_Init(void)
{
	ADS1120_Reset();
	
	LLOS_DelayMs(10);
	
	ADS1120_WriteREG(ADS1120_REG_CONFIG1, 0x50); /*180sps */
	ADS1120_WriteREG(ADS1120_REG_CONFIG2, 0x40);

	ADS1120_Start();
}

uint16_t ADS1120_ReadData_SingleShot(enum ADS1120_channel_t ch, enum ADS1120_gain_t gain)
{
	uint8_t rev;
	uint8_t cmd;
	uint16_t revData;
	uint8_t psend[2];
	uint8_t prev[2];
	
	cmd = ADS1120_ReadREG(ADS1120_REG_MUX) & 0x01;
	ADS1120_WriteREG(ADS1120_REG_MUX, cmd | ch | gain);

	psend[0] = 0xFF;
	psend[1] = 0xFF;

	ADS1120_Start();
	
	if(ADS1120_Wait(80000) != LL_ERR_SUCCESS)return 0;
	
	ADS1120_Read_Data(psend, prev, 2);
	
	revData = prev[0] & 0x00FF;
	revData = (revData << 8) | prev[1];
	
	return revData;
}
