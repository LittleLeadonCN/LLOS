#ifndef TASK1_H
#define TASK1_H

#include "llos.h"

#define TASK1_EVENT1	LL_EVENT(0)
#define TASK1_EVENT2	LL_EVENT(1)
#define TASK1_EVENT3	LL_EVENT(2)
#define TASK1_EVENT4	LL_EVENT(3)
#define TASK1_EVENT5	LL_EVENT(4)

void Task1_Init(void);

extern ll_taskId_t task1Id;
	
#endif
