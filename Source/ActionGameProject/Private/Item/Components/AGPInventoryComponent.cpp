// KJY All Rights Reserved


#include "Item/Components/AGPInventoryComponent.h"
#include "Item/Slots/AGPInventorySlot.h"
#include "Item/Instance/AGPItemInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

FAGPInventorySlotElement::FAGPInventorySlotElement()
{
}

FAGPInventorySlotElementsContainer::FAGPInventorySlotElementsContainer()
{

}

void FAGPInventorySlotElementsContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (UAGPInventoryComponent* InventoryComponent = OwnerInventoryComponent.Get())
	{
		InventoryComponent->OnPreReplicatedRemove(RemovedIndices, FinalSize);
	}
}

void FAGPInventorySlotElementsContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (UAGPInventoryComponent* InventoryComponent = OwnerInventoryComponent.Get())
	{
		InventoryComponent->OnPostReplicatedAdd(AddedIndices, FinalSize);
	}
}

void FAGPInventorySlotElementsContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (UAGPInventoryComponent* InventoryComponent = OwnerInventoryComponent.Get())
	{
		InventoryComponent->OnPostReplicatedChange(ChangedIndices, FinalSize);
	}
}

bool FAGPInventorySlotElementsContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	/* 테스트 로그 */
	/*if (DeltaParms.Connection && DeltaParms.Connection->OwningActor)
	{
		AActor* ConnectionOwningActor = DeltaParms.Connection->OwningActor;

		if (ROLE_Authority != ConnectionOwningActor->GetLocalRole())
		{
			AGP_NET_LOG(ConnectionOwningActor, LogAGPInventory, Log, TEXT("OwningActor[%s]"), *GetNameSafe(DeltaParms.Connection->OwningActor));
		}
	}*/

	return FFastArraySerializer::FastArrayDeltaSerialize<FAGPInventorySlotElement, FAGPInventorySlotElementsContainer>(InventorySlotElements, DeltaParms, *this);
}

void FAGPInventorySlotElementsContainer::InitializeContainer(UAGPInventoryComponent* InInventoryComponent)
{
	OwnerInventoryComponent = InInventoryComponent;
}

void FAGPInventorySlotElementsContainer::AddInventorySlotElement(UAGPInventorySlot* InAddInventorySlot)
{
	verifyf(InAddInventorySlot, TEXT("AddInventorySlotElement, but Invalid InAddInventorySlot"));

	FAGPInventorySlotElement& AddedInventorySlotElement = InventorySlotElements.AddDefaulted_GetRef();
	AddedInventorySlotElement.InventorySlot = InAddInventorySlot;

	//장비 인스턴스 변경되었으므로 마킹
	MarkItemDirty(AddedInventorySlotElement);
}

UAGPInventoryComponent::UAGPInventoryComponent()
{
	bWantsInitializeComponent = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	//ReplicateSubobjects => AddReplicatedSubObject, 테스트 필요
	bReplicateUsingRegisteredSubObjectList = false;

	SetIsReplicatedByDefault(true);
}

#pragma region[UActorComponent Interface]
bool UAGPInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FAGPInventorySlotElement& InventorySlotElement : InventorySlotsContainer.InventorySlotElements)
	{
		UAGPInventorySlot* InventorySlot = InventorySlotElement.InventorySlot;
		if (IsValid(InventorySlot))
		{
			WroteSomething |= Channel->ReplicateSubobject(InventorySlot, *Bunch, *RepFlags);

			UAGPItemInstance* ItemInstanceInSlot = InventorySlot->GetItemInstance();
			if (IsValid(ItemInstanceInSlot))
			{
				WroteSomething |= Channel->ReplicateSubobject(ItemInstanceInSlot, *Bunch, *RepFlags);
			}
		}
	}

	return WroteSomething;
}

void UAGPInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (FAGPInventorySlotElement& InventorySlotElement : InventorySlotsContainer.InventorySlotElements)
		{
			UAGPInventorySlot* InventorySlot = InventorySlotElement.InventorySlot;
			if (IsValid(InventorySlot))
			{
				AddReplicatedSubObject(InventorySlot);

				if (UAGPItemInstance* ItemInstanceInSlot = InventorySlot->GetItemInstance())
				{
					AddReplicatedSubObject(ItemInstanceInSlot);
				}
			}
		}
	}
}

void UAGPInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, InventorySlotsContainer, COND_OwnerOnly);
}

void UAGPInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && EWorldType::EditorPreview != CurrentWorld->WorldType)
	{
		InventorySlotsContainer.InitializeContainer(this);

		if (ROLE_Authority == GetOwnerRole())
		{
			InitializeInventorySlots();
		}
	}
}
#pragma endregion

void UAGPInventoryComponent::OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FAGPInventorySlotElement& RemoveInventorySlot = InventorySlotsContainer.InventorySlotElements[Index];

		//AGP_NET_LOG(OwnerComponent.Get(), LogAGPInventory, Log, TEXT("Inventory[%d] - <%s> Remove"), Index, *InventorySlot.GetDebugString());
	}
}

void UAGPInventoryComponent::OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FAGPInventorySlotElement& AddedInventorySlot = InventorySlotsContainer.InventorySlotElements[Index];

		OnAddedEmptyInventorySlot(AddedInventorySlot.InventorySlot, Index);
	}
}

