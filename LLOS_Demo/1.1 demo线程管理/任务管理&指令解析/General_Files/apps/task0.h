#ifndef TASK0_H
#define TASK0_H

#include "llos.h"

#define TASK0_EVENT1	LL_EVENT(0)
#define TASK0_EVENT2	LL_EVENT(1)
#define TASK0_EVENT3	LL_EVENT(2)
#define TASK0_EVENT4	LL_EVENT(3)
#define TASK0_EVENT5	LL_EVENT(4)

void Task0_Init(void);

extern ll_taskId_t task0Id;

#endif
