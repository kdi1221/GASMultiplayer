// KJY All Rights Reserved


#include "Widget/WidgetController/AGPWCStatusOverlay.h"
#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "Player/AGP_PlayerControllerBase.h"
#include "Player/AGP_PlayerStateBase.h"
#include "Characters/AGP_PlayerCharacter.h"
#include "Characters/Components/AGPScanFieldObjectComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/AGPAttributeSetWarrior.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "AGPGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/AGPGameState_Survial.h"
#include "EnhancedInputSubsystems.h"
#include "Log/AGPLogChannels.h"


void UAGPWCStatusOverlay::InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	Super::InitializeWidgetController(InOwnerPlayerController);

	checkf(InOwnerPlayerController, TEXT("[%s], Invalid OwnerPlayerController"), __FUNCTIONW__);

	/* PlayerState 설정 델리게이트 바인딩 */
	InOwnerPlayerController->OnSetPlayerStateDelegate.AddUObject(this, &UAGPWCStatusOverlay::OnSetPlayerState);
	InOwnerPlayerController->OnResetPlayerStateDelegate.AddUObject(this, &UAGPWCStatusOverlay::OnResetPlayerState);

	/* 호출시점에 PlayerState 유효 확인 및 초기화 */
	if (InOwnerPlayerController->PlayerState)
	{
		OnSetPlayerState(InOwnerPlayerController->PlayerState);
	}

	/* Pawn의 Possess 이벤트 수신 */
	InOwnerPlayerController->OnPossessedPawnChanged.AddDynamic(this, &UAGPWCStatusOverlay::OnPossessedPawnChanged);
	AAGP_PlayerCharacter* OwnerCharacter = Cast<AAGP_PlayerCharacter>(InOwnerPlayerController->GetPawn());
	if (OwnerCharacter)
	{
		OnPossessOwnerCharacter(OwnerCharacter);
	}

	/* 플레이어 Input 이벤트 수신 처리 */
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(InOwnerPlayerController->GetLocalPlayer());
	if (InputSubsystem)
	{
		InputSubsystem->ControlMappingsRebuiltDelegate.AddUniqueDynamic(this, &UAGPWCStatusOverlay::OnControlMappingRebuiltEvent);
	}
}

void UAGPWCStatusOverlay::DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	Super::DeinitializeWidgetController(InOwnerPlayerController);

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(InOwnerPlayerController->GetLocalPlayer());
	if (InputSubsystem)
	{
		InputSubsystem->ControlMappingsRebuiltDelegate.RemoveAll(this);
	}

	OnHealthValueChanged.Clear();
	OnOwnerPlayerStateChanged.Clear();

	OnUnpossessOwnerCharacter();
	OnResetPlayerState();

	if (InOwnerPlayerController)
	{
		InOwnerPlayerController->OnPossessedPawnChanged.RemoveAll(this);
	}
}

