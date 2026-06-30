// KJY All Rights Reserved


#include "Costume/AGPCostumeActorBase.h"
#include "Costume/AttachCostumeBase.h"
#include "Characters/Components/AGPCharPresentationComponent.h"

AAGPCostumeActorBase::AAGPCostumeActorBase()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;
}

void AAGPCostumeActorBase::SetCostumeActorName(const FName& InName)
{
	CostumeName = InName;
}

void AAGPCostumeActorBase::SetCostumeType(const EAGPCostumeType InCostumeType)
{
	CostumeType = InCostumeType;
}

void AAGPCostumeActorBase::SetCostumeEquipmentSlot(const FGameplayTag& InCostumeEquipmentSlotTag)
{
	SoruceEquipmentSlotTag = InCostumeEquipmentSlotTag;
}

void AAGPCostumeActorBase::AttachToOwner(UAGPCharPresentationComponent* InCharacterPresentationComponent)
{
	verifyf(InCharacterPresentationComponent, TEXT("[%s], Invalid InCharacterPresentationComponent"), __FUNCTIONW__);

	OwnerPresentationComponent = InCharacterPresentationComponent;

	/* Owner에 Attach되었을때에 대한 블루프린트 이벤트 호출 */
	OnAttachedToOwner();
}

void AAGPCostumeActorBase::SetCurrentStatus(const FName& InNewStatus)
{
	FName BeforeStatus = CurrentStatus;
	CurrentStatus = InNewStatus;

	OnChangeCurrentStatus(BeforeStatus);
}



FTransform AAGPCostumeActorBase::GetCostumeActorSocketTransform_Implementation(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	return GetActorTransform();
}

const FName& AAGPCostumeActorBase::QueryConcreteSocketName(const FName& InAbstractSocketName) const
{
	if (UAGPCharPresentationComponent* CharacterPresentationComponent = OwnerPresentationComponent.Get())
	{
		return CharacterPresentationComponent->GetSpecificSocketName(InAbstractSocketName);
	}
	else
	{
		static const FName EmptySocketName;

		return EmptySocketName;
	}
}

bool AAGPCostumeActorBase::QueryOwnerStateTag(const FGameplayTag& InCheckTag) const
{
	if (UAGPCharPresentationComponent* CharacterPresentationComponent = OwnerPresentationComponent.Get())
	{
		return CharacterPresentationComponent->HasOwnedGameplayTag(InCheckTag);
	}
	else
	{
		return false;
	}
}

USceneComponent* AAGPCostumeActorBase::GetOwnerSkeletalMeshComponent() const
{
	if (UAGPCharPresentationComponent* CharacterPresentationComponent = OwnerPresentationComponent.Get())
	{
		return CharacterPresentationComponent->GetOwenrSkeletalMeshComponent();
	}
	else
	{
		return nullptr;
	}
}