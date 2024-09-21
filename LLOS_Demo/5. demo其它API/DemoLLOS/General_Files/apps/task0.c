#include "task0.h"

ll_taskId_t task0Id = LL_ERR_INVALID;

ll_taskEvent_t Task0_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & LL_EVENT_MSG)
	{
		LOG_I("task%d receive message: %s\r\n", taskId, LLOS_Msg_Receive());
		LLOS_Msg_Clear();
		return LL_EVENT_MSG;
	}
	if(events & TASK0_EVENT1)
	{
		LOG_I("Random number: %u\r\n", (LLOS_Random()));
	    LLOS_Start_Event(taskId, TASK0_EVENT1, LLOS_Ms_To_Tick(500));
		return TASK0_EVENT1;
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
	
	LLOS_Start_Event(task0Id, TASK0_EVENT1, LLOS_Ms_To_Tick(0));
}
