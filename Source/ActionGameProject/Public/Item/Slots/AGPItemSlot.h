// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "item/Types/AGPItemSystemEnum.h"
#include "Item/Types/AGPItemSystemDelegate.h"
#include "AGPItemSlot.generated.h"

class UAGPItemInstance;

/**
 * 게임 내 아이템 인스턴스가 위치하는 슬롯들의 Base Class
 */

UCLASS(DefaultToInstanced, CollapseCategories)
class ACTIONGAMEPROJECT_API UAGPItemSlot : public UObject
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	FGameplayTag SlotCategoryTag;

	TWeakObjectPtr<UObject> SlotOwnerObject;
	
protected:
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_ItemInstance)
	TObjectPtr<UAGPItemInstance> ItemInstanceInSlot;

private:
	TMap<EAGPSlotEvent, FOnItemSlotDelegate> SlotEventDelegates =
	{
		{EAGPSlotEvent::MountedInSlot, FOnItemSlotDelegate()},
		{EAGPSlotEvent::RemoveFromSlot, FOnItemSlotDelegate()},
	};

public:
	UAGPItemSlot();

#pragma region[UObject Interface]
public:
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#pragma endregion

protected:
	UFUNCTION()
	virtual void OnRep_ItemInstance(UAGPItemInstance* InOldItemInstanceInSlot);

public:
	void MountItemInSlot(UAGPItemInstance* InItemInstance);
	void RemoveItemFromSlot();

public:
	void SetSlotOwner(UObject* InSlotOwner);

	template<class UserClass>
	void BindItemSlotEvent(EAGPSlotEvent InSlotEvent, UserClass* InUserObject, void (UserClass::* InBindFunction)(const UAGPItemSlot*, UAGPItemInstance*));

	void UnbindItemSlotEvent(UObject* InUnbindObject);

protected:
	void OnMountedItemInSlot();
	void OnRemoveItemFromSlot(UAGPItemInstance* InRemoveItemInstance);

protected:
	void SetSlotCategoryTag(const FGameplayTag& InCategoryTag);

public:
	FORCEINLINE UAGPItemInstance* GetItemInstance() const { return ItemInstanceInSlot; }
	FORCEINLINE const FGameplayTag& GetSlotCategory() const { return SlotCategoryTag; }

public:
	UObject* GetSlotOwner() const;
	
	template <class T>
	T* GetSlotOwner() const
	{
		return Cast<T>(GetSlotOwner());
	}

	ENetRole GetOwnerRole() const;
	bool IsEmpty() const;

public:
	virtual FString GetDebugString() const;
};

template<class UserClass>
void UAGPItemSlot::BindItemSlotEvent(EAGPSlotEvent InSlotEvent, UserClass* InUserObject, void (UserClass::* InBindFunction)(const UAGPItemSlot*, UAGPItemInstance*))
{
	SlotEventDelegates[InSlotEvent].AddUObject(InUserObject, InBindFunction);
}