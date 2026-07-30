/*
 * CRC通用驱动。
 * @author LittleLeaf All rights reserved
 * @version V2.0.0
 * @date 2026/06/10
 */
#ifndef LLOS_CRC_H
#define LLOS_CRC_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LL_CRC_USE_MODEL (1)

struct ll_crc_confStruct_t
{
	uint32_t initVal;
	uint32_t finalXOR;
	uint32_t poly;
	uint8_t width;
	bool isReverseInput;
	bool isReverseOutput;
};

/**
 * @brief CRC计算函数
 * @param[in] crc_confStruct: CRC配置结构体，可以使用CRC预设模型
 * @param[in] pData: 要校验的数据，若输入反转会破坏原始数据
 * @param[in] len: 要校验的数据的长度
 * @return CRC计算结果
 */
uint32_t LLOS_CRC_CAL(struct ll_crc_confStruct_t *crc_confStruct, uint8_t *pData, uint32_t len);

/* CRC预设模型 */
extern struct ll_crc_confStruct_t ll_crcModel_CRC4_ITU;
extern struct ll_crc_confStruct_t ll_crcModel_CRC5_EPC;
extern struct ll_crc_confStruct_t ll_crcModel_CRC5_ITU;
extern struct ll_crc_confStruct_t ll_crcModel_CRC5_USB;
extern struct ll_crc_confStruct_t ll_crcModel_CRC6_ITU;
extern struct ll_crc_confStruct_t ll_crcModel_CRC7_MMC;
extern struct ll_crc_confStruct_t ll_crcModel_CRC8;
extern struct ll_crc_confStruct_t ll_crcModel_CRC8_ITU;
extern struct ll_crc_confStruct_t ll_crcModel_CRC8_ROHC;
extern struct ll_crc_confStruct_t ll_crcModel_CRC8_MAXIM;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_IBM;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_MAXIM;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_USB;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_Modbus;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_CCITT;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_CCITT_FALSE;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_X25;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_XMODEM;
extern struct ll_crc_confStruct_t ll_crcModel_CRC16_DNP;
extern struct ll_crc_confStruct_t ll_crcModel_CRC32;
extern struct ll_crc_confStruct_t ll_crcModel_CRC32_MPEG_2;

#endif

#ifdef __cplusplus
}
#endif
