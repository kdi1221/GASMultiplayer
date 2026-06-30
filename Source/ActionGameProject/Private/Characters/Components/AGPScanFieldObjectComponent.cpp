// KJY All Rights Reserved


#include "Characters/Components/AGPScanFieldObjectComponent.h"
#include "Net/UnrealNetwork.h"
#include "FieldObject/AGPFieldObjectBase.h"
#include "FieldObject/AGPFieldStone.h"
#include "FieldObject/AGPInteractableObjectBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Log/AGPLogChannels.h"

UAGPScanFieldObjectComponent::UAGPScanFieldObjectComponent()
{
	bWantsInitializeComponent = false;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UAGPScanFieldObjectComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UAGPScanFieldObjectComponent, CurrentInteractionMode, COND_OwnerOnly);
}

void UAGPScanFieldObjectComponent::OnBeginOverlapFieldObject(AAGPFieldObjectBase* InFieldObject)
{
	if (!InFieldObject)
	{
		return;
	}

	if (InFieldObject->IsA<AAGPFieldStone>())
	{
		CollectFieldStoneObject(Cast<AAGPFieldStone>(InFieldObject));
	}
	else if (InFieldObject->IsA<AAGPInteractableObjectBase>())
	{
		OnBeginOverlapInteractableObject(Cast<AAGPInteractableObjectBase>(InFieldObject));
	}

	UpdateCurrentInteractionMode();

	//AGP_NET_LOG(this, LogAGPFieldObject, Log, TEXT("OtherActor : [%s]"), *InFieldObject->GetActorNameOrLabel());
}

void UAGPScanFieldObjectComponent::OnEndOverlapFieldObject(AAGPFieldObjectBase* InFieldObject)
{
	if (!InFieldObject)
	{
		return;
	}

	if (InFieldObject->IsA<AAGPFieldStone>())
	{
		DiscardFieldStoneObject(Cast<AAGPFieldStone>(InFieldObject));
	}
	else if (InFieldObject->IsA<AAGPInteractableObjectBase>())
	{
		OnEndOverlapInteractableObject(Cast<AAGPInteractableObjectBase>(InFieldObject));
	}

	UpdateCurrentInteractionMode();

	//AGP_NET_LOG(this, LogAGPFieldObject, Log, TEXT("OtherActor : [%s]"), *InFieldObject->GetActorNameOrLabel());
}

void UAGPScanFieldObjectComponent::ConsumeCurrentScanStones()
{
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	checkf(OwnerASC, TEXT("[%s], Invalid OwnerASC, Owner[%s]"), __FUNCTIONW__, GetOwner() ? *GetOwner()->GetActorNameOrLabel() : TEXT("Invalid"));

	/* Consume 과정에서 Destroy되기때문에 TSet => TArray로 옮겨서 실행 */
	TArray<TWeakObjectPtr<AAGPFieldStone>> ConsumeStoneObjects = ScanFieldStones.Array();
	ScanFieldStones.Empty();

	for (TWeakObjectPtr<AAGPFieldStone> StoneObject : ConsumeStoneObjects)
	{
		AAGPFieldStone* CurrentStoneObject = StoneObject.Get();
		if (!CurrentStoneObject)
		{
			AGP_NET_LOG(this, LogAGPFieldObject, Warning, TEXT("Invalid Current Stone Object, Owner[%s]"), GetOwner() ? *GetOwner()->GetActorNameOrLabel() : TEXT("Invalid"));
			continue;
		}

		CurrentStoneObject->ConsumeStone(OwnerASC);
	}
}

void UAGPScanFieldObjectComponent::ActivateCurrentInteractableObject()
{
	if (!ScanInteractableObject.IsValid())
	{
		return;
	}

	ScanInteractableObject->ActivateObject();
}

void UAGPScanFieldObjectComponent::CollectFieldStoneObject(AAGPFieldStone* InFieldStone)
{
	checkf(InFieldStone, TEXT("[%s], Invalid Field Stone"), __FUNCTIONW__);

	bool IsAlreadySet = false;
	ScanFieldStones.Add(InFieldStone, &IsAlreadySet);
}

void UAGPScanFieldObjectComponent::DiscardFieldStoneObject(AAGPFieldStone* InFieldStone)
{
	checkf(InFieldStone, TEXT("[%s], Invalid Field Stone"), __FUNCTIONW__);

	ScanFieldStones.Remove(InFieldStone);
}

void UAGPScanFieldObjectComponent::OnBeginOverlapInteractableObject(AAGPInteractableObjectBase* InInteractableObject)
{
	checkf(InInteractableObject, TEXT("[%s], Invalid Interactable Object"), __FUNCTIONW__);

	/* 활성화 가능상태에서만 저장 */
	if (EAGPInteractableActorState::Available == InInteractableObject->GetCurrentState())
	{
		ScanInteractableObject = InInteractableObject;
	}
}

void UAGPScanFieldObjectComponent::OnEndOverlapInteractableObject(AAGPInteractableObjectBase* InInteractableObject)
{
	checkf(InInteractableObject, TEXT("[%s], Invalid Interactable Object"), __FUNCTIONW__);

	if (ScanInteractableObject == InInteractableObject)
	{
		ScanInteractableObject.Reset();
	}
}

void UAGPScanFieldObjectComponent::UpdateCurrentInteractionMode()
{
	const EAGPInteractionMode BeforeInteractionMode = CurrentInteractionMode;
	if (ScanInteractableObject.IsValid())
	{
		CurrentInteractionMode = EAGPInteractionMode::InteractableObject;
	}
	else if (!ScanFieldStones.IsEmpty())
	{
		CurrentInteractionMode = EAGPInteractionMode::FieldStone;
	}
	else
	{
		CurrentInteractionMode = EAGPInteractionMode::None;
	}

	if (BeforeInteractionMode != CurrentInteractionMode)
	{
		NotifyInteractionModeChange();
	}
}

void UAGPScanFieldObjectComponent::NotifyInteractionModeChange() const
{
	OnChangeCurrentInteractionMode.Broadcast(CurrentInteractionMode);
}

void UAGPScanFieldObjectComponent::OnRep_CurrentInteractionMode() const
{
	NotifyInteractionModeChange();
}