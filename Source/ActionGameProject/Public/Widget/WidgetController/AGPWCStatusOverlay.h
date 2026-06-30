// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPSurvivalModeWCBase.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "AGPWCStatusOverlay.generated.h"

struct FOnAttributeChangeData;
struct FGameplayAbilitySpec;
struct FAGPBossStatusInfoCommon;
class AAGP_PlayerCharacter;
class UAGP_AbilitySystemComponent;
class UAGPAttributeSetBase;
class UAGPAttributeSetWarrior;
class AGameStateBase;



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangedOwnerPlayerStateSignature, bool, bPlayerStateValid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAddedSlotAbilitySignature, const FGameplayTag&, SlotTag, const FGameplayTag&, AbilityTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemovedSlotAbilitySignature, const FGameplayTag&, SlotTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAbilityCooldownBeginSignature, const FGameplayTag&, SlotTag, float, RemainingTime, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityCooldownEndSignature, const FGameplayTag&, SlotTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyChangeInteractionModeSignature, EAGPInteractionMode, InInteractionMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifyBossStatusSignature, const FAGPBossStatusInfoCommon&, BossStatusInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotifyControlMappingsRebuiltSignature);


/**
 * HUD의 캐릭터 상태 표시 위젯의 컨트롤러
 */

UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPWCStatusOverlay : public UAGPSurvivalModeWCBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthValueChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnRageValueChanged;

	UPROPERTY(BlueprintAssignable)
	FOnChangedOwnerPlayerStateSignature OnOwnerPlayerStateChanged;

	/* 슬롯에 특정 Ability 추가될때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnAddedSlotAbilitySignature OnAddedSlotAbility;

	/* 슬롯에 있던 Ability가 제거될때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnRemovedSlotAbilitySignature OnRemovedSlotAbility;

	/* 특정 슬롯에 위치한 Ability의 Cooldown이 시작될때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnAbilityCooldownBeginSignature OnAbilityCooldownBeginNotify;

	/* 특정 슬롯에 위치한 Ability의 Cooldown이 종료될때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnAbilityCooldownEndSignature OnAbilityCooldownEndNotify;

	/* Owner 캐릭터의 InteractionMode 변경에 따라 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FOnNotifyChangeInteractionModeSignature OnNotifyChangeInteractionMode;

	/* 표시할 Boss 상태가 추가될 때 호출 */
	UPROPERTY(BlueprintAssignable)
	FOnNotifyBossStatusSignature OnNotifyAddedBossStatus;

	/* 표시되고 있던 Boss 상태가 변경될 때 호출 */
	UPROPERTY(BlueprintAssignable)
	FOnNotifyBossStatusSignature OnNotifyModifyBossStatus;

	/* 표시되고 있던 Boss 상태가 제거될 때 호출 */
	UPROPERTY(BlueprintAssignable)
	FOnNotifyBossStatusSignature OnNotifyRemovedBossStatus;

	/* ControlMappingsRebuilt 이벤트 발생시 호출 */
	UPROPERTY(BlueprintAssignable)
	FOnNotifyControlMappingsRebuiltSignature OnNotifyControlMappingsRebuilt;

private:
	bool bPlayerStateValid = false;
	EAGPInteractionMode CurrentInteractionMode = EAGPInteractionMode::None;

	/* Common Attribute Set */
private:
	float CurrentHealthValue = 0.f;
	float MaxHealthValue = 0.f;

	/* Warrior Attribute Set */
	float CurrentRageValue = 0.f;
	float MaxRageValue = 0.f;

private:
	TWeakObjectPtr<UAGP_AbilitySystemComponent> CachedOwnerASC;
	TWeakObjectPtr<AAGP_PlayerCharacter> CachedOwnerCharacter;

	/* 현재 활성가능한 Ability중 Slot지정된 Ability들 */
	UPROPERTY()
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> CurrentHasSlotAbility;

	/* Cooldown GameplayTag - 해당되는 Ability의 Slot 정보들 */
	UPROPERTY()
	TMap<FGameplayTag, FGameplayTag> CooldownTagToAbilitySlot;

	/* 표시할 보스들 상태 정보 */
	UPROPERTY()
	TMap<int32, FAGPBossStatusInfoCommon> BossStatusInfoMap;

public:
	virtual void InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController) override;
	virtual void DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController) override;

