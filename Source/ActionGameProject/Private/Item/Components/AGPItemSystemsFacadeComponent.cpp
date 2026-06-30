// KJY All Rights Reserved


#include "Item/Components/AGPItemSystemsFacadeComponent.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"
#include "Item/FunctionLibrary/AGPItemFunctionLibrary.h"
#include "Item/Components/AGPInventoryComponent.h"
#include "Item/Components/AGPEquipmentSlotsComponent.h"
#include "Item/Components/AGPAbstractionsAvatarComponent.h"
#include "Item/Slots/AGPInventorySlot.h"
#include "Item/Slots/AGPEquipmentSlot.h"
#include "Abilities\GameplayAbilityTypes.h"
#include "AbilitySystem/AGPAbilityTypes.h"
#include "Costume/AGPCostumeStruct.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

UAGPItemSystemsFacadeComponent::UAGPItemSystemsFacadeComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UAGPItemSystemsFacadeComponent::SetInventoryComponent(UAGPInventoryComponent* InInventoryComponent)
{
	CachedInventoryComponent = InInventoryComponent;
}

void UAGPItemSystemsFacadeComponent::SetEquipmentSlotsComponent(UAGPEquipmentSlotsComponent* InEquipmentSlotsComponent)
{
	CachedEquipmentSlotsComponent = InEquipmentSlotsComponent;
}

void UAGPItemSystemsFacadeComponent::SetAbstractionsAvatarComponent(UAGPAbstractionsAvatarComponent* InAbstractionsAvatarComponent)
{
	CachedAbstractionsAvatarComponent = InAbstractionsAvatarComponent;
}

UAGPInventorySlot* UAGPItemSystemsFacadeComponent::GetInventorySlot(const int32 InSlotIndex) const
{
	if (UAGPInventoryComponent* InventoryComponent = CachedInventoryComponent.Get())
	{
		return InventoryComponent->GetInventorySlot(InSlotIndex);
	}

	return nullptr;
}

UAGPInventorySlot* UAGPItemSystemsFacadeComponent::GetCanPlacedInventorySlot(UAGPItemInstance* InItemInstance) const
{
	if (UAGPInventoryComponent* InventoryComponent = CachedInventoryComponent.Get())
	{
		return InventoryComponent->GetCanPlacedInventorySlot(InItemInstance);
	}

	return nullptr;
}

UAGPEquipmentSlot* UAGPItemSystemsFacadeComponent::GetEquipmentSlot(const FGameplayTag& InTagEquipSlot) const
{
	if (UAGPEquipmentSlotsComponent* EquipmentSlotsComponent = CachedEquipmentSlotsComponent.Get())
	{
		return EquipmentSlotsComponent->GetEquipmentSlot(InTagEquipSlot);
	}

	return nullptr;
}

bool UAGPItemSystemsFacadeComponent::OwnerHasTag(const FGameplayTag& InCheckTag) const
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (IsValid(AbilitySystemComponent))
	{
		const FGameplayTagContainer& OwnerGameplayTagContainer = AbilitySystemComponent->GetOwnedGameplayTags();
		return OwnerGameplayTagContainer.HasTag(InCheckTag);
	}

	return false;
}

bool UAGPItemSystemsFacadeComponent::OwnerHasAnyTag(const FGameplayTagContainer& InQueryTagContainer) const
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (IsValid(AbilitySystemComponent))
	{
		const FGameplayTagContainer& OwnerGameplayTagContainer = AbilitySystemComponent->GetOwnedGameplayTags();
		return OwnerGameplayTagContainer.HasAny(InQueryTagContainer);
	}

	return false;
}

FAGPRequestPacketKey UAGPItemSystemsFacadeComponent::RequestBaseFeature(const FGameplayTag& InBaseFeatureTag, const FGameplayAbilityTargetDataHandle& InPayloadEventData, const FItemBaseFeatureResultDelegate& InResultDelegate)
{
	FAGPRequestPacketKey RequestPacketKey;

	const uint8 CheckConditionResult = CheckBaseFeatureCondition(InBaseFeatureTag, InPayloadEventData);
	if (AGPBaseFeatureResult::Common::Success == CheckConditionResult)
	{
		if (ROLE_Authority == GetOwnerRole())
		{
			InternalBaseFeatureProcess(InBaseFeatureTag, InPayloadEventData);

			InResultDelegate.ExecuteIfBound(CheckConditionResult);
		}
		else
		{
			RequestPacketKey = RequestBaseFeature_ToServer(InBaseFeatureTag, InPayloadEventData, InResultDelegate);
		}
	}
	else
	{
		InResultDelegate.ExecuteIfBound(CheckConditionResult);
	}

	return RequestPacketKey;
}

