// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AGPBTTask_IdleWaitAbort.generated.h"

/**
 * 외부에서 중단(Abort, 데코레이터등의 조건등에 의해)될때까지 대기하는 Task
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTTask_IdleWaitAbort : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UAGPBTTask_IdleWaitAbort();

#pragma region[UBTTaskNode Interface]
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
#pragma endregion
};
