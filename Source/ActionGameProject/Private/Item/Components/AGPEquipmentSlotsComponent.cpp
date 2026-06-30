// KJY All Rights Reserved


#include "Item/Components/AGPEquipmentSlotsComponent.h"
#include "Item/Slots/AGPEquipmentSlot.h"
#include "Item/Instance/AGPItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "AGPGameplayTags.h"
#include "Item/Interfaces/AGPItemSystemExternalInterface.h"
#include "Log/AGPLogChannels.h"

FAGPEquipmentSlotElement::FAGPEquipmentSlotElement()
{

}

FAGPEquipmentSlotElementsContainer::FAGPEquipmentSlotElementsContainer()
{

}

void FAGPEquipmentSlotElementsContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (UAGPEquipmentSlotsComponent* EquipmentSlotsComponent = OwnerEquipmentSlotsComponent.Get())
	{
		EquipmentSlotsComponent->OnPreReplicatedRemove(RemovedIndices, FinalSize);
	}
}

void FAGPEquipmentSlotElementsContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (UAGPEquipmentSlotsComponent* EquipmentSlotsComponent = OwnerEquipmentSlotsComponent.Get())
	{
		EquipmentSlotsComponent->OnPostReplicatedAdd(AddedIndices, FinalSize);
	}
}

void FAGPEquipmentSlotElementsContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (UAGPEquipmentSlotsComponent* EquipmentSlotsComponent = OwnerEquipmentSlotsComponent.Get())
	{
		EquipmentSlotsComponent->OnPostReplicatedChange(ChangedIndices, FinalSize);
	}
}

bool FAGPEquipmentSlotElementsContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	/* 테스트 로그 */
	/*if (DeltaParms.Connection && DeltaParms.Connection->OwningActor)
	{
		AActor* ConnectionOwningActor = DeltaParms.Connection->OwningActor;

		if(ROLE_Authority != ConnectionOwningActor->GetLocalRole())
		{
			AGP_NET_LOG(ConnectionOwningActor, LogAGPEquipment, Log, TEXT("OwningActor[%s]"), *GetNameSafe(DeltaParms.Connection->OwningActor));
		}
	}*/

	return FFastArraySerializer::FastArrayDeltaSerialize<FAGPEquipmentSlotElement, FAGPEquipmentSlotElementsContainer>(EquipmentSlotElements, DeltaParms, *this);
}

void FAGPEquipmentSlotElementsContainer::InitializeContainer(UAGPEquipmentSlotsComponent* InEquipmentComponent)
{
	OwnerEquipmentSlotsComponent = InEquipmentComponent;
}

void FAGPEquipmentSlotElementsContainer::AddEquipmentSlotElement(UAGPEquipmentSlot* InAddEquipmentSlot, const FGameplayTag& InSlotTag)
{
	verifyf(InAddEquipmentSlot, TEXT("AddEquipmentSlotElement, but Invalid InAddEquipmentSlot"));
	verifyf(InSlotTag.IsValid(), TEXT("AddEquipmentSlotElement, but Invalid InSlotTag"));

	FAGPEquipmentSlotElement& AddedEquipmentSlotElement = EquipmentSlotElements.AddDefaulted_GetRef();
	AddedEquipmentSlotElement.EquipmentSlot = InAddEquipmentSlot;
	AddedEquipmentSlotElement.SlotTag = InSlotTag;

	//장비 인스턴스 변경되었으므로 마킹
	MarkItemDirty(AddedEquipmentSlotElement);
}



UAGPEquipmentSlotsComponent::UAGPEquipmentSlotsComponent()
{
	bWantsInitializeComponent = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	//ReplicateSubobjects => AddReplicatedSubObject, 테스트 필요
	bReplicateUsingRegisteredSubObjectList = false;

	SetIsReplicatedByDefault(true);
}