void UAGPItemSystemsFacadeComponent::RemoveRequestPacketWaitInfo(const FAGPRequestPacketKey& InRequestPacketKey)
{
	BaseFeaturePacketResultDelegateMap.Remove(InRequestPacketKey);
}

void UAGPItemSystemsFacadeComponent::InitializeItemEquipment(TSubclassOf<UAGPItemInstance> InEquipItemClass, const FGameplayTag& InTagEquipSlot)
{
	//서버에서만 실행가능
	verifyf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	// 요청이 들어온 장비 슬롯이 존재하는지 확인
	UAGPEquipmentSlot* EquipmentSlotInstance = GetEquipmentSlot(InTagEquipSlot);
	verifyf(EquipmentSlotInstance, TEXT("%s, but Slot[%s] is Invalid.."), __FUNCTIONW__, *InTagEquipSlot.ToString());

	//TODO : 해당 슬롯에 착용이 가능한지 확인


	//장비할 아이템 인스턴스 생성
	UAGPItemInstance* NewEquipmentItemInstance = UAGPItemFunctionLibrary::CreateAGPItemInstance(this, InEquipItemClass, 1);
	verifyf(NewEquipmentItemInstance, TEXT("InitializePlayerItems - NewEquipmentItemInstance Invalid.."));

	//이전에 착용한 아이템이 있으면 뺀다.
	UAGPItemInstance* PreviousEquipItemInstance = EquipmentSlotInstance->GetItemInstance();
	if (PreviousEquipItemInstance)
	{
		EquipmentSlotInstance->RemoveItemFromSlot();
	}

	// 해당 장비 슬롯에 장착
	EquipmentSlotInstance->MountItemInSlot(NewEquipmentItemInstance);

	// 이전에 착용했던 아이템이 있으면 경고 표시
	if (PreviousEquipItemInstance)
	{
		AGP_NET_LOG(this, LogAGPItem, Warning, TEXT("%s - [%s]Slot Already EquipItem[%s]"), __FUNCTIONW__, *InTagEquipSlot.ToString(), *PreviousEquipItemInstance->GetDebugString());
	}
}

void UAGPItemSystemsFacadeComponent::InitializeItemToInventory(TSubclassOf<UAGPItemInstance> InItemClass, const int32 InSlotIndex, const int32 InItemStackNum)
{
	//서버에서만 실행가능
	verifyf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	//인벤토리 내 슬롯을 가져옴
	UAGPInventorySlot* InventorySlotInstance = GetInventorySlot(InSlotIndex);
	verifyf(InventorySlotInstance, TEXT("%s, but Slot[%d] is Invalid.."), __FUNCTIONW__, InSlotIndex);

	//인벤토리에 추가할 아이템 인스턴스 생성
	UAGPItemInstance* NewInventoryItemInstance = UAGPItemFunctionLibrary::CreateAGPItemInstance(this, InItemClass, InItemStackNum);
	verifyf(NewInventoryItemInstance, TEXT("%s - NewInventoryItemInstance Invalid.."), __FUNCTIONW__);

	//슬롯에 이전에 위치해 있던 아이템이 있으면 제거
	UAGPItemInstance* PreviousItemInstance = InventorySlotInstance->GetItemInstance();
	if (PreviousItemInstance)
	{
		InventorySlotInstance->RemoveItemFromSlot();
	}

	// 해당 슬롯에 아이템 위치
	InventorySlotInstance->MountItemInSlot(NewInventoryItemInstance);

	// 이전에 위치해 있던 아이템이 있으면 경고 표시
	if (PreviousItemInstance)
	{
		AGP_NET_LOG(this, LogAGPItem, Warning, TEXT("%s - [%d]InventorySlot Already Mounted Item[%s]"), __FUNCTIONW__, InSlotIndex, *PreviousItemInstance->GetDebugString());
	}
}