private:
	void OnSetPlayerState(APlayerState* InOwnerPlayerState);
	void OnResetPlayerState();

	void OnAddedAttributeSet(UAGPAttributeSetBase* InAddAttributeSet);
	void OnRemoveAttributeSet(UAGPAttributeSetBase* InRemoveAttributeSet);

	void OnAddedAttributeSet_Warrior(UAGPAttributeSetWarrior* InAddAttributeSetWarrior);
	void OnRemoveAttributeSet_Warrior(UAGPAttributeSetWarrior* InRemoveAttributeSetWarrior);

	//TODO : 이후 다른 클래스 캐릭터 추가시에는 현재 캐릭터에 따라 다른 HUD 표시할수있게 해야 함

	void OnChangeCurrentHealth(const FOnAttributeChangeData& Data);
	void OnChangeMaxHealth(const FOnAttributeChangeData& Data);

	void OnChangeCurrentRage(const FOnAttributeChangeData& Data);
	void OnChangeMaxRage(const FOnAttributeChangeData& Data);

	void OnGrantedAbility(FGameplayAbilitySpec& InAbilitySpec);
	void OnRemovedAbility(FGameplayAbilitySpec& InAbilitySpec);

	/* 슬롯지정된 Ability 추가 */
	void OnAddedActivateSlotAbility(const FGameplayTag& InAbilitySlotTag, const FGameplayAbilitySpec& InAbilitySpec);
	void OnRemovedActivateSlotAbility(const FGameplayTag& InAbilitySlotTag);
	void OnRemovedActivateSlotAbility(const FGameplayTag& InAbilitySlotTag, const FGameplayAbilitySpec& InAbilitySpec);

	/* Ability Cooldown Event 수신 */
	void OnAbilityCooldownEventNotify(const FGameplayTag Tag, int32 NewCount);

	/* 특정 Ability Cooldown 시작 */
	void OnAbilityCooldownBegin(const FGameplayTag& InAbilitySlotTag, const FGameplayTag& InCooldownTag);

	/* 특정 Ability Cooldown 종료 */
	void OnAbilityCooldownEnd(const FGameplayTag& InAbilitySlotTag);

	/* Owner Character Possess 이벤트 수신 */
	void OnPossessOwnerCharacter(AAGP_PlayerCharacter* InNewCharacter);

	/* Owner Character Unpossess 이벤트 수신 */
	void OnUnpossessOwnerCharacter();

	/* Boss 상태 추가될때 이벤트 수신 */
	void OnAddedBossStatusInfo(const FAGPBossStatusInfoCommon& AddedBossStatusInfo);

	/* Boss 상태 수정될때 이벤트 수신 */
	void OnModifyBossStatusInfo(const FAGPBossStatusInfoCommon& ChangedBossStatusInfo);

	/* Boss 상태 제거될때 이벤트 수신 */
	void OnRemovedBossStatusInfo(const FAGPBossStatusInfoCommon& RemovedBossStatusInfo);

private:
	/* Owner Controller에 Possess 된 Pawn이 변경될때 호출되는 콜백 함수 */
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/* Owner Character의 Interaction Mode 변경 이벤트 콜백 함수 */
	UFUNCTION()
	void OnOwnerInteractionModeChanged(EAGPInteractionMode InInteractionMode);

	/* Control Mapping Context Rebuild 콜백 함수 */
	UFUNCTION()
	void OnControlMappingRebuiltEvent();

public:
	/* 해당 슬롯에 위치한 Ability의 Tag 반환 */
	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "SlotTag"))
	const FGameplayTag& GetAbilityTagFromAbilitySlot(const FGameplayTag& SlotTag) const;

	/* 해당 슬롯에 위치한 Ability의 Cooldown Tag 반환 */
	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "SlotTag"))
	const FGameplayTag& GetAbilityCooldownTagFromAbilitySlot(const FGameplayTag& SlotTag) const;

	/* 현재 Cooldown의 시간 정보(전체 시간, 경과 시간) 반환 */
	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "InCooldownTag"))
	bool GetCooldownTimeByTag(const FGameplayTag& InCooldownTag, float& OutRemainingTime, float& OutCooldownTotalTime) const;

	/* 현재 Cooldown 진행 중 여부 반환 */
	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "InCooldownTag"))
	bool IsCooldownProgress(const FGameplayTag& InCooldownTag) const;

private:
	/* AbilitySpecHandle로부터 OwnerASC의 AbilitySpec 반환 */
	FGameplayAbilitySpec* GetAbilitySpecFromHandle(const FGameplayAbilitySpecHandle& InSpecHandle) const;

	/* 특정 AbilitySpec으로부터 Ability 자체 Tag 찾아서 반환 */
	const FGameplayTag& GetAbilityTagFromSpec(const FGameplayAbilitySpec& InAbilitySpec) const;

	/* 특정 AbilitySpec으로부터 Ability의 SlotTag 찾아서 반환 */
	const FGameplayTag& GetAbilitySlotTagFromSpec(const FGameplayAbilitySpec& InAbilitySpec) const;

	/* 특정 AbilitySpec으로부터 Ability의 Cooldown Tag 찾아서 반환 */
	const FGameplayTag& GetCooldownTagFromSpec(const FGameplayAbilitySpec& InAbilitySpec) const;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsValidPlayerState() const { return bPlayerStateValid; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentHeatlhValue() const { return CurrentHealthValue; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHeatlhValue() const { return MaxHealthValue; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentRageValue() const { return CurrentRageValue; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxRageValue() const { return MaxRageValue; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE EAGPInteractionMode GetCurrentInteractionMode() const { return CurrentInteractionMode; }

	UFUNCTION(BlueprintPure)
	const TMap<int32, FAGPBossStatusInfoCommon>& GetBossStatusInfos() const {return BossStatusInfoMap;}
};
