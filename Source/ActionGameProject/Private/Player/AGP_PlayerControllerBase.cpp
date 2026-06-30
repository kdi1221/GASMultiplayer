// KJY All Rights Reserved


#include "Player/AGP_PlayerControllerBase.h"
#include "Input/AGPInputConfig.h"
#include "Input/AGPInputComponent.h"
#include "InputMappingContext.h"
#include "Characters/AGP_CharacterBase.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "AGPGameplayTags.h"
#include "Player/AGP_PlayerStateBase.h"
#include "Player/AGPHUD.h"
#include "Kismet/KismetMathLibrary.h"
#include "Log/AGPLogChannels.h"

void AAGP_PlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	CachedPlayerInputComponent = CastChecked<UAGPInputComponent>(InputComponent);

	/* 기본 입력 설정 */
	BindingBaseInputHandles.Empty();
	BindInputConfig(BaseInputConfig, BindingBaseInputHandles);	
}

void AAGP_PlayerControllerBase::InitPlayerState()
{
	Super::InitPlayerState();

	if (GetNetMode() != NM_Client)
	{
		OnSetPlayerStateDelegate.Broadcast(PlayerState);
	}
}

void AAGP_PlayerControllerBase::CleanupPlayerState()
{
	OnResetPlayerStateDelegate.Broadcast();

	Super::CleanupPlayerState();
}

void AAGP_PlayerControllerBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (PlayerState)
	{
		OnSetPlayerStateDelegate.Broadcast(PlayerState);
	}
	else
	{
		OnResetPlayerStateDelegate.Broadcast();
	}
}

void AAGP_PlayerControllerBase::UpdateRotation(float DeltaTime)
{
	if (IsTaretLockMovePawn())
	{
		/* 타겟잠금상태에서는 타겟을 향해 회전하도록 함 */
		const FVector TargetLockLocation = GetTargetLockPositionFromPawn();
		const FVector PossessPawnLocation = GetPossessPawnLocation();
		const FRotator CurrentRotation = GetControlRotation();
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(PossessPawnLocation, TargetLockLocation);

		/* Pitch Offset 적용 */
		LookAtRot -= FRotator(TargetLockPitchOffsetDistance, 0.f, 0.f);
		const FRotator TargetRotation = FMath::RInterpTo(CurrentRotation, LookAtRot, DeltaTime, TargetLockRotationInterpSpeed);

		SetControlRotation(FRotator(TargetRotation.Pitch, TargetRotation.Yaw, 0.f));

		/* 입력된 RotationInput은 무시한다. */
		RotationInput = FRotator::ZeroRotator;

		return;
	}

	Super::UpdateRotation(DeltaTime);
}

void AAGP_PlayerControllerBase::BindInputConfig(UAGPInputConfig* InInputConfig, TArray<uint32>& OutBindingHandles)
{
	static const FAGPInputBindFunctions<ThisClass> NativeInputBindFunctions(
		{
			{ETriggerEvent::Started, &ThisClass::OnNativeActionInput_Started},
			{ETriggerEvent::Triggered, &ThisClass::OnNativeActionInput_Triggered},
			{ETriggerEvent::Completed, &ThisClass::OnNativeActionInput_Completed},
			{ETriggerEvent::Ongoing, &ThisClass::OnNativeActionInput_Ongoning},
			{ETriggerEvent::Canceled, &ThisClass::OnNativeActionInput_Canceled},
		}
		);

	static const FAGPInputBindFunctions<ThisClass> AbilityInputBindFunctions(
		{
			{ETriggerEvent::Started, &ThisClass::OnAbilityInput_Started},
			{ETriggerEvent::Triggered, &ThisClass::OnAbilityInput_Triggered},
			{ETriggerEvent::Completed, &ThisClass::OnAbilityInput_Completed},
			{ETriggerEvent::Ongoing, &ThisClass::OnAbilityInput_Ongoning},
			{ETriggerEvent::Canceled, &ThisClass::OnAbilityInput_Canceled},
		}
		);

	UAGPInputComponent* AGPInputComponent = CachedPlayerInputComponent.Get();
	verifyf(AGPInputComponent, TEXT("Bind Input, Input Component Invalid"));

	AGPInputComponent->BindingInputConfig(InInputConfig, this, this, NativeInputBindFunctions, AbilityInputBindFunctions, OutBindingHandles);
}

void AAGP_PlayerControllerBase::UnbindInputConfig(UAGPInputConfig* InInputConfig, const TArray<uint32>& InBindingHandles)
{
	UAGPInputComponent* AGPInputComponent = CachedPlayerInputComponent.Get();
	verifyf(AGPInputComponent, TEXT("Bind Input, Input Component Invalid"));

	AGPInputComponent->UnbindingInputMapping(InInputConfig, InBindingHandles, this);
}

