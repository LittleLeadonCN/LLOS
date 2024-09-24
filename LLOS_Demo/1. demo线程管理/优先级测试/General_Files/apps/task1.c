#include <llos.h>
#include "task1.h"
#include "task0.h"

ll_taskId_t task1Id = LL_ERR_INVALID;

ll_taskEvent_t Task1_Events(ll_taskId_t taskId, ll_taskEvent_t events)
{
	if(events & LL_EVENT_MSG)
	{
		LOG_I("task%d receive message: %s\r\n", taskId, (char *)LLOS_Msg_Receive());
		LLOS_Msg_Clear();
		return LL_EVENT_MSG;
	}
	if(events & TASK1_EVENT1)
	{
		LLOS_Msg_Send(task0Id, (uint8_t *)"from task1 -> task0 are you OK?"); /* 向task0发送消息 */
		return TASK1_EVENT1;
	}
	if(events & TASK1_EVENT2)
	{
		LLOS_Stop_Event(task0Id, TASK0_EVENT2); /* 停止TASK1_EVENT2 */
		return TASK1_EVENT1;
	}
	
	return 0xFFFF;
}

void Task1_Init(void)
{
    task1Id = LLOS_Register_Events(Task1_Events);
    if(task1Id == LL_ERR_INVALID)
    {
    	LOG_I("Task1 init failed!\r\n");
		while(1);
    }
	
	LLOS_Start_Event(task1Id, TASK1_EVENT1, LLOS_Ms_To_Tick(5000)); /* 5s后向task0发送消息 */
	LLOS_Start_Event(task1Id, TASK1_EVENT2, LLOS_Ms_To_Tick(10000)); /* 10s后停止TASK1_EVENT2 */
}
