// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Characters/AGP_CharacterBase.h"
#include "Item/Types/AGPItemSystemStruct.h"
#include "GenericTeamAgentInterface.h"
#include "Common/AGPCommonEnums.h"
#include "AGP_NPCBase.generated.h"

class UAGPInventoryComponent;
class UAGPEquipmentSlotsComponent;
class UAGPAbstractionsAvatarComponent;
class UAGPItemSystemsFacadeComponent;

/* 랜덤하게 NPC 소환(스킬, GameMode 등)시 NPC 클래스 및 확률 가중치 정보 */
USTRUCT(BlueprintType)
struct FAGPNPCSummonInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AAGP_NPCBase> SpawnNPCClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpawnWeight = 0.f;
};

/**
 * 게임 내 None Player Character(몬스터, 기타 비플레이어 캐릭터들)의 Base 클래스
 */

UCLASS()
class ACTIONGAMEPROJECT_API AAGP_NPCBase : public AAGP_CharacterBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGP|AbilitySystem")
	TObjectPtr<UAGP_AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGP|AbilitySystem")
	TObjectPtr<UAGPAttributeSetCommon> CommonAttributeSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemSystem")
	TObjectPtr<UAGPInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemSystem")
	TObjectPtr<UAGPEquipmentSlotsComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemSystem")
	TObjectPtr<UAGPAbstractionsAvatarComponent> AbstractionsAvatarComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemSystem")
	TObjectPtr<UAGPItemSystemsFacadeComponent> ItemSystemFacadeComponent;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialize", meta = (Categories = "Items.Slots.Equipment"))
	TMap<FGameplayTag, TSubclassOf<UAGPItemInstance>> InitializeEquipments;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialize")
	TArray<FInitializeInventoryItemData> InitializeInventoryItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Property")
	int32 MonsterNameID = 0;

public:
	AAGP_NPCBase();

#pragma region[AActor Interface]
public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
#pragma endregion

#pragma region[AAGP_CharacterBase Interface]
public:
	virtual UAGP_AbilitySystemComponent* GetAGPAbilitySystemComponent() const override;
	virtual UAGPAttributeSetCommon* GetCommonAttributeSet() const override;
#pragma endregion

#pragma region[IAGPItemSystemExternalInterface]
public:
	virtual UAGPItemSystemsFacadeComponent* GetItemSystemFacadeComponent() const override;
#pragma endregion

#pragma region[IGenericTeamAgentInterface]
public:
	virtual FGenericTeamId GetGenericTeamId() const override;
#pragma endregion

private:
	void InitializeItems();
	void InitializeAbility();

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetMonsterNameID() const {return MonsterNameID;}
};
