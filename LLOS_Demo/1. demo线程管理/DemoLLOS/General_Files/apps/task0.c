#include "task0.h"
#include "task1.h"

ll_taskId_t task0Id = LL_ERR_INVALID;

ll_taskEvent_t Task0_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & LL_EVENT_MSG)
	{
		LOG_I("task%d receive message: %s\r\n", task0Id, (char *)LLOS_Msg_Receive());
		LLOS_Msg_Clear();
		LLOS_Msg_Send(task1Id, (uint8_t *)"from task0 -> task1 are you OK?"); /* 向task1发送消息 */
		return LL_EVENT_MSG;
	}
	if(events & TASK0_EVENT1)
	{
		LOG_I("task%d event1\r\n", task0Id);
	    LLOS_Start_Event(task0Id, TASK0_EVENT1, LLOS_Ms_To_Tick(500)); /* 500ms循环TASK0_EVENT1 */
		return TASK0_EVENT1;
	}
	if(events & TASK0_EVENT2)
	{
		LOG_I("task%d event2\r\n", task0Id);
	    LLOS_Start_Event(task0Id, TASK0_EVENT2, LLOS_Ms_To_Tick(1000)); /* 1000ms循环TASK0_EVENT2 */
		return TASK0_EVENT2;
	}
	if(events & TASK0_EVENT3)
	{
		LOG_I("task%d stop event1\r\n", task0Id);
	    LLOS_Stop_Event(task0Id, TASK0_EVENT1); /* 停止TASK0_EVENT1 */
		LLOS_Start_Event(task0Id, TASK0_EVENT2, LLOS_Ms_To_Tick(0)); /* 立即启动TASK0_EVENT2 */
		return TASK0_EVENT3;
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
	
	LLOS_Start_Event(task0Id, TASK0_EVENT1, LLOS_Ms_To_Tick(0)); /* 立即启动TASK0_EVENT1 */
	LLOS_Start_Event(task0Id, TASK0_EVENT3, LLOS_Ms_To_Tick(3000)); /* 3s后停止TASK0_EVENT1 */
}
