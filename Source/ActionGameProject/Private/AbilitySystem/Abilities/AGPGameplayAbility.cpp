// KJY All Rights Reserved


#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "Log/AGPLogChannels.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayPrediction.h"
#include "Characters/AGP_CharacterBase.h"
#include "Player/AGP_PlayerControllerBase.h"

bool UAGPGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	const bool Result = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("%s, Result[%s], OwnerTag[%s]"), *GetNameSafe(this), (Result ? TEXT("TRUE") : (TEXT("FALSE"))), *ActorInfo->AbilitySystemComponent->GetOwnedGameplayTags().ToString());

	return Result;
}

void UAGPGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("%s"), *GetNameSafe(this));
}

void UAGPGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("%s"), *GetNameSafe(this));
}

void UAGPGameplayAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	//const FPredictionKey& ActivationPredictionKey = ActivationInfo.GetActivationPredictionKey();
	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("%s, PredictionKey[%s]"), *GetNameSafe(this), *ActivationPredictionKey.ToString());
}

void UAGPGameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	//const FPredictionKey& ActivationPredictionKey = ActivationInfo.GetActivationPredictionKey();
	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("%s, PredictionKey[%s]"), *GetNameSafe(this), *ActivationPredictionKey.ToString());

	K2_PreActivate();
}

void UAGPGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//const FPredictionKey& ActivationPredictionKey = ActivationInfo.GetActivationPredictionKey();
	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("%s, PredictionKey[%s]"), *GetNameSafe(this), *ActivationPredictionKey.ToString());
}

void UAGPGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	/*const FPredictionKey& ActivationPredictionKey = ActivationInfo.GetActivationPredictionKey();
	AGP_NET_LOG(this, LogAGPGamePlayAbility, Log,
		TEXT("%s, bReplicateEndAbility[%s], bWasCancelled[%s], ActivationMode[%s], OwnerTag[%s], PredictionKey[%s]"), 
		*GetNameSafe(this), 
		(bReplicateEndAbility ? TEXT("TRUE") : TEXT("FALSE")), 
		(bWasCancelled ? TEXT("TRUE") : TEXT("FALSE")),
		*UEnum::GetDisplayValueAsText(ActivationInfo.ActivationMode).ToString(), 
		*ActorInfo->AbilitySystemComponent->GetOwnedGameplayTags().ToString(),
		*ActivationPredictionKey.ToString());*/
}

void UAGPGameplayAbility::OnNotifyInputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	K2_OnInputPressed(Handle, ActivationInfo, InputActionValue, InInputTag);
}

void UAGPGameplayAbility::OnNotifyInputTriggered(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	K2_OnInputTriggered(Handle, ActivationInfo, InputActionValue, InInputTag);
}

void UAGPGameplayAbility::OnNotifyInputOngoning(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	K2_OnInputOngoning(Handle, ActivationInfo, InputActionValue, InInputTag);
}

void UAGPGameplayAbility::OnNotifyInputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	K2_OnInputReleased(Handle, ActivationInfo, InputActionValue, InInputTag);
}

void UAGPGameplayAbility::OnNotifyInputCanceled(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	K2_OnInputCanceled(Handle, ActivationInfo, InputActionValue, InInputTag);
}




void UAGPGameplayAbility::InvokeReplicatedEvent_InputPressed(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Handle, ActivationInfo.GetActivationPredictionKey());
}

void UAGPGameplayAbility::InvokeReplicatedEvent_InputReleased(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Handle, ActivationInfo.GetActivationPredictionKey());
}

FGameplayEffectContextHandle UAGPGameplayAbility::BP_MakeEffectContextHandleAbility() const
{
	checkf(CurrentActorInfo, TEXT("[%s], Invalid CurrentActorInfo"), __FUNCTIONW__);

	return MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
}

FActiveGameplayEffectHandle UAGPGameplayAbility::BP_ApplyGESpecHandleToTargetAbility(AActor* InTarget, const FGameplayEffectSpecHandle& InSpecHandle, EAGPExecutionResult& OutExecutionResult)
{
	FActiveGameplayEffectHandle ApplyGameplayEffectHandle = ApplyGESpecHandleToTargetAbility_Implementation(InTarget, InSpecHandle);

	OutExecutionResult = ApplyGameplayEffectHandle.WasSuccessfullyApplied() ? EAGPExecutionResult::Success : EAGPExecutionResult::Fail;

	return ApplyGameplayEffectHandle;
}

UAGP_AbilitySystemComponent* UAGPGameplayAbility::GetAGPAbilitySystemFromActorInfo() const
{
	return Cast<UAGP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

bool UAGPGameplayAbility::IsActiveAbility() const
{
	return IsActive();
}

AController* UAGPGameplayAbility::GetAvatarController() const
{
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	return AvatarPawn ? AvatarPawn->GetController() : nullptr;
}

AAGP_PlayerControllerBase* UAGPGameplayAbility::GetAvatarPlayerController() const
{
	return Cast<AAGP_PlayerControllerBase>(GetAvatarController());
}

AAGP_CharacterBase* UAGPGameplayAbility::GetAvatarCharacter() const
{
	return Cast<AAGP_CharacterBase>(GetAvatarActorFromActorInfo());
}

UAGPCharacterMovementComponent* UAGPGameplayAbility::GetAvatarMovementComponent() const
{
	AAGP_CharacterBase* AvatarCharacter = Cast<AAGP_CharacterBase>(GetAvatarActorFromActorInfo());	
	return AvatarCharacter ? AvatarCharacter->GetCharacterMovementComponent() : nullptr;
}

/* Avatar Controller 구분 (플레이어 / AI) */
bool UAGPGameplayAbility::IsAvatarPlayerController() const
{
	AController* AvatarController = GetAvatarController();
	if (!AvatarController)
	{
		return false;
	}

	return AvatarController->IsPlayerController();
}

FActiveGameplayEffectHandle UAGPGameplayAbility::ApplyGESpecHandleToTargetAbility_Implementation(AActor* InTarget, const FGameplayEffectSpecHandle& InSpecHandle)
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InTarget);

	checkf(SourceASC, TEXT("[%s], Invalid SourceASC"), __FUNCTIONW__);
	checkf(TargetASC, TEXT("[%s], Invalid TargetASC"), __FUNCTIONW__);
	checkf(InSpecHandle.IsValid(), TEXT("[%s], Invalid SpecHandle"), __FUNCTIONW__);

	return SourceASC->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data, TargetASC);
}
