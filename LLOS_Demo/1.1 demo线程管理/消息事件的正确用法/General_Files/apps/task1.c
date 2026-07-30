#include "task1.h"
#include "task0.h"

ll_taskId_t task1Id = LL_ERR_INVALID;

static ll_taskEvent_t Task1_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & 0x0001)
	{
		struct task0_testStruct task0_test = {.a = 1, .b = 2, .c = 3, "Hello task0"};
		LLOS_Msg_Send(task0Id, &task0_test);
		return 0x0001;
	}
	
	return 0xFFFF;
}

void Task1_Init(void)
{
    task1Id = LLOS_Register_Events(Task1_Events);
    if(task1Id == LL_ERR_INVALID)
    {
    	LL_LOG_E("%s ", "init failed!\r\n", __FUNCTION__);
		while(1);
    }
	LLOS_Start_Event(task1Id, 0x0001, LLOS_Ms_To_Tick(1000));
}