void UAGPItemSystemsFacadeComponent::AddCostumeToOwner(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, TSubclassOf<UAttachCostumeBase> InCostumeClass)
{
	//서버에서만 실행가능
	checkf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	if (UAGPAbstractionsAvatarComponent* OwnerAbstractionsAvatarComponent = CachedAbstractionsAvatarComponent.Get())
	{
		OwnerAbstractionsAvatarComponent->AddCostumeInstance(InCostumeIdentifierHandle, InCostumeClass);
	}
}

void UAGPItemSystemsFacadeComponent::RemoveCostumeInstance(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle)
{
	//서버에서만 실행가능
	checkf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	if (UAGPAbstractionsAvatarComponent* OwnerAbstractionsAvatarComponent = CachedAbstractionsAvatarComponent.Get())
	{
		OwnerAbstractionsAvatarComponent->RemoveCostumeInstance(InCostumeIdentifierHandle);
	}
}


void UAGPItemSystemsFacadeComponent::SetOwnerAnimLinkLayer(const FName& InAnimLinkLayerName)
{
	//서버에서만 실행가능
	verifyf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	if (UAGPAbstractionsAvatarComponent* OwnerAbstractionsAvatarComponent = CachedAbstractionsAvatarComponent.Get())
	{
		OwnerAbstractionsAvatarComponent->SetAnimationLinkLayer(InAnimLinkLayerName);
	}
}

void UAGPItemSystemsFacadeComponent::ResetOwnerAnimLinkLayer()
{
	//서버에서만 실행가능
	verifyf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	if (UAGPAbstractionsAvatarComponent* OwnerAbstractionsAvatarComponent = CachedAbstractionsAvatarComponent.Get())
	{
		OwnerAbstractionsAvatarComponent->ResetAnimLinkLayer();
	}
}

uint8 UAGPItemSystemsFacadeComponent::CheckBaseFeatureCondition(const FGameplayTag& InBaseFeatureTag, const FGameplayAbilityTargetDataHandle& InPayloadEventData)
{
	if (InBaseFeatureTag.MatchesTagExact(AGPGameplayTags::Ability_Interaction_Item_EquipItem))
	{
		return CheckBaseFeatureCondition_InventoryItemEquip(InPayloadEventData);
	}

	if (InBaseFeatureTag.MatchesTagExact(AGPGameplayTags::Ability_Interaction_Item_DisrobeEquipment))
	{
		return CheckBaseFeatureCondition_DisrobeEquipmentToInventory(InPayloadEventData);
	}

	return AGPBaseFeatureResult::Common::Unknonw_Fail;
}

uint8 UAGPItemSystemsFacadeComponent::CheckBaseFeatureCondition_InventoryItemEquip(const FGameplayAbilityTargetDataHandle& InPayloadEventData)
{
	const FGameplayAbilityTargetData_ReuqestInventoryItemEquip* ReuqestInventoryItemEquip = static_cast<const FGameplayAbilityTargetData_ReuqestInventoryItemEquip*>(InPayloadEventData.Get(0));
	checkf(ReuqestInventoryItemEquip, TEXT("[%s], Invalid ReuqestInventoryItemEquip"), __FUNCTIONW__);

	//1. 인덱스에 해당하는 슬롯을 확인
	UAGPInventorySlot* InventorySlotInstance = GetInventorySlot(ReuqestInventoryItemEquip->InventorySlotIndex);
	if (!InventorySlotInstance)
	{
		return AGPBaseFeatureResult::ReuqestInventoryItemEquip::InvalidSlotIndex;
	}

	//2. 슬롯에 위치한 아이템 확인
	UAGPItemInstance* InventoryItemInstance = InventorySlotInstance->GetItemInstance();
	if (!InventoryItemInstance)
	{
		return AGPBaseFeatureResult::ReuqestInventoryItemEquip::InvalidSlotInItemInstance;
	}

	//3. 요청이 들어온 장비 슬롯이 존재하는지 확인
	UAGPEquipmentSlot* EquipmentSlotInstance = GetEquipmentSlot(ReuqestInventoryItemEquip->EquipSlotTag);
	if (!EquipmentSlotInstance)
	{
		return AGPBaseFeatureResult::ReuqestInventoryItemEquip::InvalidEquipmentSlot;
	}

	//TODO : 4. 해당 슬롯에 착용이 가능한지 확인


	return AGPBaseFeatureResult::Common::Success;
}

