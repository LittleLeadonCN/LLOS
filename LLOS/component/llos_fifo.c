/*
 * @author LittleLeaf All rights reserved
 */
#include "llos_fifo.h"

static bool ready;

ll_err_t LLOS_FIFO_Init(llos_fifo_t *fifo, uint8_t *buffer, uint32_t size)
{
	if (size == 0 || (size & (4 - 1)) != 0 || ((size) & (size - 1)) != 0)
	{
		LL_LOG_E("%s ", "size format error!\r\n", __FUNCTION__);
		return LL_ERR_PARA;
	}
	if (buffer == NULL)
	{
		LL_LOG_E("%s ", "buffer null!\r\n", __FUNCTION__);
		return LL_ERR_NULL;
	}
	
	fifo->pHead = 0;
	fifo->pEnd = 0;
	fifo->pBuffer = buffer;
	fifo->size = size;
	memset(buffer, 0, size);

	ready = true;

	return LL_ERR_SUCCESS;
}

void LLOS_FIFO_Clear(llos_fifo_t *fifo)
{
	if (fifo == NULL)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return;
	}
	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return;
	}
	fifo->pHead = 0;
	fifo->pEnd = 0;
	memset(fifo->pBuffer, 0, fifo->size);
}

uint32_t LLOS_FIFO_Get_UsedSize(llos_fifo_t *fifo)
{
	if (fifo == NULL)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return 0xFFFFFFFF;
	}
	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return 0xFFFFFFFF;
	}
	return fifo->pEnd - fifo->pHead;
}

uint32_t LLOS_FIFO_Get_AvailableSize(llos_fifo_t *fifo)
{
	if (fifo == NULL)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return 0xFFFFFFFF;
	}
	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return 0xFFFFFFFF;
	}
	return fifo->size - LLOS_FIFO_Get_UsedSize(fifo);
}

uint32_t LLOS_FIFO_Input(llos_fifo_t *fifo, const uint8_t *pData, uint32_t len)
{
	if (fifo == NULL || pData == NULL || len == 0)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return 0;
	}
	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return 0;
	}

	uint32_t maxLen = LLOS_FIFO_Get_AvailableSize(fifo);
	uint32_t writeLen = LL_MIN(len, maxLen);
	uint32_t index = 0;

	for (index = 0; index < writeLen; index++)
	{
		fifo->pBuffer[fifo->pEnd & (fifo->size - 1)] = pData[index];
		fifo->pEnd++;
	}

	return writeLen;
}

uint32_t LLOS_FIFO_Output(llos_fifo_t *fifo, uint8_t *pData, uint32_t len)
{
	if (fifo == NULL || pData == NULL || len == 0)
	{
		LL_LOG_E("%s ", "parameter error!\r\n", __FUNCTION__);
		return 0;
	}
	if(!ready)
	{
		LL_LOG_E("%s ", "not initialized!\r\n", __FUNCTION__);
		return 0;
	}

	uint32_t maxLen = LLOS_FIFO_Get_UsedSize(fifo);
	uint32_t readLen = LL_MIN(len, maxLen);
	uint32_t index = 0;

	for (index = 0; index < readLen; index++)
	{
		pData[index] = fifo->pBuffer[fifo->pHead & (fifo->size - 1)];
		fifo->pHead++;
	}

	return readLen;
}
