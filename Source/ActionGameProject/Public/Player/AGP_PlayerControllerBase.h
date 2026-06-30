// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AGP_PlayerControllerBase.generated.h"

class UInputAction;
class UAGPInputConfig;
class UAGPInputComponent;
class UAGP_AbilitySystemComponent;
struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE_OneParam(FSetPlayerStateOnControllerSignature, APlayerState* /*InCurrentPlayerState*/);
DECLARE_MULTICAST_DELEGATE(FResetPlayerStateOnControllerSignature);


/**
 * PlayerController Base Class
 */

UCLASS()
class ACTIONGAMEPROJECT_API AAGP_PlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
	
#pragma region [Inputs]
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAGPInputConfig> BaseInputConfig;

protected:
	TWeakObjectPtr<UAGPInputComponent> CachedPlayerInputComponent;
	TArray<uint32> BindingBaseInputHandles;
#pragma endregion	

protected:
	/* 타겟잠금 시 적용될 Pitch Offset */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock")
	float TargetLockPitchOffsetDistance = 20.f;

	/* 타겟잠금 시 적용될 회전보간속도 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock")
	float TargetLockRotationInterpSpeed = 5.f;

public:
	/* PlayerState 설정 시 호출되는 델리게이트 */
	FSetPlayerStateOnControllerSignature OnSetPlayerStateDelegate;

	/* 설정된 PlayerState 초기화 시 호출되는 델리게이트 */
	FResetPlayerStateOnControllerSignature OnResetPlayerStateDelegate;
	
protected:
	virtual void SetupInputComponent() override;

public:
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	virtual void UpdateRotation(float DeltaTime) override;


#pragma region [Input Functions]
public:
	void BindInputConfig(UAGPInputConfig* InInputConfig, TArray<uint32>& OutBindingHandles);
	void UnbindInputConfig(UAGPInputConfig* InInputConfig, const TArray<uint32>& InBindingHandles);

protected:
	void OnNativeActionInput_Started(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnNativeActionInput_Triggered(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnNativeActionInput_Completed(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnNativeActionInput_Ongoning(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnNativeActionInput_Canceled(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);

	void OnAbilityInput_Started(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnAbilityInput_Triggered(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnAbilityInput_Completed(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnAbilityInput_Ongoning(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);
	void OnAbilityInput_Canceled(const FInputActionValue& InputActionValue, FGameplayTag InInputTag);

	void OnInput_MoveHeld(const FInputActionValue& InputActionValue);
	void OnInput_LookHeld(const FInputActionValue& InputActionValue);
	void OnInput_IngameMenu(const FInputActionValue& InputActionValue);
#pragma endregion

private:
	UAGP_AbilitySystemComponent* GetASCFromPawn() const;

	/* Possess된 Pawn의 타겟잠금여부 */
	bool IsTaretLockMovePawn() const;

	/* Possess된 Pawn의 타겟잠금 대상위치 반환 */
	FVector GetTargetLockPositionFromPawn() const;

	/* Possess된 Pawn의 위치 반환 */
	FVector GetPossessPawnLocation() const;
};
