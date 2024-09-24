#ifndef __TASK1_H
#define __TASK1_H

#include <llos.h>

#define TASK1_EVENT1	0x0001
#define TASK1_EVENT2	0x0002
#define TASK1_EVENT3	0x0004
#define TASK1_EVENT4	0x0008
#define TASK1_EVENT5	0x0010

void Task1_Init(void);

extern ll_taskId_t task1Id;
	
#endif
