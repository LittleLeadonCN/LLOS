#ifndef __TASK0_H
#define __TASK0_H

#include <llos.h>

#define TASK0_EVENT1	0x0001
#define TASK0_EVENT2	0x0002
#define TASK0_EVENT3	0x0004
#define TASK0_EVENT4	0x0008
#define TASK0_EVENT5	0x0010
#define TASK0_EVENT6	0x0020
#define TASK0_EVENT7	0x0040
#define TASK0_EVENT8	0x0080
#define TASK0_EVENT9	0x0100
#define TASK0_EVENT10	0x0200
#define TASK0_EVENT11	0x0400
#define TASK0_EVENT12	0x0800
#define TASK0_EVENT13	0x1000
#define TASK0_EVENT14	0x2000
#define TASK0_EVENT15	0x4000
#define TASK0_MSG		0x8000

void Task0_Init(void);

extern ll_taskId_t task0Id;

#endif
