// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AGPBTService_GetNearestTarget.generated.h"

/**
 * 감지된 대상들 중 가장 가까운 대상 확인
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTService_GetNearestTarget : public UBTService
{
	GENERATED_BODY()
	
private:
	/* 가장 가까운 대상을 찾았을때, 저장할 블랙보드 키 */
	UPROPERTY(EditAnywhere, Category = "Target")
	FBlackboardKeySelector InSetTargetActorKey;

public:
	UAGPBTService_GetNearestTarget();

private:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
