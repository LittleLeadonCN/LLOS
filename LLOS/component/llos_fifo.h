/*
 * 环形fifo
 * @author LittleLeaf All rights reserved
 * @version V1.0.1
 * @date 2026/06/05
 */
#ifndef LLOS_FIFO_H
#define LLOS_FIFO_H

#include "llos.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint32_t pHead;
    uint32_t pEnd;
    uint32_t size;
    uint8_t *pBuffer;
} llos_fifo_t;

/**
    * @brief 初始化fifo
    * @param[in] fifo: 指定的缓冲区结构体
    * @param[in] buffer: 缓冲区地址
    * @param[in] size: 缓冲区大小(要求为2的指数次幂大小)
    * @return 错误码
    */
ll_err_t LLOS_FIFO_Init(llos_fifo_t *fifo, uint8_t *buffer, uint32_t size);

/**
    * @brief 重置fifo
    * @param[in] fifo: 指定的缓冲区结构体
    */
void LLOS_FIFO_Clear(llos_fifo_t *fifo);

/**
    * @brief 获取已使用的fifo大小
    * @param[in] fifo: 指定的缓冲区结构体
    * @return 已使用的fifo大小
    */
uint32_t LLOS_FIFO_Get_UsedSize(llos_fifo_t *fifo);

/**
    * @brief 获取剩余的的fifo大小
    * @param[in] fifo: 指定的缓冲区结构体
    * @return 剩余的fifo大小
    */
uint32_t LLOS_FIFO_Get_AvailableSize(llos_fifo_t *fifo);

/**
    * @brief 数据输入
    * @param[in] fifo: 指定的缓冲区结构体
    * @param[in] pData: 数据地址
    * @param[in] len: 数据长度
    * @return 成功写入的数据量
    */
uint32_t LLOS_FIFO_Input(llos_fifo_t *fifo, const uint8_t *pData, uint32_t len);

/**
    * @brief 数据输出
    * @param[in] fifo: 指定的缓冲区结构体
    * @param[out] pData: 数据地址
    * @param[in] len: 数据长度
    * @return 成功读出的数据量
    */
uint32_t LLOS_FIFO_Output(llos_fifo_t *fifo, uint8_t *pData, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
