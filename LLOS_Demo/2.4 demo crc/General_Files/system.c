#include "system.h"
#include "CH58x_common.h"
#include "llos.h"
#include "llos_crc.h"

/*
A3 F1 2C 89 5E 0B D4 76
1F 8A 42 E7 3D C0 95 68
BA 04 D8 51 6F 23 9E C7
10 FA 8C 36 5B E1 7D 92
B4 0E 6A C5 39 8F 27 D1
63 AC F9 45 8E 18 B7 02
EC 5F 23 91 4D A6 78 CF
09 DE 34 62 FB 87 41 A5
*/
static uint8_t random_data[64] =
{
    0xA3, 0xF1, 0x2C, 0x89, 0x5E, 0x0B, 0xD4, 0x76,
    0x1F, 0x8A, 0x42, 0xE7, 0x3D, 0xC0, 0x95, 0x68,
    0xBA, 0x04, 0xD8, 0x51, 0x6F, 0x23, 0x9E, 0xC7,
    0x10, 0xFA, 0x8C, 0x36, 0x5B, 0xE1, 0x7D, 0x92,
    0xB4, 0x0E, 0x6A, 0xC5, 0x39, 0x8F, 0x27, 0xD1,
    0x63, 0xAC, 0xF9, 0x45, 0x8E, 0x18, 0xB7, 0x02,
    0xEC, 0x5F, 0x23, 0x91, 0x4D, 0xA6, 0x78, 0xCF,
    0x09, 0xDE, 0x34, 0x62, 0xFB, 0x87, 0x41, 0xA5
};

void System_Init(void)
{
    GPIOB_ModeCfg(GPIO_Pin_22, GPIO_ModeIN_PU);

	uint16_t crc;
	crc = LLOS_CRC_CAL(&ll_crcModel_CRC16_Modbus, random_data, sizeof(random_data));
	printf("CRC: %04X\r\n", crc);
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
