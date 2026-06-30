// KJY All Rights Reserved


#include "AI/BTTask/AGPBTTask_ActivateAbilityEndWait.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "Log/AGPLogChannels.h"

UAGPBTTask_ActivateAbilityEndWait::UAGPBTTask_ActivateAbilityEndWait()
{
	NodeName = TEXT("ActivateAbilityEndWait");

	/* Tick 수신하지 않음 */
	bNotifyTick = false;

	/* Task 종료 수신받음 */
	bNotifyTaskFinished = true;

	/* Node Instance 생성 */
	bCreateNodeInstance = true;

	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UAGPBTTask_ActivateAbilityEndWait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
#pragma region[Init]
	UAbilitySystemComponent* OwnerASC = GetOwnerASC(OwnerComp);
	if (!OwnerASC)
	{
		return EBTNodeResult::Failed;
	}

	if (!ActivateAbilityTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}
#pragma endregion

	/* 태그가 설정된 Ability 가져옴 */
	TArray<FGameplayAbilitySpec*> AbilitiesToActivatePtrs;
	OwnerASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(ActivateAbilityTag), 
																	AbilitiesToActivatePtrs);

	if (AbilitiesToActivatePtrs.Num() < 1)
	{
		return EBTNodeResult::Failed;
	}
#pragma region [Select Ability Spec]
	const int32 SelectAbilitySpecIndex = FMath::RandRange(0, AbilitiesToActivatePtrs.Num() - 1);
	FGameplayAbilitySpec* SelectAbilitySpecPtr = AbilitiesToActivatePtrs[SelectAbilitySpecIndex];
	if (!SelectAbilitySpecPtr)
	{
		return EBTNodeResult::Failed;
	}
#pragma endregion

	if (bWaitingAbilityEnd)
	{
		OwnerBehaviorTreeComponent = &OwnerComp;

		/* Ability 종료 대기 델리게이트에 바인딩 */
		OwnerASC->OnAbilityEnded.AddUObject(this, &UAGPBTTask_ActivateAbilityEndWait::OnAbilityEnded);
	}

	/* 활성화 성공하면 InProgress / 실패하면 Fail 반환 */
	if (OwnerASC->TryActivateAbility(SelectAbilitySpecPtr->Handle))
	{
		if (bWaitingAbilityEnd)
		{
			ActivateAbilityHandle = SelectAbilitySpecPtr->Handle;
			return EBTNodeResult::Type::InProgress;
		}
		else
		{
			/* 곧바로 성공 반환 */
			return EBTNodeResult::Type::Succeeded;
		}
	}
	else
	{
		return EBTNodeResult::Type::Failed;
	}
}

FString UAGPBTTask_ActivateAbilityEndWait::GetStaticDescription() const
{
	FString ReturnDescription = Super::GetStaticDescription();

	ReturnDescription += TEXT(" : Select Ability\n");
	ReturnDescription += FString::Printf(TEXT("\nAbility[%s]"), *ActivateAbilityTag.ToString());

	return ReturnDescription;
}

void UAGPBTTask_ActivateAbilityEndWait::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	//AGP_NET_LOG(this, LogAGPAI, Log, TEXT("TaskResult[%s]"), *UEnum::GetDisplayValueAsText(TaskResult).ToString());

	if (bWaitingAbilityEnd)
	{
		/* 등록되어있던 델리게이트 바인딩 해제 */
		UAbilitySystemComponent* OwnerASC = GetOwnerASC(OwnerComp);
		if (OwnerASC)
		{
			OwnerASC->OnAbilityEnded.RemoveAll(this);
		}

		OwnerBehaviorTreeComponent = nullptr;
	}
}

void UAGPBTTask_ActivateAbilityEndWait::OnAbilityEnded(const FAbilityEndedData& InAbilityEndedData)
{
	/* Task에서 활성화했던 Ability에 대해서만 Task 종료 처리 */
	if (InAbilityEndedData.AbilitySpecHandle != ActivateAbilityHandle)
	{
		return;
	}

	/* 종료된 Ability가 Task에서 활성화한 Ability면 Task를 종료한다. */
	if (OwnerBehaviorTreeComponent)
	{
		/* Ability가 Cancel되었으면 Failed를, 그렇지 않으면 Success를 반환한다. */
		FinishLatentTask(*OwnerBehaviorTreeComponent, InAbilityEndedData.bWasCancelled ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}	
}

UAbilitySystemComponent* UAGPBTTask_ActivateAbilityEndWait::GetOwnerASC(UBehaviorTreeComponent& InOwnerBehaviorComponent) const
{
	AAIController* OwnerAIController = InOwnerBehaviorComponent.GetAIOwner();
	if (OwnerAIController)
	{
		return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIController->GetPawn());
	}

	return nullptr;
}