#pragma region[UActorComponent Interface]
bool UAGPEquipmentSlotsComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FAGPEquipmentSlotElement& EquipmentSlotElement : EquipmentSlotsContainer.EquipmentSlotElements)
	{
		UAGPEquipmentSlot* EquipmentSlot = EquipmentSlotElement.EquipmentSlot;
		if (IsValid(EquipmentSlot))
		{
			WroteSomething |= Channel->ReplicateSubobject(EquipmentSlot, *Bunch, *RepFlags);

			UAGPItemInstance* ItemInstanceInSlot = EquipmentSlot->GetItemInstance();
			if (IsValid(ItemInstanceInSlot))
			{
				WroteSomething |= Channel->ReplicateSubobject(ItemInstanceInSlot, *Bunch, *RepFlags);
			}
		}
	}

	return WroteSomething;
}

void UAGPEquipmentSlotsComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (FAGPEquipmentSlotElement& EquipmentSlotElement : EquipmentSlotsContainer.EquipmentSlotElements)
		{
			UAGPEquipmentSlot* EquipmentSlot = EquipmentSlotElement.EquipmentSlot;
			if (IsValid(EquipmentSlot))
			{
				AddReplicatedSubObject(EquipmentSlot);

				if (UAGPItemInstance* ItemInstanceInSlot = EquipmentSlot->GetItemInstance())
				{
					AddReplicatedSubObject(ItemInstanceInSlot);
				}
			}
		}
	}
}

void UAGPEquipmentSlotsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, EquipmentSlotsContainer, COND_OwnerOnly);
}

void UAGPEquipmentSlotsComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && EWorldType::EditorPreview != CurrentWorld->WorldType)
	{
		EquipmentSlotsContainer.InitializeContainer(this);

		if (ROLE_Authority == GetOwnerRole())
		{
			InitializeEquipmentSlots();
		}
	}
}
#pragma endregion



void UAGPEquipmentSlotsComponent::OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FAGPEquipmentSlotElement& RemoveEquipmentSlot = EquipmentSlotsContainer.EquipmentSlotElements[Index];

		//AGP_NET_LOG(OwnerComponent.Get(), LogAGPEquipment, Log, TEXT("EquipmentSlot[%d] - <%s> Remove"), Index, *RemoveEquipmentSlot.GetDebugString());
	}

}

void UAGPEquipmentSlotsComponent::OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FAGPEquipmentSlotElement& AddedEquipmentSlot = EquipmentSlotsContainer.EquipmentSlotElements[Index];

		OnAddedEmptyEquipmentSlot(AddedEquipmentSlot.EquipmentSlot, AddedEquipmentSlot.SlotTag);
	}
}

void UAGPEquipmentSlotsComponent::OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FAGPEquipmentSlotElement& ChangeEquipmentSlot = EquipmentSlotsContainer.EquipmentSlotElements[Index];

		//AGP_NET_LOG(OwnerComponent.Get(), LogAGPEquipment, Log, TEXT("EquipmentSlot[%d] - <%s> Remove"), Index, *ChangeEquipmentSlot.GetDebugString());
	}
}

void UAGPEquipmentSlotsComponent::InitializeEquipmentSlots()
{
	for (const FGameplayTag& AddSlotTag : InitializeEquipmentSlotTags)
	{
		AddEquipmentSlot(AddSlotTag);
	}
}

void UAGPEquipmentSlotsComponent::AddEquipmentSlot(const FGameplayTag& InSlotTag)
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	verifyf(!IsContainEquipSlot(InSlotTag), TEXT("AddEquipmentSlot, but SlotTag[%s] is already Contain"), *InSlotTag.ToString());

	//Equipment Slot 생성
	UAGPEquipmentSlot* NewEquipmentSlot = NewObject<UAGPEquipmentSlot>(this);
	verifyf(NewEquipmentSlot, TEXT("NewEquipmentSlot Invalid.."));

	EquipmentSlotsContainer.AddEquipmentSlotElement(NewEquipmentSlot, InSlotTag);

	OnAddedEmptyEquipmentSlot(NewEquipmentSlot, InSlotTag);
}

