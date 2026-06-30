// KJY All Rights Reserved


#include "AI/Decorator/AGPBTDecorator_isActorPerception.h"
#include "AI/AGP_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Log/AGPLogChannels.h"

UAGPBTDecorator_isActorPerception::UAGPBTDecorator_isActorPerception()
{
	NodeName = "Check Target Perception";

	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	INIT_DECORATOR_NODE_NOTIFY_FLAGS();

	/* Actor만 지정 가능 */
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAGPBTDecorator_isActorPerception, BlackboardKey), AActor::StaticClass());
}

void UAGPBTDecorator_isActorPerception::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/* Abort 모드가 None이 아닐때만 콜백 대기 */
	if (FlowAbortMode == EBTFlowAbortMode::None)
	{
		return;
	}

	//AGP_LOG(LogAGPAI, Log, TEXT("NPC[%s]"), *GetNameSafe(OwnerComp.GetOwner()));

	OwnerBehaviorTreeComponent = &OwnerComp;

	AAGP_AIController* OwnerAIController = Cast<AAGP_AIController>(OwnerComp.GetAIOwner());
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);

	UAIPerceptionComponent* OwnerAIPerception = OwnerAIController->GetAGPPerceptionComponent();
	checkf(OwnerAIPerception, TEXT("[%s], Invalid OwnerAIPerception"), __FUNCTIONW__);

	OwnerAIPerception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &UAGPBTDecorator_isActorPerception::OnPerceptionUpdated);
	OwnerAIPerception->OnTargetPerceptionForgotten.AddUniqueDynamic(this, &UAGPBTDecorator_isActorPerception::OnPerceptionForgotten);

	/* BB에 지정된 Actor값이 변경될때 호출되는 콜백함수 등록 */
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		const FBlackboard::FKey KeyID = BlackboardComp->GetKeyID(BlackboardKey.SelectedKeyName);
		if (KeyID != FBlackboard::InvalidKey)
		{
			BlackboardComp->RegisterObserver(KeyID, this, FOnBlackboardChangeNotification::CreateUObject(this, &UAGPBTDecorator_isActorPerception::OnBBObserverActorChange));
		}
	}
}

void UAGPBTDecorator_isActorPerception::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/* Abort 모드가 None이 아닐때만 콜백 대기 */
	if (FlowAbortMode == EBTFlowAbortMode::None)
	{
		return;
	}

	//AGP_LOG(LogAGPAI, Log, TEXT("NPC[%s]"), *GetNameSafe(OwnerComp.GetOwner()));

	/* BB에 지정된 Actor값이 변경될때 호출되는 콜백함수 등록 해제 */
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->UnregisterObserversFrom(this);
	}
	
	AAGP_AIController* OwnerAIController = Cast<AAGP_AIController>(OwnerComp.GetAIOwner());
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);

	UAIPerceptionComponent* OwnerAIPerception = OwnerAIController->GetAGPPerceptionComponent();
	checkf(OwnerAIPerception, TEXT("[%s], Invalid OwnerAIPerception"), __FUNCTIONW__);

	OwnerAIPerception->OnTargetPerceptionUpdated.RemoveAll(this);
	OwnerAIPerception->OnTargetPerceptionForgotten.RemoveAll(this);

	OwnerBehaviorTreeComponent = nullptr;
}

bool UAGPBTDecorator_isActorPerception::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAGP_AIController* OwnerAIController = Cast<AAGP_AIController>(OwnerComp.GetAIOwner());
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);

	UBlackboardComponent* OwnerBlackboardComponent = OwnerComp.GetBlackboardComponent();
	checkf(OwnerBlackboardComponent, TEXT("[%s], Invalid OwnerBlackboardComponent"), __FUNCTIONW__);

	AActor* CheckActor = GetObserverActorFromBB(OwnerBlackboardComponent);
	return CheckActor ? OwnerAIController->IsPerceptionActor(CheckActor) : false;

	/*if (CheckActor)
	{
		const bool Result = OwnerAIController->IsPerceptionActor(CheckActor);
		AGP_LOG(LogAGPAI, Log, TEXT("CheckActor[%s] Is Perception Result[%s]"), *CheckActor->GetActorNameOrLabel(), Result ? TEXT("TRUE") : TEXT("FALSE"));
		return Result;
	}
	else
	{
		AGP_LOG(LogAGPAI, Log, TEXT("CheckActor Invalid"));
		return false;
	}*/
}

FString UAGPBTDecorator_isActorPerception::GetStaticDescription() const
{
	const FString KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	return FString::Printf(TEXT("%s: %s"), *Super::GetStaticDescription(), *KeyDesc);
}

void UAGPBTDecorator_isActorPerception::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	//AGP_LOG(LogAGPAI, Log, TEXT("Actor[%s], Stimulus[%s]"), *GetNameSafe(Actor), *Stimulus.GetDebugDescription());

	if (!OwnerBehaviorTreeComponent)
	{
		return;
	}

	AActor* ObserverActor = GetObserverActorFromBB(OwnerBehaviorTreeComponent->GetBlackboardComponent());
	if (!ObserverActor)
	{
		return;
	}

	if (ObserverActor != Actor)
	{
		return;
	}

	/* 현재 BB에 저장된 감시대상 Actor가 감지된 경우 */
	const bool bIsOnActiveBranch = OwnerBehaviorTreeComponent->IsExecutingBranch(GetMyNode(), GetChildIndex());
	const bool bIsInversed = IsInversed();

	if (bIsOnActiveBranch)
	{
		/* 현재 노드가 활성화되어있을때 */
		if ((FlowAbortMode == EBTFlowAbortMode::Self || FlowAbortMode == EBTFlowAbortMode::Both) && bIsInversed)
		{
			//AGP_LOG(LogAGPAI, Log, TEXT("RequestBranchDeactivation"));

			/* Inversed인 경우, 즉 대상이 감지되지 않을때만 실행해야함 => Self 또는 Both이면 자신의 노드 실행 중지 */
			OwnerBehaviorTreeComponent->RequestBranchDeactivation(*this);
		}
	}
	else
	{
		/* 현재 노드가 활성화되어있지 않을때 */
		if ((FlowAbortMode == EBTFlowAbortMode::LowerPriority || FlowAbortMode == EBTFlowAbortMode::Both) && !bIsInversed)
		{
			//AGP_LOG(LogAGPAI, Log, TEXT("RequestBranchActivation"));

			/* Inversed가 아닐때, 즉 대상이 감지되었을때 실행해야함 => LowerPriority 또는 Both이면 활성화와 동시에 하위 노드들 실행 중지 */
			OwnerBehaviorTreeComponent->RequestBranchActivation(*this, false);
		}
	}
}