void UAGPWCStatusOverlay::OnSetPlayerState(APlayerState* InOwnerPlayerState)
{
	UAGP_AbilitySystemComponent* OwnerASC = Cast<UAGP_AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InOwnerPlayerState));
	checkf(OwnerASC, TEXT("[%s], Invalid OwnerASC"), __FUNCTIONW__);

	/* PlayerState 상태 변경 */
	bPlayerStateValid = true;

	/* ASC 캐싱 */
	CachedOwnerASC = OwnerASC;

	//활성화 가능한 Ability중 Slot정보를 가지는 Ability 확인해서 추가
	FAGPForEachAbility SlotAbilityNotifyDelegate;
	SlotAbilityNotifyDelegate.BindLambda
	(
		[&](FGameplayAbilitySpec& AbilitySpec)
		{
			/* 해당 Ability에 지정된 SlotTag를 찾음 */
			const FGameplayTag& FindAbilitySlotTag = GetAbilitySlotTagFromSpec(AbilitySpec);
			if (FindAbilitySlotTag.IsValid())
			{
				CurrentHasSlotAbility.Add(FindAbilitySlotTag, AbilitySpec.Handle);
				OnAddedActivateSlotAbility(FindAbilitySlotTag, AbilitySpec);
			}
		}
	);
	OwnerASC->ForEachAbility_Execute(SlotAbilityNotifyDelegate);

	/* 이후 Ability의 추가/삭제에 대한 이벤트 바인딩 */
	OwnerASC->OnAbilityGranted.AddUObject(this, &UAGPWCStatusOverlay::OnGrantedAbility);
	OwnerASC->OnAbilityRemoved.AddUObject(this, &UAGPWCStatusOverlay::OnRemovedAbility);

	/* 현재 추가된 Custom AttributeSet 확인 */
	FAGPForEachAttributeSet ForEachAttributeSetDelegate;
	ForEachAttributeSetDelegate.BindLambda
	(
		[&](UAGPAttributeSetBase* InAttributeSet)
		{
			OnAddedAttributeSet(InAttributeSet);
		}
	);
	OwnerASC->ForEachAttributeSet_Execute(ForEachAttributeSetDelegate);

	/* Custom AttributeSet 변동 이벤트에 대한 델리게이트 설정 */
	OwnerASC->OnCustomAttributeSetAdded.AddUObject(this, &UAGPWCStatusOverlay::OnAddedAttributeSet);
	OwnerASC->OnCustomAttributeSetRemoved.AddUObject(this, &UAGPWCStatusOverlay::OnRemoveAttributeSet);

	/* CommonAttributeSet에 대한 처리 */
	const UAGPAttributeSetCommon* CommonAttribute = Cast<UAGPAttributeSetCommon>(OwnerASC->GetAttributeSet(UAGPAttributeSetCommon::StaticClass()));
	checkf(CommonAttribute, TEXT("[%s] - Invalid CommonAttribute"), __FUNCTIONW__);
	
	/* 현재, 최대 체력치 값 저장 */
	CurrentHealthValue = CommonAttribute->GetCurrentHealth();
	MaxHealthValue = CommonAttribute->GetMaxHealth();
	
	/* 현재 체력, 최대 체력 변동에 대한 델리게이트에 바인딩 */
	OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetCurrentHealthAttribute()).AddUObject(this, &UAGPWCStatusOverlay::OnChangeCurrentHealth);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetMaxHealthAttribute()).AddUObject(this, &UAGPWCStatusOverlay::OnChangeMaxHealth);

	/* 업데이트 된 현재 상태들 알림(초기화) */
	OnOwnerPlayerStateChanged.Broadcast(bPlayerStateValid);
	OnHealthValueChanged.Broadcast(CurrentHealthValue, MaxHealthValue);
}

void UAGPWCStatusOverlay::OnResetPlayerState()
{
	UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get();
	if (OwnerASC)
	{
		/* 기존 추가된 Custom AttributeSet에 대한 제거 작업 */
		FAGPForEachAttributeSet ForEachAttributeSetDelegate;
		ForEachAttributeSetDelegate.BindLambda
		(
			[&](UAGPAttributeSetBase* InAttributeSet)
			{
				OnRemoveAttributeSet(InAttributeSet);
			}
		);
		OwnerASC->ForEachAttributeSet_Execute(ForEachAttributeSetDelegate);

		OwnerASC->OnCustomAttributeSetAdded.RemoveAll(this);
		OwnerASC->OnCustomAttributeSetRemoved.RemoveAll(this);

		const UAGPAttributeSetCommon* CommonAttribute = Cast<UAGPAttributeSetCommon>(OwnerASC->GetAttributeSet(UAGPAttributeSetCommon::StaticClass()));
		if (CommonAttribute)
		{
			/* 현재 체력, 최대 체력 변동에 대한 델리게이트 언바인딩 */
			OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetCurrentHealthAttribute()).RemoveAll(this);
			OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetMaxHealthAttribute()).RemoveAll(this);
		}

		OwnerASC->OnAbilityGranted.RemoveAll(this);
		OwnerASC->OnAbilityRemoved.RemoveAll(this);

		CachedOwnerASC.Reset();
	}	

	/* 기존에 슬롯 지정되어있는 Ability 정보들 초기화 */
	for (TPair<FGameplayTag, FGameplayAbilitySpecHandle>& SlotAbility : CurrentHasSlotAbility)
	{
		if (FGameplayAbilitySpec* SlotAbilitySpec = GetAbilitySpecFromHandle(SlotAbility.Value))
		{
			OnRemovedActivateSlotAbility(SlotAbility.Key, *SlotAbilitySpec);
		}
		else
		{
			OnRemovedActivateSlotAbility(SlotAbility.Key);
		}
	}
	CurrentHasSlotAbility.Reset();

	/* PlayerState 상태 변경 */
	bPlayerStateValid = false;
	OnOwnerPlayerStateChanged.Broadcast(bPlayerStateValid);
}

