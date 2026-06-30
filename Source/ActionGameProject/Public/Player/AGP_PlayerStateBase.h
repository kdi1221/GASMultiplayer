// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "Item/Interfaces/AGPItemSystemExternalInterface.h"
#include "Item/Types/AGPItemSystemStruct.h"
#include "GenericTeamAgentInterface.h"
#include "Common/AGPCommonEnums.h"
#include "AGP_PlayerStateBase.generated.h"

class UAGP_AbilitySystemComponent;
class UAGPAttributeSetCommon;
class UAGPInventoryComponent;
class UAGPEquipmentSlotsComponent;
class UAGPAbstractionsAvatarComponent;
class UAGPItemSystemsFacadeComponent;
class UAGPItemInstance;
class UAGPGameplayAbility;
class AAGP_CharacterBase;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStateInSurvivalModeChanged, const AAGP_PlayerStateBase*, EAGPSurvivalModePlayerState);

/**
 * 플레이어의 상태 정보 저장
 * 여기서는 플레이어 캐릭터와 관련해서 AbilitySystemComponent 소유
 */

UCLASS()
class ACTIONGAMEPROJECT_API AAGP_PlayerStateBase : public APlayerState
												, public IAbilitySystemInterface
												, public IAGPItemSystemExternalInterface
												, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UAGP_AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem|Attribute")
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

protected:
	UPROPERTY(VisibleInstanceOnly, Category = "TeamGroup")
	FGenericTeamId PlayerTeamGroupID;

protected:
	/* Survival Mode에서의 PlayerState 상태 */
	UPROPERTY()
	EAGPSurvivalModePlayerState CurrentStateInSurvivalMode = EAGPSurvivalModePlayerState::None;

public:
	FOnStateInSurvivalModeChanged OnStateInSurvivalModeChanged;
	
public:
	AAGP_PlayerStateBase();

#pragma region[IAbilitySystemInterface]
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
#pragma endregion

#pragma region[IAGPItemSystemExternalInterface]
public:
	virtual UAGPItemSystemsFacadeComponent* GetItemSystemFacadeComponent() const override;
#pragma endregion

#pragma region[IGenericTeamAgentInterface]
public:
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
#pragma endregion
	


#pragma region[Owner Interface]
public:
	virtual void PostInitializeComponents() override;
#pragma endregion

public:
	void InitializePlayerItems();
	void InitializePlayerAbilities();

	void SetAvatarCharacter(AAGP_CharacterBase* InAvatarCharacter);
	void ResetAvatarCharacter();

	void SetCurrentStateInSurvivalMode(const EAGPSurvivalModePlayerState InNewState);

public:
	FORCEINLINE UAGPAttributeSetCommon* GetAttributeSetCommon() const { return CommonAttributeSet; }
	FORCEINLINE bool IsAliveInSurvivalMode() const { return EAGPSurvivalModePlayerState::Active == CurrentStateInSurvivalMode; }
};
