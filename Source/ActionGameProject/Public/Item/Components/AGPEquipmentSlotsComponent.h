// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "item/Types/AGPItemSystemEnum.h"
#include "Item/Types/AGPItemSystemDelegate.h"
#include "AGPEquipmentSlotsComponent.generated.h"

class UAGPItemSlot;
class UAGPEquipmentSlot;
class UAGPItemInstance;

USTRUCT(BlueprintType)
struct FAGPEquipmentSlotElement : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	friend UAGPEquipmentSlotsComponent;
	friend FAGPEquipmentSlotElementsContainer;

private:
	UPROPERTY(VisibleInstanceOnly, Instanced)
	TObjectPtr<UAGPEquipmentSlot> EquipmentSlot = nullptr;

	UPROPERTY(VisibleInstanceOnly)
	FGameplayTag SlotTag = FGameplayTag::EmptyTag;

public:
	FAGPEquipmentSlotElement();
};

USTRUCT(BlueprintType)
struct FAGPEquipmentSlotElementsContainer : public FFastArraySerializer
{
	GENERATED_BODY()

private:
	friend UAGPEquipmentSlotsComponent;

private:
	UPROPERTY(VisibleInstanceOnly)
	TArray<FAGPEquipmentSlotElement> EquipmentSlotElements;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UAGPEquipmentSlotsComponent> OwnerEquipmentSlotsComponent;

public:
	FAGPEquipmentSlotElementsContainer();

public:
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

public:
	void InitializeContainer(UAGPEquipmentSlotsComponent* InEquipmentComponent);
	void AddEquipmentSlotElement(UAGPEquipmentSlot* InAddEquipmentSlot, const FGameplayTag& InSlotTag);
};

template<>
struct TStructOpsTypeTraits<FAGPEquipmentSlotElementsContainer> : public TStructOpsTypeTraitsBase2<FAGPEquipmentSlotElementsContainer>
{
	enum { WithNetDeltaSerializer = true };
};




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONGAMEPROJECT_API UAGPEquipmentSlotsComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	friend FAGPEquipmentSlotElementsContainer;

private:
	/* 장비슬롯자체에 대한 리플리케이션을 위한 FastArraySerializer */
	UPROPERTY(Replicated)
	FAGPEquipmentSlotElementsContainer EquipmentSlotsContainer;

	/* 장비슬롯태그 - 장비슬롯인스턴스 맵 */
	UPROPERTY(VisibleInstanceOnly, Instanced, Category = "EquipmentItems", meta = (ForceInlineRow))
	TMap<FGameplayTag, TObjectPtr<UAGPEquipmentSlot>> MapEquipmentSlots;

private:
	/* 초기화 과정에서 생성할 장비 슬롯들 */
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Items.Slots.Equipment"))
	TSet<FGameplayTag> InitializeEquipmentSlotTags;

public:	
	UAGPEquipmentSlotsComponent();

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
	void InitializeEquipmentSlots();
	void AddEquipmentSlot(const FGameplayTag& InSlotTag);
	void OnAddedEmptyEquipmentSlot(UAGPEquipmentSlot* InAddEquipmentSlot, const FGameplayTag& InAddSlotTag);

public:
	void OnEquipmentSlotItemMounted(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance);
	void OnEquipmentSlotItemRemoved(const UAGPItemSlot* InItemSlot, UAGPItemInstance* InItemInstance);

public:
	UAGPEquipmentSlot* GetEquipmentSlot(const FGameplayTag& InSlotTag) const;

private:
	/* 해당 장비슬롯 태그를 가지고 있는 장비슬롯이 존재하는지 여부 반환 */
	bool IsContainEquipSlot(const FGameplayTag& InTagEquipSlot) const;
};