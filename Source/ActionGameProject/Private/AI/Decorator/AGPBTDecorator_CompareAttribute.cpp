// KJY All Rights Reserved


#include "AI/Decorator/AGPBTDecorator_CompareAttribute.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Log/AGPLogChannels.h"

UAGPBTDecorator_CompareAttribute::UAGPBTDecorator_CompareAttribute()
{
	NodeName = "Compare Attribute";

	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
}

void UAGPBTDecorator_CompareAttribute::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/* Abort 모드가 None이 아닐때만 콜백 대기 */
	if (FlowAbortMode == EBTFlowAbortMode::None)
	{
		return;
	}

	OwnerBehaviorTreeComponent = &OwnerComp;

	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);

	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIController->GetPawn());
	checkf(PawnASC, TEXT("[%s], Invalid Owner Pawn ASC"), __FUNCTIONW__);

	PawnASC->GetGameplayAttributeValueChangeDelegate(CurrentAttribute).AddUObject(this, &UAGPBTDecorator_CompareAttribute::OnChangeCurrentAttribute);
	if (EAGPCheckAttributeValue::Rate == CheckOperation)
	{
		PawnASC->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UAGPBTDecorator_CompareAttribute::OnChangeMaxAttribute);
	}
}

void UAGPBTDecorator_CompareAttribute::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	/* Abort 모드가 None이 아닐때만 콜백 대기 */
	if (FlowAbortMode == EBTFlowAbortMode::None)
	{
		return;
	}

	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);
	
	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIController->GetPawn());
	if (PawnASC)
	{
		PawnASC->GetGameplayAttributeValueChangeDelegate(CurrentAttribute).RemoveAll(this);
		if (EAGPCheckAttributeValue::Rate == CheckOperation)
		{
			PawnASC->GetGameplayAttributeValueChangeDelegate(MaxAttribute).RemoveAll(this);
		}
	}

	OwnerBehaviorTreeComponent = nullptr;
}

bool UAGPBTDecorator_CompareAttribute::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bFoundCurrentAttribute = false;
	const float CurrentAttributeValue = GetAttributeValue(&OwnerComp, CurrentAttribute, bFoundCurrentAttribute);
	if (!bFoundCurrentAttribute)
	{
		return false;
	}

	float CheckValue = 0.f;
	switch (CheckOperation)
	{
	case EAGPCheckAttributeValue::Rate:
		{
			bool bFoundMaxAttribute = false;
			const float MaxAttributeValue = GetAttributeValue(&OwnerComp, MaxAttribute, bFoundMaxAttribute);
			if (!bFoundMaxAttribute)
			{
				return false;
			}

			CheckValue = MaxAttributeValue > 0.f ? FMath::Max(0.f, CurrentAttributeValue / MaxAttributeValue) : 0.f;
		}
		break;

	case EAGPCheckAttributeValue::Value:
		{
			CheckValue = CurrentAttributeValue;
		}
		break;
	}

	return CompareValue(CheckValue);
}

FString UAGPBTDecorator_CompareAttribute::GetStaticDescription() const
{
	FString ReturnDescription = Super::GetStaticDescription();

	ReturnDescription += TEXT("\n");
	switch (CheckOperation)
	{
	case EAGPCheckAttributeValue::Rate:
		{
			ReturnDescription += FString::Printf(TEXT("%s / %s"), *CurrentAttribute.GetName(), *MaxAttribute.GetName());
		}
		break;

	case EAGPCheckAttributeValue::Value:
		{
			ReturnDescription += *CurrentAttribute.GetName();
		}
		break;
	}

	switch (CompareOperation)
	{
	case EArithmeticKeyOperation::Equal:
		ReturnDescription += TEXT(" == ");
		break;

	case EArithmeticKeyOperation::NotEqual:
		ReturnDescription += TEXT(" != ");
		break;

	case EArithmeticKeyOperation::Less:
		ReturnDescription += TEXT(" < ");
		break;

	case EArithmeticKeyOperation::LessOrEqual:
		ReturnDescription += TEXT(" <= ");
		break;

	case EArithmeticKeyOperation::Greater:
		ReturnDescription += TEXT(" > ");
		break;

	case EArithmeticKeyOperation::GreaterOrEqual:
		ReturnDescription += TEXT(" >= ");
		break;

	default:
		ReturnDescription += TEXT("\nCompare Invalid\nValueThreshold = ");
		break;
	}

	ReturnDescription += FString::Printf(TEXT("%.2f"), ValueThreshold);

	return ReturnDescription;
}