void UAGPEquipmentSlotsComponent::OnAddedEmptyEquipmentSlot(UAGPEquipmentSlot* InAddEquipmentSlot, const FGameplayTag& InAddSlotTag)
{
	verifyf(InAddEquipmentSlot, TEXT("OnAddedEmptyEquipmentSlot, But InAddEquipmentSlot Invalid.."));
	verifyf(InAddSlotTag.IsValid(), TEXT("OnAddedEmptyInventorySlot, But InAddSlotTag Invalid.."));

	//빈 슬롯에 대해서만 여기로 들어온다고 가정함
	verifyf(InAddEquipmentSlot->IsEmpty(), TEXT("InAddEquipmentSlot, But InAddEquipmentSlot is not empty.."));

	/* 추가된 Equipment Slot 각종 초기화 작업 */
	InAddEquipmentSlot->SetSlotOwner(GetOwner());
	InAddEquipmentSlot->SetEquipmentSlotTag(InAddSlotTag);
	InAddEquipmentSlot->BindItemSlotEvent(EAGPSlotEvent::MountedInSlot, this, &UAGPEquipmentSlotsComponent::OnEquipmentSlotItemMounted);
	InAddEquipmentSlot->BindItemSlotEvent(EAGPSlotEvent::RemoveFromSlot, this, &UAGPEquipmentSlotsComponent::OnEquipmentSlotItemRemoved);

	//AGP_NET_LOG(this, LogAGPEquipment, Log, TEXT("%s"), *InAddEquipmentSlot->GetDebugString());

	MapEquipmentSlots.Add(InAddSlotTag, InAddEquipmentSlot);

	/* SubObjectList 사용하는경우 새로운 슬롯이 생성되었을때 Replicate를 위해 추가해주어야 함. */
	/* 클라이언트의 경우 리플레이시스템등에 사용될 수 있으므로 호출 */
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(InAddEquipmentSlot);
	}

	/* 슬롯 내 ItemInstance의 경우 이후 Replicate과정에서 OnRep함수에 의해 호출되는 OnInventorySlotItemMounted에서 처리 */
}

void UAGPEquipmentSlotsComponent::OnEquipmentSlotItemMounted(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance)
{
	verifyf(InItemSlot, TEXT("OnInventorySlotItemMounted, but InItemSlot Invalid"));
	verifyf(InItemInstance, TEXT("OnInventorySlotItemMounted, but InItemInstance Invalid"));

	//AGP_NET_LOG(this, LogAGPEquipment, Log, TEXT("Slot - %s"), *InItemSlot->GetDebugString());

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(InItemInstance);
	}
}

void UAGPEquipmentSlotsComponent::OnEquipmentSlotItemRemoved(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance)
{
	verifyf(InItemSlot, TEXT("OnInventorySlotItemRemoved, but InItemSlot Invalid"));
	verifyf(InItemInstance, TEXT("OnInventorySlotItemRemoved, but InItemInstance Invalid"));

	//AGP_NET_LOG(this, LogAGPEquipment, Log, TEXT("Slot<%s>, Removed Item<%s>"), *InItemSlot->GetDebugString(), *InItemInstance->GetDebugString());

	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(InItemInstance);
	}
}

UAGPEquipmentSlot* UAGPEquipmentSlotsComponent::GetEquipmentSlot(const FGameplayTag& InSlotTag) const
{
	if (!IsContainEquipSlot(InSlotTag))
	{
		return nullptr;
	}

	return MapEquipmentSlots[InSlotTag];
}

bool UAGPEquipmentSlotsComponent::IsContainEquipSlot(const FGameplayTag& InTagEquipSlot) const
{
	return MapEquipmentSlots.Contains(InTagEquipSlot);
}