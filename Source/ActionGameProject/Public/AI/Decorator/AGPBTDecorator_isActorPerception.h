// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "AGPBTDecorator_isActorPerception.generated.h"

/**
 * 대상이 현재 감지중인 상태인지 확인
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTDecorator_isActorPerception : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
	
private:
	/* 부모의 BehaviorTreeComponent, PerceptionComponent감지 이벤트 콜백함수에서 사용하기 위해 캐싱 */
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> OwnerBehaviorTreeComponent;
	
public:
	UAGPBTDecorator_isActorPerception();
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	virtual FString GetStaticDescription() const override;

private:
	/* Owner PerceptionComponent를 통해 특정 객체가 인식되었을때 호출 */
	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/* Owner PerceptionComponent를 통해 인식된 객체가 잊혀졌을때 호출 */
	UFUNCTION()
	virtual void OnPerceptionForgotten(AActor* Actor);

	/* 감시중인 BB Actor가 변경되었을때 호출 */
	EBlackboardNotificationResult OnBBObserverActorChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

private:
	/* 현재 감지 여부 확인중인 Actor 반환(From BB) */
	AActor* GetObserverActorFromBB(const UBlackboardComponent* InBlackboardComponent) const;
};
