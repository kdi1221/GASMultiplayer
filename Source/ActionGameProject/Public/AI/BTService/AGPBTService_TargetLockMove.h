// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "AGPBTService_TargetLockMove.generated.h"

class AAGP_CharacterBase;

/**
 *  AI 타겟잠금이동 설정/갱신/해제
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTService_TargetLockMove : public UBTService
{
	GENERATED_BODY()
	
private:
	/* 타겟잠금 대상 Actor Key */
	UPROPERTY(EditAnywhere, Category = "Target")
	FBlackboardKeySelector InTargetActorKey;

private:
	UPROPERTY()
	TObjectPtr<AAGP_CharacterBase> OwningCharacter;
	
public:
	UAGPBTService_TargetLockMove();

private:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual FString GetStaticDescription() const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void UpdateTargetReferenceLocation(UBehaviorTreeComponent& OwnerComp);
	void StopOwningCharacterTargetLockMove();
};
