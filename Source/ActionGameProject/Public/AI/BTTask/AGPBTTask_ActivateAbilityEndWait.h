// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"
#include "AGPBTTask_ActivateAbilityEndWait.generated.h"

struct FAbilityEndedData;
class UAbilitySystemComponent;
class UAGPGameplayAbility;

/**
 * AI의 Ability 사용 및 사용완료까지 대기
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTTask_ActivateAbilityEndWait : public UBTTaskNode
{
	GENERATED_BODY()
	
private:
	/* 활성화 Ability 지정 Tag */
	UPROPERTY(EditAnywhere, Category = "ActivateAbility")
	FGameplayTag ActivateAbilityTag;

	/* Ability 종료 대기 */
	UPROPERTY(EditAnywhere, Category = "ActivateAbility")
	bool bWaitingAbilityEnd = true;

private:
	/* 부모의 BehaviorTreeComponent */
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> OwnerBehaviorTreeComponent;

	/* 활성화 한 Ability Handle */
	UPROPERTY()
	FGameplayAbilitySpecHandle ActivateAbilityHandle;
	
public:
	UAGPBTTask_ActivateAbilityEndWait();

#pragma region[UBTTaskNode Interface]
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, 
											uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, 
								uint8* NodeMemory, 
								EBTNodeResult::Type TaskResult) override;
#pragma endregion

private:
	void OnAbilityEnded(const FAbilityEndedData& InAbilityEndedData);

private:
	UAbilitySystemComponent* GetOwnerASC(UBehaviorTreeComponent& InOwnerBehaviorComponent) const;
};

