// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "item/Types/AGPItemSystemEnum.h"
#include "Item/Types/AGPItemSystemConstDefine.h"
#include "Item/Types/AGPItemSystemDelegate.h"
#include "Common/AGPCommonStructs.h"
#include "AGPItemSystemsFacadeComponent.generated.h"

struct FGameplayEventData;
struct FCostumeInstanceIdentifierHandle;

class UAGPInventoryComponent;
class UAGPEquipmentSlotsComponent;
class UAGPAbstractionsAvatarComponent;
class UAGPItemInstance;
class UAGPInventorySlot;
class UAGPEquipmentSlot;
class UAttachCostumeBase;


DECLARE_DELEGATE_OneParam(FItemBaseFeatureResultDelegate, const uint8 /*ResultCode*/);

USTRUCT()
struct FAGPItemBaseFeatureRequestContext
{
	GENERATED_BODY()
	
public:
	FTimerHandle RPCWaitTimerHandle;
	FItemBaseFeatureResultDelegate RPCResultDelegate;

public:
	void Reset()
	{
		RPCWaitTimerHandle.Invalidate();
		RPCResultDelegate.Unbind();
	}
};


/* 외부 -> 아이템 시스템들의 각 기능에 접근하기 위한 퍼사드 패턴 컴포넌트 */

UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPItemSystemsFacadeComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	TWeakObjectPtr<UAGPInventoryComponent> CachedInventoryComponent;
	TWeakObjectPtr<UAGPEquipmentSlotsComponent> CachedEquipmentSlotsComponent;
	TWeakObjectPtr<UAGPAbstractionsAvatarComponent> CachedAbstractionsAvatarComponent;

private:
	/* 서버로 보낸 요청 RPC에 대한 결과 대기 Delegate들 */
	TMap<FAGPRequestPacketKey, FAGPItemBaseFeatureRequestContext> BaseFeaturePacketResultDelegateMap;
	
public:	
	UAGPItemSystemsFacadeComponent();
		
public:
	void SetInventoryComponent(UAGPInventoryComponent* InInventoryComponent);
	void SetEquipmentSlotsComponent(UAGPEquipmentSlotsComponent* InEquipmentSlotsComponent);
	void SetAbstractionsAvatarComponent(UAGPAbstractionsAvatarComponent* InAbstractionsAvatarComponent);

	/* 기능 Function */
protected:
	/* 인벤토리 내 특정 슬롯에 대한 인스턴스 반환 */
	UAGPInventorySlot* GetInventorySlot(const int32 InSlotIndex) const;

	/* 인벤토리 내 해당아이템을 위치시킬 수 있는 인벤토리 슬롯 인스턴스 반환*/
	UAGPInventorySlot* GetCanPlacedInventorySlot(UAGPItemInstance* InItemInstance) const;

	/* 특정 슬롯에 대한 장비슬롯 인스턴스 반환 */
	UAGPEquipmentSlot* GetEquipmentSlot(const FGameplayTag& InTagEquipSlot) const;
	
	/* Owner가 해당 태그를 가지고 있는지 확인 */
	bool OwnerHasTag(const FGameplayTag& InCheckTag) const;

	/* Owner의 Query 태그 컨테이너 내 태그 소유 여부 반환(HasAny) */
	bool OwnerHasAnyTag(const FGameplayTagContainer& InQueryTagContainer) const;

	/* 외부 클래스에 공개 함수들(Client => Server, Server Call) */
public:
	FAGPRequestPacketKey RequestBaseFeature(const FGameplayTag& InBaseFeatureTag, const FGameplayAbilityTargetDataHandle& InPayloadEventData, const FItemBaseFeatureResultDelegate& InResultDelegate);
	void RemoveRequestPacketWaitInfo(const FAGPRequestPacketKey& InRequestPacketKey);

	/* 외부 클래스에 공개 함수들(Server Call) */
public:
	/* 초기 장비 아이템 착용 */
	void InitializeItemEquipment(TSubclassOf<UAGPItemInstance> InEquipItemClass, const FGameplayTag& InTagEquipSlot);

	/* 초기 인벤토리 아이템 삽입 */
	void InitializeItemToInventory(TSubclassOf<UAGPItemInstance> InItemClass, const int32 InSlotIndex, const int32 InItemStackNum);

	/* 특정 Costume 추가 */
	void AddCostumeToOwner(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, TSubclassOf<UAttachCostumeBase> InCostumeClass);

	/* 특정 Costume Instance 제거 */
	void RemoveCostumeInstance(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle);

	/* AnimLinkLayer 설정 */
	void SetOwnerAnimLinkLayer(const FName& InAnimLinkLayerName);

	/* AnimLinkLayer 초기화 */
	void ResetOwnerAnimLinkLayer();

private:
	/* BaseFeature 실행 가능 여부 조건 검사 */
	uint8 CheckBaseFeatureCondition(const FGameplayTag& InBaseFeatureTag, const FGameplayAbilityTargetDataHandle& InPayloadEventData);

	/* Inventory내 특정 아이템 장착에 대한 조건검사 */
	uint8 CheckBaseFeatureCondition_InventoryItemEquip(const FGameplayAbilityTargetDataHandle& InPayloadEventData);

	/* 장착한 장비 해제 후 인벤토리에 위치시킬때 조건검사 */
	uint8 CheckBaseFeatureCondition_DisrobeEquipmentToInventory(const FGameplayAbilityTargetDataHandle& InPayloadEventData);

private:
	/* BaseFeature 실행 */
	void InternalBaseFeatureProcess(const FGameplayTag& InBaseFeatureTag, const FGameplayAbilityTargetDataHandle& InPayloadEventData);

	/* BaseFeature - Inventory내 특정 아이템 장착 실행 */
	void InternalBaseFeatureProcess_InventoryItemEquip(const FGameplayAbilityTargetDataHandle& InPayloadEventData);

	/* BaseFeature - 장비 아이템 해제 후 인벤토리에 위치 */
	void InternalBaseFeatureProcess_DisrobeEquipmentToInventory(const FGameplayAbilityTargetDataHandle& InPayloadEventData);

private:
	FAGPRequestPacketKey RequestBaseFeature_ToServer(const FGameplayTag InBaseFeatureTag, const FGameplayAbilityTargetDataHandle InPayloadEventData, const FItemBaseFeatureResultDelegate& InResultDelegate);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestBaseFeature(const FGameplayTag InBaseFeatureTag, const FGameplayAbilityTargetDataHandle InPayloadEventData, const FAGPRequestPacketKey InRequestPacketKey);

	UFUNCTION(Client, Reliable)
	void ClientBaseFeatureResult(const uint8 InResult, const FAGPRequestPacketKey InRequestPacketKey);

	UFUNCTION()
	void BroadcastRPCResult(const uint8 InResult, const FAGPRequestPacketKey InRequestPacketKey);
	
};