void UAGPInventoryComponent::OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FAGPInventorySlotElement& ChangeInventorySlot = InventorySlotsContainer.InventorySlotElements[Index];

		//AGP_NET_LOG(OwnerComponent.Get(), LogAGPInventory, Log, TEXT("Inventory[%d] - <%s> Remove"), Index, *InventorySlot.GetDebugString());
	}
}

void UAGPInventoryComponent::AddEmptyInventorySlot(const int32 InSlotIndex)
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	//Inventory Slot 생성
	UAGPInventorySlot* NewInventorySlot = NewObject<UAGPInventorySlot>(this);
	verifyf(NewInventorySlot, TEXT("NewInventorySlot Invalid.."));

	InventorySlotsContainer.AddInventorySlotElement(NewInventorySlot);

	OnAddedEmptyInventorySlot(NewInventorySlot, InSlotIndex);
}

void UAGPInventoryComponent::InitializeInventorySlots()
{
	/* 지정된 초기 인벤토리 슬롯 개수에 따라 슬롯 추가 */
	for (int32 SlotNum = 0; SlotNum < InitailizeInventorySlotNum; ++SlotNum)
	{
		AddEmptyInventorySlot(SlotNum);
	}
}

void UAGPInventoryComponent::OnAddedEmptyInventorySlot(UAGPInventorySlot* InAddInventorySlot, const int32 InAddSlotIndex)
{
	verifyf(InAddInventorySlot, TEXT("OnAddedEmptyInventorySlot, But InAddInventorySlot Invalid.."));
	verifyf(InAddSlotIndex > AGPItemConstDefine::INVALID_ITEM_SLOT_INDEX, TEXT("OnAddedEmptyInventorySlot, But InAddSlotIndex[%d] Invalid.."), InAddSlotIndex);

	//빈 슬롯에 대해서만 여기로 들어온다고 가정함
	verifyf(InAddInventorySlot->IsEmpty(), TEXT("OnAddedEmptyInventorySlot, But InAddInventorySlot is not empty.."));

	//AGP_NET_LOG(this, LogAGPInventory, Log, TEXT("%s"), *InAddInventorySlot->GetDebugString());

	/* 추가된 Inventory Slot 각종 초기화 작업 */
	InAddInventorySlot->SetSlotOwner(GetOwner());
	InAddInventorySlot->SetInventorySlotIndex(InAddSlotIndex);
	InAddInventorySlot->BindItemSlotEvent(EAGPSlotEvent::MountedInSlot, this, &UAGPInventoryComponent::OnInventorySlotItemMounted);
	InAddInventorySlot->BindItemSlotEvent(EAGPSlotEvent::RemoveFromSlot, this, &UAGPInventoryComponent::OnInventorySlotItemRemoved);

	/* SubObjectList 사용하는경우 새로운 슬롯이 생성되었을때 Replicate를 위해 추가해주어야 함. */
	/* 클라이언트의 경우 리플레이시스템등에 사용될 수 있으므로 호출 */
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(InAddInventorySlot);
	}

	/* 슬롯 내 ItemInstance의 경우 이후 Replicate과정에서 OnRep함수에 의해 호출되는 OnInventorySlotItemMounted에서 처리 */
}

void UAGPInventoryComponent::OnInventorySlotItemMounted(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance)
{
	verifyf(InItemSlot, TEXT("OnInventorySlotItemMounted, but InItemSlot Invalid"));
	verifyf(InItemInstance, TEXT("OnInventorySlotItemMounted, but InItemInstance Invalid"));

	//AGP_NET_LOG(this, LogAGPInventory, Log, TEXT("Slot - %s"), *InItemSlot->GetDebugString());

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(InItemInstance);
	}
}

void UAGPInventoryComponent::OnInventorySlotItemRemoved(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance)
{
	verifyf(InItemSlot, TEXT("OnInventorySlotItemRemoved, but InItemSlot Invalid"));
	verifyf(InItemInstance, TEXT("OnInventorySlotItemRemoved, but InItemInstance Invalid"));

	//AGP_NET_LOG(this, LogAGPInventory, Log, TEXT("Slot - %s"), *InItemSlot->GetDebugString());

	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(InItemInstance);
	}
}

UAGPInventorySlot* UAGPInventoryComponent::GetInventorySlot(const int32 InSlotIndex) const
{
	if (!IsValidSlotIndex(InSlotIndex))
	{
		return nullptr;
	}

	return InventorySlotsContainer.InventorySlotElements[InSlotIndex].InventorySlot;
}

UAGPInventorySlot* UAGPInventoryComponent::GetCanPlacedInventorySlot(UAGPItemInstance* InItemInstance) const
{
	if (!InItemInstance)
	{
		return nullptr;
	}

	//TODO : 체크할 아이템의 스택여부등을 확인하여 인벤토리 내 슬롯을 먼저 찾는다.

	//인벤토리 내 빈슬롯을 찾아서 반환
	const FAGPInventorySlotElement* FindSlotElement = InventorySlotsContainer.InventorySlotElements.FindByPredicate
	(
		[](const FAGPInventorySlotElement& InSlotElement)
		{
			const UAGPInventorySlot* CheckElementSlotInstance = InSlotElement.InventorySlot;

			if (CheckElementSlotInstance && CheckElementSlotInstance->IsEmpty())
			{
				return true;
			}

			return false;
		}
	);

	return FindSlotElement->InventorySlot;
}

bool UAGPInventoryComponent::IsValidSlotIndex(const int32 InCheckSlotIndex) const
{
	return InventorySlotsContainer.InventorySlotElements.IsValidIndex(InCheckSlotIndex);
}