void UAGPWCStatusOverlay::OnAddedAttributeSet(UAGPAttributeSetBase* InAddAttributeSet)
{
	checkf(InAddAttributeSet, TEXT("[%s], Invalid InAddAttributeSet"), __FUNCTIONW__);

	if (InAddAttributeSet->IsA<UAGPAttributeSetWarrior>())
	{
		OnAddedAttributeSet_Warrior(Cast<UAGPAttributeSetWarrior>(InAddAttributeSet));
	}
}

void UAGPWCStatusOverlay::OnRemoveAttributeSet(UAGPAttributeSetBase* InRemoveAttributeSet)
{
	checkf(InRemoveAttributeSet, TEXT("[%s], Invalid InAddAttributeSet"), __FUNCTIONW__);

	if (InRemoveAttributeSet->IsA<UAGPAttributeSetWarrior>())
	{
		OnRemoveAttributeSet_Warrior(Cast<UAGPAttributeSetWarrior>(InRemoveAttributeSet));
	}
}

void UAGPWCStatusOverlay::OnAddedAttributeSet_Warrior(UAGPAttributeSetWarrior* InAddAttributeSetWarrior)
{
	UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get();
	checkf(OwnerASC, TEXT("[%s], Invalid OwnerASC"), __FUNCTIONW__);

	/* WarriorAttributeSet에 대한 처리 */
	checkf(InAddAttributeSetWarrior, TEXT("[%s], Invalid InAddAttributeSetWarrior"), __FUNCTIONW__);

	/* 현재, 최대 분노 값 저장 */
	CurrentRageValue = InAddAttributeSetWarrior->GetCurrentRage();
	MaxRageValue = InAddAttributeSetWarrior->GetMaxRage();

	/* 분노 값 변동에 대한 이벤트 델리게이트 바인딩 */
	OwnerASC->GetGameplayAttributeValueChangeDelegate(InAddAttributeSetWarrior->GetCurrentRageAttribute()).AddUObject(this, &UAGPWCStatusOverlay::OnChangeCurrentRage);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(InAddAttributeSetWarrior->GetMaxRageAttribute()).AddUObject(this, &UAGPWCStatusOverlay::OnChangeMaxRage);

	/* 현재 값들 알림(초기화) */
	OnRageValueChanged.Broadcast(CurrentRageValue, MaxRageValue);

	//AGP_NET_LOG(this, LogAGPAttribute, Log, TEXT("Added Attribute Set - [%s], CurrentRage[%f], MaxRage[%f]"), *GetNameSafe(InAddAttributeSetWarrior), CurrentRageValue, MaxRageValue);
}

void UAGPWCStatusOverlay::OnRemoveAttributeSet_Warrior(UAGPAttributeSetWarrior* InRemoveAttributeSetWarrior)
{
	UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get();
	checkf(OwnerASC, TEXT("[%s], Invalid OwnerASC"), __FUNCTIONW__);

	/* 델리게이트 언바인딩 */
	OwnerASC->GetGameplayAttributeValueChangeDelegate(InRemoveAttributeSetWarrior->GetCurrentRageAttribute()).RemoveAll(this);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(InRemoveAttributeSetWarrior->GetMaxRageAttribute()).RemoveAll(this);

	//AGP_NET_LOG(this, LogAGPAttribute, Log, TEXT("Remove Attribute Set - [%s]"), *GetNameSafe(InRemoveAttributeSetWarrior));
}

void UAGPWCStatusOverlay::OnChangeCurrentHealth(const FOnAttributeChangeData& Data)
{
	CurrentHealthValue = Data.NewValue;

	OnHealthValueChanged.Broadcast(CurrentHealthValue, MaxHealthValue);
}

void UAGPWCStatusOverlay::OnChangeMaxHealth(const FOnAttributeChangeData& Data)
{
	MaxHealthValue = Data.NewValue;

	OnHealthValueChanged.Broadcast(CurrentHealthValue, MaxHealthValue);
}

void UAGPWCStatusOverlay::OnChangeCurrentRage(const FOnAttributeChangeData& Data)
{
	CurrentRageValue = Data.NewValue;

	OnRageValueChanged.Broadcast(CurrentRageValue, MaxRageValue);
}

