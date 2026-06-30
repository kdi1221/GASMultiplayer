// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AGPBTService_OrientToTarget.generated.h"

/**
 * 타겟을 향해 회전하는 Service 노드 => 사용하지않지만 이후 추가적인 몬스터패턴구현하면서 참고할수있으니 다른몬스터구현완료될때가지 삭제 보류
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTService_OrientToTarget : public UBTService
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, Category = "Target")
	FBlackboardKeySelector InTargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Target")
	float RotationInterpSpeed;
	
public:
	UAGPBTService_OrientToTarget();
	
private:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