float UAGPBTDecorator_CompareAttribute::GetAttributeValue(UBehaviorTreeComponent* OwnerComp, const FGameplayAttribute& FindAttribute, bool& bIsFoundValue) const
{
	if (!OwnerComp)
	{
		bIsFoundValue = false;
		return 0.f;
	}

	AAIController* OwnerAIController = OwnerComp->GetAIOwner();
	if (!OwnerAIController)
	{
		bIsFoundValue = false;
		return 0.f;
	}

	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIController->GetPawn());
	if (!PawnASC)
	{
		bIsFoundValue = false;
		return 0.f;
	}
	
	return PawnASC->GetGameplayAttributeValue(FindAttribute, bIsFoundValue);
}

bool UAGPBTDecorator_CompareAttribute::CompareValue(const float InCheckValue) const
{
	bool CompareResult = false;

	switch (CompareOperation)
	{
	case EArithmeticKeyOperation::Equal:
		CompareResult = (InCheckValue == ValueThreshold);
		break;

	case EArithmeticKeyOperation::NotEqual:
		CompareResult = (InCheckValue != ValueThreshold);
		break;

	case EArithmeticKeyOperation::Less:
		CompareResult = InCheckValue < ValueThreshold;
		break;

	case EArithmeticKeyOperation::LessOrEqual:
		CompareResult = InCheckValue <= ValueThreshold;
		break;

	case EArithmeticKeyOperation::Greater:
		CompareResult = InCheckValue > ValueThreshold;
		break;

	case EArithmeticKeyOperation::GreaterOrEqual:
		CompareResult = InCheckValue >= ValueThreshold;
		break;

	default:
		AGP_LOG(LogAGPAI, Warning, TEXT("Compare Operation Is Invalid"));
		break;
	}

	return CompareResult;
}

void UAGPBTDecorator_CompareAttribute::OnChangeCurrentAttribute(const FOnAttributeChangeData& Data)
{
	if (!OwnerBehaviorTreeComponent)
	{
		return;
	}

	const float CurrentAttributeValue = Data.NewValue;

	float CheckValue = 0.f;
	switch (CheckOperation)
	{
	case EAGPCheckAttributeValue::Rate:
		{
			bool bFoundMaxAttribute = false;
			const float MaxAttributeValue = GetAttributeValue(OwnerBehaviorTreeComponent, MaxAttribute, bFoundMaxAttribute);
			if (!bFoundMaxAttribute)
			{
				return;
			}

			CheckValue = MaxAttributeValue > 0.f ? FMath::Max(0.f, CurrentAttributeValue / MaxAttributeValue) : 0.f;
		}
		break;

	case EAGPCheckAttributeValue::Value:
		{
			CheckValue = CurrentAttributeValue;
		}
		break;
	}

	OnChangeAttributeEvaluateAbort(CheckValue);
}

void UAGPBTDecorator_CompareAttribute::OnChangeMaxAttribute(const FOnAttributeChangeData& Data)
{
	if (!OwnerBehaviorTreeComponent)
	{
		return;
	}

	const float MaxAttributeValue = Data.NewValue;

	bool bFoundCurrentAttribute = false;
	const float CurrentAttributeValue = GetAttributeValue(OwnerBehaviorTreeComponent, CurrentAttribute, bFoundCurrentAttribute);
	if (!bFoundCurrentAttribute)
	{
		return;
	}

	const float CheckValue = MaxAttributeValue > 0.f ? FMath::Max(0.f, CurrentAttributeValue / MaxAttributeValue) : 0.f;

	OnChangeAttributeEvaluateAbort(CheckValue);
}

void UAGPBTDecorator_CompareAttribute::OnChangeAttributeEvaluateAbort(const float CheckValue)
{
	const bool CheckResult = CompareValue(CheckValue);
	const bool bIsOnActiveBranch = OwnerBehaviorTreeComponent->IsExecutingBranch(GetMyNode(), GetChildIndex());
	const bool bIsInversed = IsInversed();

	if (bIsOnActiveBranch)
	{
		/* 현재 노드가 활성화되어있을때 */
		if ((FlowAbortMode == EBTFlowAbortMode::Self || FlowAbortMode == EBTFlowAbortMode::Both) && CheckResult == bIsInversed)
		{
			/* 조건이 만족하지 않으면 => Self 또는 Both이면 자신의 노드 실행 중지 */
			OwnerBehaviorTreeComponent->RequestBranchDeactivation(*this);
		}
	}
	else
	{
		/* 현재 노드가 활성화되어있지 않을때 */
		if ((FlowAbortMode == EBTFlowAbortMode::LowerPriority || FlowAbortMode == EBTFlowAbortMode::Both) && CheckResult != bIsInversed)
		{
			/* 조건이 만족하면 => LowerPriority 또는 Both이면 활성화와 동시에 하위 노드들 실행 중지 */
			OwnerBehaviorTreeComponent->RequestBranchActivation(*this, false);
		}
	}
}