void UAGPWCStatusOverlay::OnChangeMaxRage(const FOnAttributeChangeData& Data)
{
	MaxRageValue = Data.NewValue;

	OnRageValueChanged.Broadcast(CurrentRageValue, MaxRageValue);
}

void UAGPWCStatusOverlay::OnGrantedAbility(FGameplayAbilitySpec& InAbilitySpec)
{
	/* 추가된 Ability에 Slot이 지정되어있는지 확인해서 추가한다. */
	const FGameplayTag& FindAbilitySlotTag = GetAbilitySlotTagFromSpec(InAbilitySpec);
	if (FindAbilitySlotTag.IsValid())
	{
		CurrentHasSlotAbility.Add(FindAbilitySlotTag, InAbilitySpec.Handle);
		OnAddedActivateSlotAbility(FindAbilitySlotTag, InAbilitySpec);
	}
}

void UAGPWCStatusOverlay::OnRemovedAbility(FGameplayAbilitySpec& InAbilitySpec)
{
	/* 제거된 Ability에 Slot이 지정되어있는지 확인해서 제거처리. */
	const FGameplayTag& FindAbilitySlotTag = GetAbilitySlotTagFromSpec(InAbilitySpec);
	const FGameplayAbilitySpecHandle* FindAbilitySpecHandle = CurrentHasSlotAbility.Find(FindAbilitySlotTag);
	if (!FindAbilitySpecHandle)
	{
		return;
	}

	OnRemovedActivateSlotAbility(FindAbilitySlotTag, InAbilitySpec);
	CurrentHasSlotAbility.Remove(FindAbilitySlotTag);
}

void UAGPWCStatusOverlay::OnAddedActivateSlotAbility(const FGameplayTag& InAbilitySlotTag, const FGameplayAbilitySpec& InAbilitySpec)
{
	checkf(InAbilitySpec.Ability, TEXT("[%s], Invalid InAbilitySpec.Ability"), __FUNCTIONW__);

	const FGameplayTag& FindAbilityTag = GetAbilityTagFromSpec(InAbilitySpec);
	checkf(FindAbilityTag.IsValid(), TEXT("[%s], Invalid FindAbilityTag"), __FUNCTIONW__);

	UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get();
	checkf(OwnerASC, TEXT("[%s], Invalid OwnerASC"), __FUNCTIONW__);

	/* 슬롯에 Ability 추가되었음을 알림 */
	OnAddedSlotAbility.Broadcast(InAbilitySlotTag, FindAbilityTag);

	/* Cooldown 정보 */
	const FGameplayTag& CooldownTag = GetCooldownTagFromSpec(InAbilitySpec);
	if (CooldownTag.IsValid())
	{
		CooldownTagToAbilitySlot.Add(CooldownTag, InAbilitySlotTag);

		/* Cooldown Tag 이벤트 수신 확인 */
		OwnerASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAGPWCStatusOverlay::OnAbilityCooldownEventNotify);

		/* 현재 Cooldown 상태면 */
		if(IsCooldownProgress(CooldownTag))
		{
			//토탈시간, 남은시간등을 확인해서 Cooldown 상태임을 알림
			OnAbilityCooldownBegin(InAbilitySlotTag, CooldownTag);
		}
		else
		{
			//Cooldown상태가 아님을 알림
			OnAbilityCooldownEnd(InAbilitySlotTag);
		}
	}
	else
	{
		/* Cooldown이 유효하지 않음 */
		OnAbilityCooldownEnd(InAbilitySlotTag);
	}
}

void UAGPWCStatusOverlay::OnRemovedActivateSlotAbility(const FGameplayTag& InAbilitySlotTag)
{
	/* 슬롯에 추가된 Ability가 제거되었음을 알림 */
	OnRemovedSlotAbility.Broadcast(InAbilitySlotTag);
}