void UAGPBTDecorator_isActorPerception::OnPerceptionForgotten(AActor* Actor)
{
	if (!OwnerBehaviorTreeComponent)
	{
		return;
	}

	//AGP_LOG(LogAGPAI, Log, TEXT("Actor[%s]"), *GetNameSafe(Actor));

	AActor* ObserverActor = GetObserverActorFromBB(OwnerBehaviorTreeComponent->GetBlackboardComponent());
	if (!ObserverActor)
	{
		return;
	}

	if (ObserverActor != Actor)
	{
		return;
	}

	/* 현재 BB에 저장된 감시대상 Actor가 감지된 대상에서 지워진 경우 */
	const bool bIsOnActiveBranch = OwnerBehaviorTreeComponent->IsExecutingBranch(GetMyNode(), GetChildIndex());
	const bool bIsInversed = IsInversed();

	if (bIsOnActiveBranch)
	{
		/* 현재 노드가 활성화되어있을때 */
		if ((FlowAbortMode == EBTFlowAbortMode::Self || FlowAbortMode == EBTFlowAbortMode::Both) && !bIsInversed)
		{
			//AGP_LOG(LogAGPAI, Log, TEXT("RequestBranchDeactivation"));

			/* Inversed가 아닌 경우, 즉 대상이 더이상 감지되지 않으면 실행중지 => Self 또는 Both이면 자신의 노드 실행 중지 */
			OwnerBehaviorTreeComponent->RequestBranchDeactivation(*this);
		}
	}
	else
	{
		/* 현재 노드가 활성화되어있지 않을때 */
		if ((FlowAbortMode == EBTFlowAbortMode::LowerPriority || FlowAbortMode == EBTFlowAbortMode::Both) && bIsInversed)
		{
			//AGP_LOG(LogAGPAI, Log, TEXT("RequestBranchActivation"));

			/* Inversed인 경우, 즉 대상이 더이상 감지되지 않았을때 실행해야 함 => LowerPriority 또는 Both이면 활성화와 동시에 하위 노드들 실행 중지 */
			OwnerBehaviorTreeComponent->RequestBranchActivation(*this, false);
		}
	}
}

EBlackboardNotificationResult UAGPBTDecorator_isActorPerception::OnBBObserverActorChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	//AGP_LOG(LogAGPAI, Log, TEXT("Call"));

	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	if (BehaviorComp)
	{
		/* BB에 저장된 Actor가 변경되면 감지 여부를 확인하고 노드를 활성화하거나 취소한다. */
		const bool bIsOnActiveBranch = BehaviorComp->IsExecutingBranch(GetMyNode(), GetChildIndex());
		const bool bIsInversed = IsInversed();
	
		if (bIsOnActiveBranch)
		{
			/* 현재 노드가 활성화되어 있을 때*/
			if ((FlowAbortMode == EBTFlowAbortMode::Self || FlowAbortMode == EBTFlowAbortMode::Both) && CalculateRawConditionValue(*BehaviorComp, nullptr) == bIsInversed)
			{
				//AGP_LOG(LogAGPAI, Log, TEXT("RequestBranchDeactivation"));

				/* 해당 Actor에 대한 감지 결과가 Inverse와 동일(ex : 감지되지 않음 == Inverse(False) / 감지됨 == Inverse(true)) => Self, Both일때 자신의 노드 실행 중지 */
				OwnerBehaviorTreeComponent->RequestBranchDeactivation(*this);
			}
		}
		else
		{
			/* 현재 노드가 활성화되어 있지 않을 때*/
			if ((FlowAbortMode == EBTFlowAbortMode::LowerPriority || FlowAbortMode == EBTFlowAbortMode::Both) && CalculateRawConditionValue(*BehaviorComp, nullptr) != bIsInversed)
			{
				//AGP_LOG(LogAGPAI, Log, TEXT("RequestBranchActivation"));

				/* 해당 Actor에 대한 감지 결과가 Inverse와 다름(ex : 감지되지 않음 != Inverse(True) / 감지됨 != Inverse(false)) => LowerPriority, Both일때 활성화와 동시에 하위 노드들 실행 중지 */
				OwnerBehaviorTreeComponent->RequestBranchActivation(*this, false);
			}
		}
	}

	return BehaviorComp ? EBlackboardNotificationResult::ContinueObserving : EBlackboardNotificationResult::RemoveObserver;
}

AActor* UAGPBTDecorator_isActorPerception::GetObserverActorFromBB(const UBlackboardComponent* InBlackboardComponent) const
{
	if (!InBlackboardComponent)
	{
		return nullptr;
	}

	return Cast<AActor>(InBlackboardComponent->GetValueAsObject(BlackboardKey.SelectedKeyName));
}
