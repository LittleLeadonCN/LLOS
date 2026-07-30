#include "task0.h"

ll_taskId_t task0Id = LL_ERR_INVALID;

static ll_taskEvent_t Task0_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & LL_EVENT_MSG)
	{
		LL_LOG_I("task%d receive message: %s\r\n", taskId, (char *)LLOS_Msg_Receive(taskId));
		LLOS_Msg_Clear(taskId);
		return LL_EVENT_MSG;
	}
	if(events & TASK0_EVENT0)
	{
		LL_LOG_I("task%d event0\r\n", taskId);
		return TASK0_EVENT0;
	}
	if(events & TASK0_EVENT1)
	{
		LL_LOG_I("task%d event1\r\n", taskId);
		return TASK0_EVENT1;
	}
	if(events & TASK0_EVENT2)
	{
		LL_LOG_I("task%d event2\r\n", taskId);
		return TASK0_EVENT2;
	}
	if(events & TASK0_EVENT3)
	{
		LL_LOG_I("task%d event3\r\n", taskId);
		return TASK0_EVENT3;
	}
	if(events & TASK0_EVENT4)
	{
		LL_LOG_I("task%d event4\r\n", taskId);
		return TASK0_EVENT4;
	}
	if(events & TASK0_EVENT5)
	{
		LL_LOG_I("task%d event5\r\n", taskId);
		return TASK0_EVENT5;
	}
	if(events & TASK0_EVENT6)
	{
		LL_LOG_I("task%d event6\r\n", taskId);
		return TASK0_EVENT6;
	}
	if(events & TASK0_EVENT7)
	{
		LL_LOG_I("task%d event7\r\n", taskId);
		return TASK0_EVENT7;
	}
	if(events & TASK0_EVENT8)
	{
		LL_LOG_I("task%d event8\r\n", taskId);
		return TASK0_EVENT8;
	}
	if(events & TASK0_EVENT9)
	{
		LL_LOG_I("task%d event9\r\n", taskId);
		return TASK0_EVENT9;
	}
	if(events & TASK0_EVENT10)
	{
		LL_LOG_I("task%d event10\r\n", taskId);
		return TASK0_EVENT10;
	}
	if(events & TASK0_EVENT11)
	{
		LL_LOG_I("task%d event11\r\n", taskId);
		return TASK0_EVENT11;
	}
	if(events & TASK0_EVENT12)
	{
		LL_LOG_I("task%d event12\r\n", taskId);
		return TASK0_EVENT12;
	}
	if(events & TASK0_EVENT13)
	{
		LL_LOG_I("task%d event13\r\n", taskId);
		return TASK0_EVENT13;
	}
	if(events & TASK0_EVENT14)
	{
		LL_LOG_I("task%d event14\r\n", taskId);
		return TASK0_EVENT14;
	}
	if(events & TASK0_EVENT15)
	{
		LL_LOG_I("task%d event15\r\n", taskId);
		return TASK0_EVENT15;
	}
	if(events & TASK0_EVENT16)
	{
		LL_LOG_I("task%d event16\r\n", taskId);
		return TASK0_EVENT16;
	}
	if(events & TASK0_EVENT17)
	{
		LL_LOG_I("task%d event17\r\n", taskId);
		return TASK0_EVENT17;
	}
	if(events & TASK0_EVENT18)
	{
		LL_LOG_I("task%d event18\r\n", taskId);
		return TASK0_EVENT18;
	}
	if(events & TASK0_EVENT19)
	{
		LL_LOG_I("task%d event19\r\n", taskId);
		return TASK0_EVENT19;
	}
	if(events & TASK0_EVENT20)
	{
		LL_LOG_I("task%d event20\r\n", taskId);
		return TASK0_EVENT20;
	}
	if(events & TASK0_EVENT21)
	{
		LL_LOG_I("task%d event21\r\n", taskId);
		return TASK0_EVENT21;
	}
	if(events & TASK0_EVENT22)
	{
		LL_LOG_I("task%d event22\r\n", taskId);
		return TASK0_EVENT22;
	}
	if(events & TASK0_EVENT23)
	{
		LL_LOG_I("task%d event23\r\n", taskId);
		return TASK0_EVENT23;
	}
	if(events & TASK0_EVENT24)
	{
		LL_LOG_I("task%d event24\r\n", taskId);
		return TASK0_EVENT24;
	}
	if(events & TASK0_EVENT25)
	{
		LL_LOG_I("task%d event25\r\n", taskId);
		return TASK0_EVENT25;
	}
	if(events & TASK0_EVENT26)
	{
		LL_LOG_I("task%d event26\r\n", taskId);
		return TASK0_EVENT26;
	}
	if(events & TASK0_EVENT27)
	{
		LL_LOG_I("task%d event27\r\n", taskId);
		return TASK0_EVENT27;
	}
	if(events & TASK0_EVENT28)
	{
		LL_LOG_I("task%d event28\r\n", taskId);
		return TASK0_EVENT28;
	}
	if(events & TASK0_EVENT29)
	{
		LL_LOG_I("task%d event29\r\n", taskId);
		return TASK0_EVENT29;
	}
	if(events & TASK0_EVENT30)
	{
		LL_LOG_I("task%d event30\r\n", taskId);
		return TASK0_EVENT30;
	}
	
	return 0xFFFF;
}

void Task0_Init(void)
{
    task0Id = LLOS_Register_Events(Task0_Events);
    if(task0Id == LL_ERR_INVALID)
    {
    	LL_LOG_E("%s ", "init failed!\r\n", __FUNCTION__);
		while(1);
    }
	
	LLOS_Start_Event(task0Id, LL_EVENT_ALL, LLOS_Ms_To_Tick(0));
}