uint8 UAGPItemSystemsFacadeComponent::CheckBaseFeatureCondition_DisrobeEquipmentToInventory(const FGameplayAbilityTargetDataHandle& InPayloadEventData)
{
	const FGameplayAbilityTargetData_DisrobeEquipmentToInventory* DisrobeEquipmentToInventory = static_cast<const FGameplayAbilityTargetData_DisrobeEquipmentToInventory*>(InPayloadEventData.Get(0));
	checkf(DisrobeEquipmentToInventory, TEXT("[%s], Invalid DisrobeEquipmentToInventory"), __FUNCTIONW__);

	//1. 요청이 들어온 장비 슬롯이 존재하는지 확인
	UAGPEquipmentSlot* EquipmentSlotInstance = GetEquipmentSlot(DisrobeEquipmentToInventory->EquipSlotTag);
	if (!EquipmentSlotInstance)
	{
		return AGPBaseFeatureResult::DisrobeEquipmentToInventory::InvalidEquipmentSlot;
	}

	//2. 장비 슬롯에 장착한 아이템이 있는지 확인
	UAGPItemInstance* EquipItemInstance = EquipmentSlotInstance->GetItemInstance();
	if (!EquipItemInstance)
	{
		return AGPBaseFeatureResult::DisrobeEquipmentToInventory::InvalidSlotInItemInstance;
	}

	//3. 장비가 해제된 후 인벤토리 내 위치가 예상되는 슬롯 탐색 및 확인
	UAGPInventorySlot* TargetInventorySlotInstance = nullptr;
	if (AGPItemConstDefine::INVALID_ITEM_SLOT_INDEX == DisrobeEquipmentToInventory->InventorySlotIndex)
	{
		//인벤토리 슬롯 인덱스가 지정되어있지 않은 경우 장비 해제 후 담을 수 있는 빈 공간을 확인
		TargetInventorySlotInstance = GetCanPlacedInventorySlot(EquipItemInstance);
	}
	else
	{
		UAGPInventorySlot* InventorySlotInstance = GetInventorySlot(DisrobeEquipmentToInventory->InventorySlotIndex);
		if (!InventorySlotInstance)
		{
			return AGPBaseFeatureResult::DisrobeEquipmentToInventory::InvalidInventorySlot;
		}

		if (!InventorySlotInstance->IsEmpty())
		{
			//TODO : 안비어있으면 스택가능한지 체크, 현재는 그냥 return;
			return AGPBaseFeatureResult::DisrobeEquipmentToInventory::InvalidInventorySlot;
		}

		TargetInventorySlotInstance = InventorySlotInstance;
	}

	if (!TargetInventorySlotInstance)
	{
		return AGPBaseFeatureResult::DisrobeEquipmentToInventory::InvalidInventorySlot;
	}

	return AGPBaseFeatureResult::Common::Success;
}

void UAGPItemSystemsFacadeComponent::InternalBaseFeatureProcess(const FGameplayTag& InBaseFeatureTag, const FGameplayAbilityTargetDataHandle& InPayloadEventData)
{
	if (InBaseFeatureTag.MatchesTagExact(AGPGameplayTags::Ability_Interaction_Item_EquipItem))
	{
		InternalBaseFeatureProcess_InventoryItemEquip(InPayloadEventData);
	}

	if (InBaseFeatureTag.MatchesTagExact(AGPGameplayTags::Ability_Interaction_Item_DisrobeEquipment))
	{
		InternalBaseFeatureProcess_DisrobeEquipmentToInventory(InPayloadEventData);
	}
}

