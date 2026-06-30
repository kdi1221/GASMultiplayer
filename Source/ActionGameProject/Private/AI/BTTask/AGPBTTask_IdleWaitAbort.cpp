// KJY All Rights Reserved


#include "AI/BTTask/AGPBTTask_IdleWaitAbort.h"

UAGPBTTask_IdleWaitAbort::UAGPBTTask_IdleWaitAbort()
{
	NodeName = TEXT("Idle Wait Abort");

	/* Tick 수신하지 않음 */
	bNotifyTick = false;

	/* Task 종료 수신받지 않음 */
	bNotifyTaskFinished = false;

	/* Node Instance 생성 하지 않음 */
	bCreateNodeInstance = false;

	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UAGPBTTask_IdleWaitAbort::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/* InProgress를 반환해서 외부에서 종료되기 전까지 대기 */
	return EBTNodeResult::Type::InProgress;
}