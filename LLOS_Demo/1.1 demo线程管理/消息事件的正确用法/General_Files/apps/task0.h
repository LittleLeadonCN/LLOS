#ifndef TASK0_H
#define TASK0_H

#include "llos.h"

struct task0_testStruct
{
	uint8_t a;
	uint16_t b;
	uint32_t c;
	char *d;
};

void Task0_Init(void);

extern ll_taskId_t task0Id;

#endif
