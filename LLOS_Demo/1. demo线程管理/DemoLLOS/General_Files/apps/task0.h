#ifndef __TASK0_H
#define __TASK0_H

#include <llos.h>

#define TASK0_EVENT1	0x0001
#define TASK0_EVENT2	0x0002
#define TASK0_EVENT3	0x0004
#define TASK0_EVENT4	0x0008
#define TASK0_EVENT5	0x0010

void Task0_Init(void);

extern ll_taskId_t task0Id;

#endif