void UAGPItemSystemsFacadeComponent::InternalBaseFeatureProcess_InventoryItemEquip(const FGameplayAbilityTargetDataHandle& InPayloadEventData)
{
	checkf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	const FGameplayAbilityTargetData_ReuqestInventoryItemEquip* ReuqestInventoryItemEquip = static_cast<const FGameplayAbilityTargetData_ReuqestInventoryItemEquip*>(InPayloadEventData.Get(0));
	checkf(ReuqestInventoryItemEquip, TEXT("[%s], Invalid ReuqestInventoryItemEquip"), __FUNCTIONW__);

	UAGPInventorySlot* InventorySlotInstance = GetInventorySlot(ReuqestInventoryItemEquip->InventorySlotIndex);
	checkf(InventorySlotInstance, TEXT("[%s], Invalid InventorySlotInstance"), __FUNCTIONW__);
	
	UAGPEquipmentSlot* EquipmentSlotInstance = GetEquipmentSlot(ReuqestInventoryItemEquip->EquipSlotTag);
	checkf(EquipmentSlotInstance, TEXT("[%s], Invalid EquipmentSlotInstance"), __FUNCTIONW__);

	UAGPItemInstance* InventoryItemInstance = InventorySlotInstance->GetItemInstance();
	checkf(InventoryItemInstance, TEXT("[%s], InventoryItemInstance Invalid..."), __FUNCTIONW__);

	//1. 인벤토리 슬롯에서 아이템을 먼저 뺀다.
	InventorySlotInstance->RemoveItemFromSlot();

	//2. 이전에 장착 슬롯에 착용했던 아이템이 있으면 뺀다.
	UAGPItemInstance* PreviousEquipItemInstance = EquipmentSlotInstance->GetItemInstance();
	if (PreviousEquipItemInstance)
	{
		EquipmentSlotInstance->RemoveItemFromSlot();
	}

	//3. 해당 장비 슬롯에 장착
	EquipmentSlotInstance->MountItemInSlot(InventoryItemInstance);

	//4. 이전에 착용했던 아이템이 있으면 인벤토리 슬롯에 위치시킴
	if (PreviousEquipItemInstance)
	{
		InventorySlotInstance->MountItemInSlot(PreviousEquipItemInstance);
	}
}

