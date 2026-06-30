// KJY All Rights Reserved


#include "Item/Slots/AGPItemSlot.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Item/Instance/AGPItemInstance.h"
#include "Log/AGPLogChannels.h"

UAGPItemSlot::UAGPItemSlot()
{
	
}

bool UAGPItemSlot::IsSupportedForNetworking() const
{
	return true;
}

void UAGPItemSlot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ItemInstanceInSlot, COND_OwnerOnly);
}

void UAGPItemSlot::OnRep_ItemInstance(UAGPItemInstance* InOldItemInstanceInSlot)
{
	if (InOldItemInstanceInSlot)
	{
		OnRemoveItemFromSlot(InOldItemInstanceInSlot);
	}

	if (ItemInstanceInSlot)
	{
		OnMountedItemInSlot();
	}
}

void UAGPItemSlot::MountItemInSlot(UAGPItemInstance* InItemInstance)
{
	verifyf(InItemInstance, TEXT("MountItemInSlot, but InItemInstance Invalid"));

	ItemInstanceInSlot = InItemInstance;

	OnMountedItemInSlot();
}

void UAGPItemSlot::RemoveItemFromSlot()
{
	verifyf(ItemInstanceInSlot, TEXT("ItemInstanceInSlot, but ItemInstanceInSlot Invalid"));

	UAGPItemInstance* RemoveItemInstance = ItemInstanceInSlot;
	ItemInstanceInSlot = nullptr;

	OnRemoveItemFromSlot(RemoveItemInstance);
}

void UAGPItemSlot::SetSlotOwner(UObject* InSlotOwner)
{
	SlotOwnerObject = InSlotOwner;
}

void UAGPItemSlot::UnbindItemSlotEvent(UObject* InUnbindObject)
{
	checkf(InUnbindObject, TEXT("[%s], InUnbindObject Invalid"), __FUNCTIONW__);

	for(TPair<EAGPSlotEvent, FOnItemSlotDelegate>& PairSlotEventDelegate : SlotEventDelegates)
	{ 
		PairSlotEventDelegate.Value.RemoveAll(InUnbindObject);
	}
}

void UAGPItemSlot::OnMountedItemInSlot()
{
	verifyf(ItemInstanceInSlot, TEXT("[%s], but ItemInstanceInSlot Invalid.."), __FUNCTIONW__)

	/* 아이템 인스턴스에게 해당 슬롯에 장착되었음을 알림 */
	ItemInstanceInSlot->OnMountedInSlot(this);

	/* 새로운 아이템 인스턴스가 슬롯에 위치해있음을 알림 */
	SlotEventDelegates[EAGPSlotEvent::MountedInSlot].Broadcast(this, ItemInstanceInSlot);
}

void UAGPItemSlot::OnRemoveItemFromSlot(UAGPItemInstance* InRemoveItemInstance)
{
	verifyf(InRemoveItemInstance, TEXT("[%s], but InRemoveItemInstance Invalid.."), __FUNCTIONW__)

	/* 아이템 인스턴스에게 해당 슬롯에서 해제되었음을 알림 */
	InRemoveItemInstance->OnRemovedFromSlot(this);

	/* 이전에 있던 아이템 인스턴스가 슬롯에서 제거되었음을 알림 */
	SlotEventDelegates[EAGPSlotEvent::RemoveFromSlot].Broadcast(this, InRemoveItemInstance);
}

void UAGPItemSlot::SetSlotCategoryTag(const FGameplayTag& InCategoryTag)
{
	SlotCategoryTag = InCategoryTag;
}

UObject* UAGPItemSlot::GetSlotOwner() const
{
	return SlotOwnerObject.Get();
}

ENetRole UAGPItemSlot::GetOwnerRole() const
{
	if (const AActor* OwnerActor = GetSlotOwner<AActor>())
	{
		return OwnerActor->GetLocalRole();
	}

	return ENetRole::ROLE_None;
}

bool UAGPItemSlot::IsEmpty() const
{
	return (nullptr == GetItemInstance());
}

FString UAGPItemSlot::GetDebugString() const
{
	if (IsEmpty())
	{
		return TEXT("Empty");
	}
	else
	{
		return ItemInstanceInSlot->GetDebugString();
	}
}
