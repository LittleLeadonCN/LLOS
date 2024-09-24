#include "task0.h"

ll_taskId_t task0Id = LL_ERR_INVALID;

ll_taskEvent_t Task0_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & LL_EVENT_MSG)
	{
		LOG_I("task%d receive message: %s\r\n", task0Id, (char *)LLOS_Msg_Receive());
		LLOS_Msg_Clear();
		return LL_EVENT_MSG;
	}
	if(events & TASK0_EVENT1)
	{
		LOG_I("task%d event1\r\n", task0Id);
		return TASK0_EVENT1;
	}
	if(events & TASK0_EVENT2)
	{
		LOG_I("task%d event2\r\n", task0Id);
		return TASK0_EVENT2;
	}
	if(events & TASK0_EVENT3)
	{
		LOG_I("task%d event3\r\n", task0Id);
		return TASK0_EVENT3;
	}
	if(events & TASK0_EVENT4)
	{
		LOG_I("task%d event4\r\n", task0Id);
		return TASK0_EVENT4;
	}
	if(events & TASK0_EVENT5)
	{
		LOG_I("task%d event5\r\n", task0Id);
		return TASK0_EVENT5;
	}
	if(events & TASK0_EVENT6)
	{
		LOG_I("task%d event6\r\n", task0Id);
		return TASK0_EVENT6;
	}
	if(events & TASK0_EVENT7)
	{
		LOG_I("task%d event7\r\n", task0Id);
		return TASK0_EVENT7;
	}
	if(events & TASK0_EVENT8)
	{
		LOG_I("task%d event8\r\n", task0Id);
		return TASK0_EVENT8;
	}
	if(events & TASK0_EVENT9)
	{
		LOG_I("task%d event9\r\n", task0Id);
		return TASK0_EVENT9;
	}
	if(events & TASK0_EVENT10)
	{
		LOG_I("task%d event10\r\n", task0Id);
		return TASK0_EVENT10;
	}
	if(events & TASK0_EVENT11)
	{
		LOG_I("task%d event11\r\n", task0Id);
		return TASK0_EVENT11;
	}
	if(events & TASK0_EVENT12)
	{
		LOG_I("task%d event12\r\n", task0Id);
		return TASK0_EVENT12;
	}
	if(events & TASK0_EVENT13)
	{
		LOG_I("task%d event13\r\n", task0Id);
		return TASK0_EVENT13;
	}
	if(events & TASK0_EVENT14)
	{
		LOG_I("task%d event14\r\n", task0Id);
		return TASK0_EVENT14;
	}
	if(events & TASK0_EVENT15)
	{
		LOG_I("task%d event15\r\n", task0Id);
		return TASK0_EVENT15;
	}
	
	return 0xFFFF;
}

void Task0_Init(void)
{
    task0Id = LLOS_Register_Events(Task0_Events);
    if(task0Id == LL_ERR_INVALID)
    {
    	LOG_I("Task0 init failed!\r\n");
		while(1);
    }
	
	LLOS_Start_Event(task0Id, TASK0_EVENT1 | TASK0_EVENT2 | TASK0_EVENT3 | TASK0_EVENT4 | TASK0_EVENT5 |
								TASK0_EVENT6 | TASK0_EVENT7 | TASK0_EVENT8 | TASK0_EVENT9 | TASK0_EVENT10 |
								TASK0_EVENT11 | TASK0_EVENT12 | TASK0_EVENT13 | TASK0_EVENT14 | TASK0_EVENT15 | TASK0_MSG, LLOS_Ms_To_Tick(0)); /* 优先级测试 */
//	LLOS_Stop_Event(task0Id, TASK0_EVENT1 | TASK0_EVENT2);
}
