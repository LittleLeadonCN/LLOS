/*
 * @author LittleLeaf All rights reserved
 */
#include "llos_crc.h"

#if LL_CRC_USE_MODEL
struct ll_crc_confStruct_t ll_crcModel_CRC4_ITU = {.initVal = 0x00, .finalXOR = 0x00, .poly = 0x03, .width = 4, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC5_EPC = {.initVal = 0x09, .finalXOR = 0x00, .poly = 0x09, .width = 5, .isReverseInput = false, .isReverseOutput = false};
struct ll_crc_confStruct_t ll_crcModel_CRC5_ITU = {.initVal = 0x00, .finalXOR = 0x00, .poly = 0x15, .width = 5, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC5_USB = {.initVal = 0x1F, .finalXOR = 0x1F, .poly = 0x05, .width = 5, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC6_ITU = {.initVal = 0x00, .finalXOR = 0x00, .poly = 0x03, .width = 6, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC7_MMC = {.initVal = 0x00, .finalXOR = 0x00, .poly = 0x09, .width = 7, .isReverseInput = false, .isReverseOutput = false};

struct ll_crc_confStruct_t ll_crcModel_CRC8 = {.initVal = 0x00, .finalXOR = 0x00, .poly = 0x07, .width = 8, .isReverseInput = false, .isReverseOutput = false};
struct ll_crc_confStruct_t ll_crcModel_CRC8_ITU = {.initVal = 0x00, .finalXOR = 0x55, .poly = 0x07, .width = 8, .isReverseInput = false, .isReverseOutput = false};
struct ll_crc_confStruct_t ll_crcModel_CRC8_ROHC = {.initVal = 0xFF, .finalXOR = 0x00, .poly = 0x07, .width = 8, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC8_MAXIM = {.initVal = 0x00, .finalXOR = 0x00, .poly = 0x31, .width = 8, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC16_IBM = {.initVal = 0x0000, .finalXOR = 0x0000, .poly = 0x8005, .width = 16, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC16_MAXIM = {.initVal = 0x0000, .finalXOR = 0xFFFF, .poly = 0x8005, .width = 16, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC16_USB = {.initVal = 0xFFFF, .finalXOR = 0xFFFF, .poly = 0x8005, .width = 16, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC16_Modbus = {.initVal = 0xFFFF, .finalXOR = 0x0000, .poly = 0x8005, .width = 16, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC16_CCITT = {.initVal = 0x0000, .finalXOR = 0x0000, .poly = 0x1021, .width = 16, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC16_CCITT_FALSE = {.initVal = 0xFFFF, .finalXOR = 0x0000, .poly = 0x1021, .width = 16, .isReverseInput = false, .isReverseOutput = false};
struct ll_crc_confStruct_t ll_crcModel_CRC16_X25 = {.initVal = 0xFFFF, .finalXOR = 0xFFFF, .poly = 0x1021, .width = 16, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC16_XMODEM = {.initVal = 0x0000, .finalXOR = 0x0000, .poly = 0x1021, .width = 16, .isReverseInput = false, .isReverseOutput = false};
struct ll_crc_confStruct_t ll_crcModel_CRC16_DNP = {.initVal = 0x0000, .finalXOR = 0xFFFF, .poly = 0x3D65, .width = 16, .isReverseInput = true, .isReverseOutput = true};
struct ll_crc_confStruct_t ll_crcModel_CRC32 = {.initVal = 0xFFFFFFFF, .finalXOR = 0xFFFFFFFF, .poly = 0x04C11DB7, .width = 32, .isReverseInput = true, .isReverseOutput = true}; // #
struct ll_crc_confStruct_t ll_crcModel_CRC32_MPEG_2 = {.initVal = 0xFFFFFFFF, .finalXOR = 0x00000000, .poly = 0x04C11DB7, .width = 32, .isReverseInput = false, .isReverseOutput = false};
#endif

static const uint8_t byte_rev_table[256] =
{
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

static uint8_t LLOS_CRC4567_CAL(struct ll_crc_confStruct_t *crc_confStruct, uint8_t *pData, uint32_t len);
static uint32_t LLOS_CRC16_32_CAL(struct ll_crc_confStruct_t *crc_confStruct, uint8_t *pData, uint32_t len);

/* 反转输出结果的位 */
static uint32_t ReverseOutput(uint32_t value, uint8_t width)
{
	uint32_t reversed = 0x00000000;
	for (uint8_t i = 0; i < width; i++)
		reversed |= ((value >> i) & 0x00000001) << (width - 1 - i); /* 反转每一位 */
	return reversed;
}
/* 反转输入数据的每个字节 */
static void ReverseInputData(uint8_t *pData, uint32_t len)
{
	// 查表法
	for (uint32_t i = 0; i < len; i++)
	{
		pData[i] = byte_rev_table[pData[i]];
	}
	// for(uint32_t i = 0; i < len; i++)
	//     pData[i] = ReverseOutput(pData[i], 8);	/* 反转字节 */
}

uint32_t LLOS_CRC_CAL(struct ll_crc_confStruct_t *crc_confStruct, uint8_t *pData, uint32_t len)
{
	uint32_t crc;

	if (crc_confStruct == NULL)
		return 0xFFFFFFFF;
	if (crc_confStruct->width != 4 && crc_confStruct->width != 5 && crc_confStruct->width != 6 && crc_confStruct->width != 7 && crc_confStruct->width != 8 && crc_confStruct->width != 16 && crc_confStruct->width != 32)
		return 0xFFFFFFFF;

	if (crc_confStruct->isReverseInput)
		ReverseInputData(pData, len); /* 输入反转 */

	if (crc_confStruct->width == 4 || crc_confStruct->width == 5 || crc_confStruct->width == 6 || crc_confStruct->width == 7)
	{
		crc = LLOS_CRC4567_CAL(crc_confStruct, pData, len);
	}
	else
	{
		crc = LLOS_CRC16_32_CAL(crc_confStruct, pData, len);
	}

	if (crc_confStruct->isReverseOutput)
		crc = ReverseOutput(crc, crc_confStruct->width); /* 输出反转 */
	crc ^= crc_confStruct->finalXOR;
	if (crc_confStruct->isReverseInput)
		ReverseInputData(pData, len); /* 再次反转恢复数据 */

	return crc;
}

static uint32_t LLOS_CRC16_32_CAL(struct ll_crc_confStruct_t *crc_confStruct, uint8_t *pData, uint32_t len)
{
	uint32_t crc = crc_confStruct->initVal;
	uint64_t mask = (crc_confStruct->width == 32) ? 0xFFFFFFFF : (1ULL << crc_confStruct->width) - 1; /* 计算CRC的掩码 */

	for (uint32_t i = 0; i < len; i++)
	{
		crc ^= (pData[i] << (crc_confStruct->width - 8)); /* 将数据的字节对齐到CRC宽度# */

		for (uint8_t j = 0; j < 8; j++)
		{
			if (crc & (1 << (crc_confStruct->width - 1))) /* 检查CRC的最高位 */
				crc = (crc << 1) ^ crc_confStruct->poly;  /* 使用配置的多项式进行异或 */
			else
				crc <<= 1;
		}
		crc &= mask; /* 保证CRC的结果不超出指定宽度 */
	}

	return crc;
}
static uint8_t LLOS_CRC4567_CAL(struct ll_crc_confStruct_t *crc_confStruct, uint8_t *pData, uint32_t len)
{
	uint8_t crc = crc_confStruct->initVal << (8 - crc_confStruct->width);

	for (uint32_t i = 0; i < len; i++)
	{
		crc ^= pData[i];

		for (uint8_t j = 0; j < 8; j++)
		{
			if (crc & 0x80)
				crc = (crc << 1) ^ (crc_confStruct->poly << (8 - crc_confStruct->width));
			else
				crc <<= 1;
		}
	}

	crc = (crc >> (8 - crc_confStruct->width));

	return crc;
}