void UAGPItemSystemsFacadeComponent::InternalBaseFeatureProcess_DisrobeEquipmentToInventory(const FGameplayAbilityTargetDataHandle& InPayloadEventData)
{
	checkf(ROLE_Authority == GetOwnerRole(), TEXT("%s, but OwnerRole not authority[%s]..."), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	const FGameplayAbilityTargetData_DisrobeEquipmentToInventory* DisrobeEquipmentToInventory = static_cast<const FGameplayAbilityTargetData_DisrobeEquipmentToInventory*>(InPayloadEventData.Get(0));
	checkf(DisrobeEquipmentToInventory, TEXT("[%s], Invalid DisrobeEquipmentToInventory"), __FUNCTIONW__);

	UAGPEquipmentSlot* EquipmentSlotInstance = GetEquipmentSlot(DisrobeEquipmentToInventory->EquipSlotTag);
	checkf(EquipmentSlotInstance, TEXT("%s, but EquipmentSlotInstance Invalid..."), __FUNCTIONW__);

	UAGPInventorySlot* InventorySlotInstance = GetInventorySlot(DisrobeEquipmentToInventory->InventorySlotIndex);
	checkf(InventorySlotInstance, TEXT("%s, but InventorySlotInstance Invalid..."), __FUNCTIONW__);

	UAGPItemInstance* EquippedItemInstance = EquipmentSlotInstance->GetItemInstance();
	checkf(EquippedItemInstance, TEXT("%s, but EquippedItemInstance Invalid..."), __FUNCTIONW__, *EquipmentSlotInstance->GetDebugString());

	//1. 장비슬롯에 착용했던 아이템을 뺀다.
	EquipmentSlotInstance->RemoveItemFromSlot();

	//2. 인벤토리 슬롯에 위치시킴
	//TODO : 스택 가능할때 처리
	InventorySlotInstance->MountItemInSlot(EquippedItemInstance);
}

FAGPRequestPacketKey UAGPItemSystemsFacadeComponent::RequestBaseFeature_ToServer(const FGameplayTag InBaseFeatureTag, const FGameplayAbilityTargetDataHandle InPayloadEventData, const FItemBaseFeatureResultDelegate& InResultDelegate)
{
	/* 이번 요청에 대한 Packet Key 생성 */
	FAGPRequestPacketKey NewPacketKey = FAGPRequestPacketKeyGenerator::GenerateRequestPacketKey();

	/* Key가 존재하는경우 => 오류 상황, 로그 표시 및 덮어쓰기 */
	if (BaseFeaturePacketResultDelegateMap.Contains(NewPacketKey))
	{
		AGP_NET_LOG(this, LogAGPItem, Error, TEXT("BaseFeature Packet Key[%s] Contain.."), *NewPacketKey.ToString());
	}

	/* Server RPC호출 전 Delegate를 PacketKey에 맵핑하여 저장 */
	FAGPItemBaseFeatureRequestContext& AddResultContext = BaseFeaturePacketResultDelegateMap.Add(NewPacketKey);
	AddResultContext.RPCResultDelegate = InResultDelegate;

	/* Timeout 타이머 설정을 위해 World를 참조하기 때문에 이를 확인 */
	UWorld* CurrentWorld = GetWorld();
	checkf(CurrentWorld, TEXT("[%s], Invalid CurrentWorld"), __FUNCTIONW__);

	/* 요청에 대한 응답이 오지 않을때 호출할 Timeout 타이머 설정(최대 1분) */
	FTimerDelegate RPCTimeoutDelegate;
	RPCTimeoutDelegate.BindUObject(this, &UAGPItemSystemsFacadeComponent::BroadcastRPCResult, AGPBaseFeatureResult::Common::RPCWaitTimeout, NewPacketKey);
	CurrentWorld->GetTimerManager().SetTimer(AddResultContext.RPCWaitTimerHandle, RPCTimeoutDelegate, 60.f, false);
	
	ServerRequestBaseFeature(InBaseFeatureTag, InPayloadEventData, NewPacketKey);

	return NewPacketKey;
}

void UAGPItemSystemsFacadeComponent::BroadcastRPCResult(const uint8 InResult, const FAGPRequestPacketKey InRequestPacketKey)
{
	/* 처리결과를 델리게이트를 통해 Broadcast하고, 타이머 리셋 후 정리 */
	FAGPItemBaseFeatureRequestContext* FindRequestContext = BaseFeaturePacketResultDelegateMap.Find(InRequestPacketKey);
	if (!FindRequestContext)
	{
		return;
	}

	FindRequestContext->RPCResultDelegate.ExecuteIfBound(InResult);
	if (UWorld* CurrentWorld = GetWorld())
	{
		CurrentWorld->GetTimerManager().ClearTimer(FindRequestContext->RPCWaitTimerHandle);
	}
	FindRequestContext->Reset();

	/* 그 이후 Context정보는 맵에서 삭제 */
	BaseFeaturePacketResultDelegateMap.Remove(InRequestPacketKey);
}

void UAGPItemSystemsFacadeComponent::ServerRequestBaseFeature_Implementation(const FGameplayTag InBaseFeatureTag, const FGameplayAbilityTargetDataHandle InPayloadEventData, const FAGPRequestPacketKey InRequestPacketKey)
{
	//Server에서 관련 기능 실행중 다음과 같은 상태들이 있으면 실행불가
	FGameplayTagContainer QueryTags;
	QueryTags.AddTag(AGPGameplayTags::State_DamageResponse);
	//QueryTags.AddTag(AGPGameplayTags::State_ActiveAbility);
	QueryTags.AddTag(AGPGameplayTags::State_Death);

	if(OwnerHasAnyTag(QueryTags))
	{
		ClientBaseFeatureResult(AGPBaseFeatureResult::Common::RejectAbilityActivating, InRequestPacketKey);
	}
	else
	{
		const uint8 CheckConditionResult = CheckBaseFeatureCondition(InBaseFeatureTag, InPayloadEventData);
		if (AGPBaseFeatureResult::Common::Success == CheckConditionResult)
		{
			InternalBaseFeatureProcess(InBaseFeatureTag, InPayloadEventData);
		}

		ClientBaseFeatureResult(CheckConditionResult, InRequestPacketKey);
	}
}

bool UAGPItemSystemsFacadeComponent::ServerRequestBaseFeature_Validate(const FGameplayTag InBaseFeatureTag, const FGameplayAbilityTargetDataHandle InPayloadEventData, const FAGPRequestPacketKey InRequestPacketKey)
{
	return true;
}

void UAGPItemSystemsFacadeComponent::ClientBaseFeatureResult_Implementation(const uint8 InResult, const FAGPRequestPacketKey InRequestPacketKey)
{
	/* 서버에서 처리결과 수신, 처리결과 알림 */
	BroadcastRPCResult(InResult, InRequestPacketKey);
}