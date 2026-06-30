// KJY All Rights Reserved


#include "Input/AGPInputComponent.h"


void UAGPInputComponent::UnbindingInputMapping(const UAGPInputConfig* InInputConfig, const TArray<uint32>& InBindingHandles, APlayerController* InPlayerController)
{
	verifyf(InInputConfig, TEXT("Input config invalid, can not proceed with binding"));
	verifyf(InPlayerController, TEXT("PlayerController invalid, can not proceed with unbinding"));

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(InPlayerController->GetLocalPlayer());
	verifyf(Subsystem, TEXT("Invalid EnhancedInput Subsystem, can not Input Unbinding"));

	for (const uint32 BindingHandle : InBindingHandles)
	{
		RemoveBindingByHandle(BindingHandle);
	}

	Subsystem->RemoveMappingContext(InInputConfig->MappingContext);
}