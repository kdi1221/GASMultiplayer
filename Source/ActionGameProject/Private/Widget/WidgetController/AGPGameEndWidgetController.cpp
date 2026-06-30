// KJY All Rights Reserved


#include "Widget/WidgetController/AGPGameEndWidgetController.h"
#include "Player/AGP_PlayerControllerBase.h"

void UAGPGameEndWidgetController::InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	Super::InitializeWidgetController(InOwnerPlayerController);

	OwnerPlayerController = InOwnerPlayerController;
}

void UAGPGameEndWidgetController::DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	Super::DeinitializeWidgetController(InOwnerPlayerController);

	OwnerPlayerController.Reset();
}

void UAGPGameEndWidgetController::ReturnToMainMenu()
{
	AAGP_PlayerControllerBase* OwnerPC = OwnerPlayerController.Get();
	if (!OwnerPC)
	{
		return;
	}

	OwnerPC->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("GameExit. Player Select ToMainMenu")));
}