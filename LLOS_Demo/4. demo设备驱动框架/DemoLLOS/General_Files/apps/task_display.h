#ifndef __TASK_DISPLAY_H
#define __TASK_DISPLAY_H

#include <llos.h>

#define TASK_DISPLAY_EVENT1		0x0001

void TaskDisplay_Init(void);

extern ll_taskId_t taskDisplayId;

#endif