void AAGP_PlayerControllerBase::OnNativeActionInput_Started(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{

}

void AAGP_PlayerControllerBase::OnNativeActionInput_Triggered(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{
	if (InInputTag == AGPGameplayTags::Input_Native_Move)
	{
		OnInput_MoveHeld(InputActionValue);
	}
	else if (InInputTag == AGPGameplayTags::Input_Native_Look)
	{
		OnInput_LookHeld(InputActionValue);
	}
	else if (InInputTag == AGPGameplayTags::Input_Native_IngameMenu)
	{
		OnInput_IngameMenu(InputActionValue);
	}
}

void AAGP_PlayerControllerBase::OnNativeActionInput_Completed(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{

}

void AAGP_PlayerControllerBase::OnNativeActionInput_Ongoning(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{

}

void AAGP_PlayerControllerBase::OnNativeActionInput_Canceled(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{

}

void AAGP_PlayerControllerBase::OnAbilityInput_Started(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{
	if (UAGP_AbilitySystemComponent* ASC = GetASCFromPawn())
	{
		ASC->OnAbilityInput_Started(InputActionValue, InInputTag);
	}
}

void AAGP_PlayerControllerBase::OnAbilityInput_Triggered(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{
	if (UAGP_AbilitySystemComponent* ASC = GetASCFromPawn())
	{
		ASC->OnAbilityInput_Triggered(InputActionValue, InInputTag);
	}
}

void AAGP_PlayerControllerBase::OnAbilityInput_Completed(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{
	if (UAGP_AbilitySystemComponent* ASC = GetASCFromPawn())
	{
		ASC->OnAbilityInput_Completed(InputActionValue, InInputTag);
	}
}

void AAGP_PlayerControllerBase::OnAbilityInput_Ongoning(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{
	if (UAGP_AbilitySystemComponent* ASC = GetASCFromPawn())
	{
		ASC->OnAbilityInput_Ongoning(InputActionValue, InInputTag);
	}
}

void AAGP_PlayerControllerBase::OnAbilityInput_Canceled(const FInputActionValue& InputActionValue, FGameplayTag InInputTag)
{
	if (UAGP_AbilitySystemComponent* ASC = GetASCFromPawn())
	{
		ASC->OnAbilityInput_Canceled(InputActionValue, InInputTag);
	}
}

void AAGP_PlayerControllerBase::OnInput_MoveHeld(const FInputActionValue& InputActionValue)
{
	APawn* ControllerPawn = GetPawn();
	if (!ControllerPawn)
	{
		return;
	}

	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.f, GetControlRotation().Yaw, 0.f);

	if (MovementVector.Y != 0.f)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);

		ControllerPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
	}

	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);

		ControllerPawn->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AAGP_PlayerControllerBase::OnInput_LookHeld(const FInputActionValue& InputActionValue)
{
	APawn* ControllerPawn = GetPawn();
	if (!ControllerPawn)
	{
		return;
	}

	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (LookAxisVector.X != 0.f)
	{
		ControllerPawn->AddControllerYawInput(LookAxisVector.X);
	}

	if (LookAxisVector.Y != 0.f)
	{
		ControllerPawn->AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAGP_PlayerControllerBase::OnInput_IngameMenu(const FInputActionValue& InputActionValue)
{
	AAGPHUD* CurrentHUD = GetHUD<AAGPHUD>();
	if (!CurrentHUD)
	{
		return;
	}

	CurrentHUD->ToggleIngameMenu();
}

UAGP_AbilitySystemComponent* AAGP_PlayerControllerBase::GetASCFromPawn() const
{
	if (AAGP_CharacterBase* ControllerCharacter = GetPawn<AAGP_CharacterBase>())
	{
		return ControllerCharacter->GetAGPAbilitySystemComponent();
	}

	return nullptr;
}

bool AAGP_PlayerControllerBase::IsTaretLockMovePawn() const
{
	AAGP_CharacterBase* PossessPawn = GetPawn<AAGP_CharacterBase>();
	return PossessPawn ? PossessPawn->IsTargetLockMove() : false;
}

FVector AAGP_PlayerControllerBase::GetTargetLockPositionFromPawn() const
{
	AAGP_CharacterBase* PossessPawn = GetPawn<AAGP_CharacterBase>();
	return PossessPawn ? PossessPawn->GetTargetLockLocation() : FVector::ZeroVector;
}

FVector AAGP_PlayerControllerBase::GetPossessPawnLocation() const
{
	APawn* PossessPawn = GetPawn();
	return PossessPawn ? PossessPawn->GetActorLocation() : FVector::ZeroVector;
}
