// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AGPInventoryComponent.generated.h"

class UAGPItemSlot;
class UAGPInventorySlot;
class UAGPItemInstance;

USTRUCT(BlueprintType)
struct FAGPInventorySlotElement : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	friend UAGPInventoryComponent;
	friend FAGPInventorySlotElementsContainer;

private:
	UPROPERTY(VisibleInstanceOnly, Instanced)
	TObjectPtr<UAGPInventorySlot> InventorySlot = nullptr;

public:
	FAGPInventorySlotElement();
};

USTRUCT(BlueprintType)
struct FAGPInventorySlotElementsContainer : public FFastArraySerializer
{
	GENERATED_BODY()
	
private:
	friend UAGPInventoryComponent;

private:
	UPROPERTY(VisibleInstanceOnly)
	TArray<FAGPInventorySlotElement> InventorySlotElements;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UAGPInventoryComponent> OwnerInventoryComponent;

public:
	FAGPInventorySlotElementsContainer();

public:
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

public:
	void InitializeContainer(UAGPInventoryComponent* InInventoryComponent);
	void AddInventorySlotElement(UAGPInventorySlot* InAddInventorySlot);
};

template<>
struct TStructOpsTypeTraits<FAGPInventorySlotElementsContainer> : public TStructOpsTypeTraitsBase2<FAGPInventorySlotElementsContainer>
{
	enum { WithNetDeltaSerializer = true };
};

/* Inventory 관리 컴포넌트 */
UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	friend FAGPInventorySlotElementsContainer;
	
private:
	UPROPERTY(VisibleInstanceOnly, Replicated, meta = (ShowOnlyInnerProperties))
	FAGPInventorySlotElementsContainer InventorySlotsContainer;

private:
	/* 초기 인벤토리슬롯 최대 갯수 */
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClampMax=300, UIMin=0, UIMax=300))
	int32 InitailizeInventorySlotNum;

public:	
	UAGPInventoryComponent();

#pragma region[UActorComponent Interface]
public:
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;
#pragma endregion

private:
	void OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

private:
	void InitializeInventorySlots();
	void AddEmptyInventorySlot(const int32 InSlotIndex);
	void OnAddedEmptyInventorySlot(UAGPInventorySlot* InAddInventorySlot, const int32 InAddSlotIndex);

public:
	void OnInventorySlotItemMounted(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance);
	void OnInventorySlotItemRemoved(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance);

public:
	UAGPInventorySlot* GetInventorySlot(const int32 InSlotIndex) const;
	UAGPInventorySlot* GetCanPlacedInventorySlot(UAGPItemInstance* InItemInstance) const;

private:
	bool IsValidSlotIndex(const int32 InCheckSlotIndex) const;
};
