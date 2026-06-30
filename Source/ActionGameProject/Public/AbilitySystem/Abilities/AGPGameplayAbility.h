// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Common/AGPCommonEnums.h"
#include "AGPGameplayAbility.generated.h"

class UAGP_AbilitySystemComponent;
class UAGPCharacterMovementComponent;
class AAGP_PlayerControllerBase;

/**
 * 각 GameplayAbility의 Base Class
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, 
									const FGameplayAbilityActorInfo* ActorInfo, 
									const FGameplayTagContainer* SourceTags = nullptr, 
									const FGameplayTagContainer* TargetTags = nullptr, 
									OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, 
								const FGameplayAbilitySpec& Spec) override;

	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, 
								const FGameplayAbilitySpec& Spec) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								const FGameplayAbilityActivationInfo ActivationInfo,
								bool bReplicateCancelAbility) override;

protected:
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, 
							const FGameplayAbilityActorInfo* ActorInfo, 
							const FGameplayAbilityActivationInfo ActivationInfo, 
							FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, 
							const FGameplayEventData* TriggerEventData = nullptr);

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								const FGameplayAbilityActivationInfo ActivationInfo,
								const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
							const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility,
							bool bWasCancelled) override;

#pragma region[Input Notify]
public:
	virtual void OnNotifyInputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void OnNotifyInputTriggered(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void OnNotifyInputOngoning(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void OnNotifyInputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void OnNotifyInputCanceled(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
#pragma endregion

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "AGP|Ability", DisplayName = "PreActivateAbility", meta = (ScriptName = "PreActivateAbility"))
	void K2_PreActivate();

	UFUNCTION(BlueprintImplementableEvent, Category = "AGP|Ability", DisplayName = "OnInputPressed", meta = (ScriptName = "OnInputPressed"))
	void K2_OnInputPressed(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);

	UFUNCTION(BlueprintImplementableEvent, Category = "AGP|Ability", DisplayName = "OnInputReleased", meta = (ScriptName = "OnInputReleased"))
	void K2_OnInputReleased(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);

	UFUNCTION(BlueprintImplementableEvent, Category = "AGP|Ability", DisplayName = "OnInputTriggered", meta = (ScriptName = "OnInputTriggered"))
	void K2_OnInputTriggered(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);

	UFUNCTION(BlueprintImplementableEvent, Category = "AGP|Ability", DisplayName = "OnInputOngoning", meta = (ScriptName = "OnInputOngoning"))
	void K2_OnInputOngoning(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);

	UFUNCTION(BlueprintImplementableEvent, Category = "AGP|Ability", DisplayName = "OnInputCanceled", meta = (ScriptName = "OnInputCanceled"))
	void K2_OnInputCanceled(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);

protected:
	UFUNCTION(BlueprintCallable)
	void InvokeReplicatedEvent_InputPressed(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo);

	UFUNCTION(BlueprintCallable)
	void InvokeReplicatedEvent_InputReleased(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActivationInfo& ActivationInfo);

protected:
	UFUNCTION(BlueprintCallable, DisplayName = "MakeEffectContextHandleAbility", meta = (ScriptName = "MakeEffectContextHandleAbility"))
	FGameplayEffectContextHandle BP_MakeEffectContextHandleAbility() const;

	/* Target 대상으로 해당 GameplayEffectSpec 적용(블루프린트 호출) */
	UFUNCTION(BlueprintCallable, DisplayName = "ApplyGESpecHandleToTargetAbility", meta = (ExpandEnumAsExecs = "OutExecutionResult"))
	FActiveGameplayEffectHandle BP_ApplyGESpecHandleToTargetAbility(AActor* InTarget, const FGameplayEffectSpecHandle& InSpecHandle, EAGPExecutionResult& OutExecutionResult);

protected:
	UFUNCTION(BlueprintPure)
	UAGP_AbilitySystemComponent* GetAGPAbilitySystemFromActorInfo() const;

	UFUNCTION(BlueprintPure)
	bool IsActiveAbility() const;

	UFUNCTION(BlueprintPure)
	AController* GetAvatarController() const;

	UFUNCTION(BlueprintPure)
	AAGP_PlayerControllerBase* GetAvatarPlayerController() const;

	UFUNCTION(BlueprintPure)
	AAGP_CharacterBase* GetAvatarCharacter() const;

	UFUNCTION(BlueprintPure)
	UAGPCharacterMovementComponent* GetAvatarMovementComponent() const;

	/* Avatar Controller 구분 (플레이어 / AI) */
	UFUNCTION(BlueprintPure)
	bool IsAvatarPlayerController() const;

protected:
	/* Target 대상으로 해당 GameplayEffectSpec 적용 */
	FActiveGameplayEffectHandle ApplyGESpecHandleToTargetAbility_Implementation(AActor* InTarget, const FGameplayEffectSpecHandle& InSpecHandle);
};
