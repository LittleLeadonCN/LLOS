#include "task0.h"

ll_taskId_t task0Id = LL_ERR_INVALID;

static ll_taskEvent_t Task0_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & LL_EVENT_MSG)
	{
		struct task0_testStruct *task0_test;
		task0_test = (struct task0_testStruct *)LLOS_Msg_Receive(taskId);
		LL_LOG_I("taskId: %d; a = %d   b = %d   c = %d   d = %s\r\n", taskId, task0_test->a, task0_test->b, task0_test->c, task0_test->d);
		LLOS_Msg_Clear(taskId);
		return LL_EVENT_MSG;
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
}