void UAGPWCStatusOverlay::OnRemovedActivateSlotAbility(const FGameplayTag& InAbilitySlotTag, const FGameplayAbilitySpec& InAbilitySpec)
{
	if (UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get())
	{
		/* Cooldown 이벤트 델리게이트 제거 */
		const FGameplayTag& CooldownTag = GetCooldownTagFromSpec(InAbilitySpec);
		if (CooldownTag.IsValid())
		{
			OwnerASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	OnRemovedActivateSlotAbility(InAbilitySlotTag);
}

const FGameplayTag& UAGPWCStatusOverlay::GetAbilityTagFromAbilitySlot(const FGameplayTag& SlotTag) const
{
	const FGameplayAbilitySpecHandle* FindAbilitySpecHandle = CurrentHasSlotAbility.Find(SlotTag);
	if (!FindAbilitySpecHandle)
	{
		return FGameplayTag::EmptyTag;
	}

	FGameplayAbilitySpec* FindAbilitySpec = GetAbilitySpecFromHandle(*FindAbilitySpecHandle);
	if (!FindAbilitySpec)
	{
		return FGameplayTag::EmptyTag;
	}

	return GetAbilityTagFromSpec(*FindAbilitySpec);
}

const FGameplayTag& UAGPWCStatusOverlay::GetAbilityCooldownTagFromAbilitySlot(const FGameplayTag& SlotTag) const
{
	const FGameplayAbilitySpecHandle* FindAbilitySpecHandle = CurrentHasSlotAbility.Find(SlotTag);
	if (!FindAbilitySpecHandle)
	{
		return FGameplayTag::EmptyTag;
	}

	FGameplayAbilitySpec* FindAbilitySpec = GetAbilitySpecFromHandle(*FindAbilitySpecHandle);
	if (!FindAbilitySpec)
	{
		return FGameplayTag::EmptyTag;
	}

	return GetCooldownTagFromSpec(*FindAbilitySpec);
}

bool UAGPWCStatusOverlay::GetCooldownTimeByTag(const FGameplayTag& InCooldownTag, float& OutRemainingTime, float& OutCooldownTotalTime) const
{
	if (!InCooldownTag.IsValid())
	{
		return false;
	}

	UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get();
	if (!OwnerASC)
	{
		return false;
	}

	FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(InCooldownTag.GetSingleTagContainer());
	TArray<TPair<float, float>> TimeRemainingAndDuration = OwnerASC->GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);

	if (TimeRemainingAndDuration.IsEmpty())
	{
		return false;
	}

	OutRemainingTime = TimeRemainingAndDuration[0].Key;
	OutCooldownTotalTime = TimeRemainingAndDuration[0].Value;

	return true;
}

bool UAGPWCStatusOverlay::IsCooldownProgress(const FGameplayTag& InCooldownTag) const
{
	if (!InCooldownTag.IsValid())
	{
		return false;
	}

	UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get();
	return OwnerASC ? OwnerASC->GetOwnedGameplayTags().HasTagExact(InCooldownTag) : false;
}

FGameplayAbilitySpec* UAGPWCStatusOverlay::GetAbilitySpecFromHandle(const FGameplayAbilitySpecHandle& InSpecHandle) const
{
	UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get();
	if (!OwnerASC)
	{
		return nullptr;
	}

	return OwnerASC->FindAbilitySpecFromHandle(InSpecHandle);
}

const FGameplayTag& UAGPWCStatusOverlay::GetAbilityTagFromSpec(const FGameplayAbilitySpec& InAbilitySpec) const
{
	if (InAbilitySpec.Ability)
	{
		const FGameplayTagContainer& AbilityTags = InAbilitySpec.Ability->GetAssetTags();
		for (const FGameplayTag& AbilityTag : AbilityTags)
		{
			/* Ability하위 태그정보를 찾음 */
			if (AbilityTag.MatchesTag(AGPGameplayTags::Ability))
			{
				return AbilityTag;
			}
		}
	}
	
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UAGPWCStatusOverlay::GetAbilitySlotTagFromSpec(const FGameplayAbilitySpec& InAbilitySpec) const
{
	const FGameplayTagContainer& DynamicAbilityTags = InAbilitySpec.GetDynamicSpecSourceTags();
	for (const FGameplayTag& AbilityTag : DynamicAbilityTags)
	{
		if (AbilityTag.MatchesTag(AGPGameplayTags::Ability_Slot))
		{
			return AbilityTag;
		}
	}

	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UAGPWCStatusOverlay::GetCooldownTagFromSpec(const FGameplayAbilitySpec& InAbilitySpec) const
{
	if (InAbilitySpec.Ability)
	{
		UGameplayEffect* CooldownGE = InAbilitySpec.Ability->GetCooldownGameplayEffect();
		if (CooldownGE)
		{
			const FGameplayTagContainer& CooldownGEGrantedTags = CooldownGE->GetGrantedTags();
			for (const FGameplayTag& GrantedTag : CooldownGEGrantedTags)
			{
				if (GrantedTag.MatchesTag(AGPGameplayTags::Cooldown))
				{
					return GrantedTag;
				}
			}
		}
	}

	return FGameplayTag::EmptyTag;
}

void UAGPWCStatusOverlay::OnAbilityCooldownEventNotify(const FGameplayTag Tag, int32 NewCount)
{
	const FGameplayTag* FindAbilitySlotTag = CooldownTagToAbilitySlot.Find(Tag);
	if (!FindAbilitySlotTag)
	{
		return;
	}

	if (NewCount > 0)
	{
		OnAbilityCooldownBegin(*FindAbilitySlotTag, Tag);
	}
	else
	{
		OnAbilityCooldownEnd(*FindAbilitySlotTag);
	}
}

void UAGPWCStatusOverlay::OnAbilityCooldownBegin(const FGameplayTag& InAbilitySlotTag, const FGameplayTag& InCooldownTag)
{
	/* 현재 Cooldown 남은시간 확인 */
	float CooldownRemainingTime = 0.f, CooldownTotalTime = 0.f;
	if (!GetCooldownTimeByTag(InCooldownTag, CooldownRemainingTime, CooldownTotalTime))
	{
		AGP_NET_LOG(this, LogAGPWidget, Warning, TEXT("Cooldown Time Invalid.. AbilitySlot[%s], Cooldown[%s]"), *InAbilitySlotTag.ToString(), *InCooldownTag.ToString());
		return;
	}

	OnAbilityCooldownBeginNotify.Broadcast(InAbilitySlotTag, CooldownRemainingTime, CooldownTotalTime);
}

void UAGPWCStatusOverlay::OnAbilityCooldownEnd(const FGameplayTag& InAbilitySlotTag)
{
	OnAbilityCooldownEndNotify.Broadcast(InAbilitySlotTag);
}

void UAGPWCStatusOverlay::OnPossessOwnerCharacter(AAGP_PlayerCharacter* InNewCharacter)
{
	checkf(InNewCharacter, TEXT("[%s], Invalid InNewCharacter"), __FUNCTIONW__);

	UAGPScanFieldObjectComponent* ScanFieldObjectComponent = InNewCharacter->GetScanFieldObjectComponent();
	checkf(ScanFieldObjectComponent, TEXT("[%s], Invalid ScanFieldObjectComponent"), __FUNCTIONW__);

	/*ScanFieldObjectComponent->OnChangeEnableConsumeStone.AddDynamic(this, &UAGPWCStatusOverlay::OnEnableConsumeStoneChanged);
	OnEnableConsumeStoneChanged(ScanFieldObjectComponent->IsEnableConsumeStone());*/
	ScanFieldObjectComponent->OnChangeCurrentInteractionMode.AddDynamic(this, &UAGPWCStatusOverlay::OnOwnerInteractionModeChanged);
	OnOwnerInteractionModeChanged(ScanFieldObjectComponent->GetCurrentInteractionMode());


	/* 현재 Boss 상태 조회 */
	FOnNotifyBossStatusInfoSignature::FDelegate NotifyBossStatusInfo;
	NotifyBossStatusInfo.BindLambda
	(
		[this](const FAGPBossStatusInfoCommon& BossStatusInfo)
		{
			OnAddedBossStatusInfo(BossStatusInfo);
		}
	);
	InNewCharacter->ForEachBossStatus_Execute(NotifyBossStatusInfo);

	/* Boss 추가 / 삭제 / 변동에 대한 델리게이트 바인딩 */
	InNewCharacter->OnAddedBossStatus.AddUObject(this, &UAGPWCStatusOverlay::OnAddedBossStatusInfo);
	InNewCharacter->OnRemoveBossStatus.AddUObject(this, &UAGPWCStatusOverlay::OnRemovedBossStatusInfo);
	InNewCharacter->OnModifyBossStatus.AddUObject(this, &UAGPWCStatusOverlay::OnModifyBossStatusInfo);

	CachedOwnerCharacter = InNewCharacter;
}

void UAGPWCStatusOverlay::OnUnpossessOwnerCharacter()
{
	AAGP_PlayerCharacter* OwnerCharacter = CachedOwnerCharacter.Get();
	if (OwnerCharacter)
	{
		UAGPScanFieldObjectComponent* ScanFieldObjectComponent = OwnerCharacter->GetScanFieldObjectComponent();
		if (ScanFieldObjectComponent)
		{
			//ScanFieldObjectComponent->OnChangeEnableConsumeStone.RemoveAll(this);
			ScanFieldObjectComponent->OnChangeCurrentInteractionMode.RemoveAll(this);
		}

		/* 기존 Boss 상태 초기화 및 Remove 호출 */
		TArray<TPair<int32, FAGPBossStatusInfoCommon>> TempBossStatusInfosForRemove = BossStatusInfoMap.Array();
		for (TPair<int32, FAGPBossStatusInfoCommon>& PairBossStatusInfo : TempBossStatusInfosForRemove)
		{
			const FAGPBossStatusInfoCommon& RemovedBossStatusInfo = PairBossStatusInfo.Value;
			OnRemovedBossStatusInfo(RemovedBossStatusInfo);
		}
		TempBossStatusInfosForRemove.Empty();

		OwnerCharacter->OnAddedBossStatus.RemoveAll(this);
		OwnerCharacter->OnRemoveBossStatus.RemoveAll(this);
		OwnerCharacter->OnModifyBossStatus.RemoveAll(this);
	}

	//OnEnableConsumeStoneChanged(false);
	CurrentInteractionMode = EAGPInteractionMode::None;
	OnOwnerInteractionModeChanged(CurrentInteractionMode);
	CachedOwnerCharacter.Reset();
}

void UAGPWCStatusOverlay::OnAddedBossStatusInfo(const FAGPBossStatusInfoCommon& AddedBossStatusInfo)
{
	//AGP_LOG(LogAGPAI, Log, TEXT("[%s]"), *AddedBossStatusInfo.ToString());
	FAGPBossStatusInfoCommon* FindBossStatusInfo = BossStatusInfoMap.Find(AddedBossStatusInfo.AIControllerUniqueID);
	if (FindBossStatusInfo)
	{
		AGP_LOG(LogAGPAI, Warning, TEXT("[%s], Already Added"), *AddedBossStatusInfo.ToString());
		return;
	}

	BossStatusInfoMap.Add(AddedBossStatusInfo.AIControllerUniqueID, AddedBossStatusInfo);
	OnNotifyAddedBossStatus.Broadcast(AddedBossStatusInfo);
}

void UAGPWCStatusOverlay::OnModifyBossStatusInfo(const FAGPBossStatusInfoCommon& ChangedBossStatusInfo)
{
	//AGP_LOG(LogAGPAI, Log, TEXT("[%s]"), *ChangedBossStatusInfo.ToString());

	FAGPBossStatusInfoCommon* FindBossStatusInfo = BossStatusInfoMap.Find(ChangedBossStatusInfo.AIControllerUniqueID);
	if (!FindBossStatusInfo)
	{
		return;
	}

	*FindBossStatusInfo = ChangedBossStatusInfo;
	OnNotifyModifyBossStatus.Broadcast(ChangedBossStatusInfo);
}

void UAGPWCStatusOverlay::OnRemovedBossStatusInfo(const FAGPBossStatusInfoCommon& RemovedBossStatusInfo)
{
	//AGP_LOG(LogAGPAI, Log, TEXT("[%s]"), *RemovedBossStatusInfo.ToString());

	const int32 RemovedNum = BossStatusInfoMap.Remove(RemovedBossStatusInfo.AIControllerUniqueID);
	if (RemovedNum > 0)
	{
		OnNotifyRemovedBossStatus.Broadcast(RemovedBossStatusInfo);
	}
}

void UAGPWCStatusOverlay::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (NewPawn)
	{
		AAGP_PlayerCharacter* OwnerCharacter = Cast<AAGP_PlayerCharacter>(NewPawn);
		if (OwnerCharacter)
		{
			OnPossessOwnerCharacter(OwnerCharacter);
		}
	}
	else
	{
		OnUnpossessOwnerCharacter();
	}
}

void UAGPWCStatusOverlay::OnOwnerInteractionModeChanged(EAGPInteractionMode InInteractionMode)
{
	CurrentInteractionMode = InInteractionMode;
	OnNotifyChangeInteractionMode.Broadcast(CurrentInteractionMode);
}

void UAGPWCStatusOverlay::OnControlMappingRebuiltEvent()
{
	OnNotifyControlMappingsRebuilt.Broadcast();
}