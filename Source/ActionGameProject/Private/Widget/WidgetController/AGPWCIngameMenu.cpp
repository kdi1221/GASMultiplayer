// KJY All Rights Reserved


#include "Widget/WidgetController/AGPWCIngameMenu.h"
#include "Player/AGP_PlayerControllerBase.h"

void UAGPWCIngameMenu::InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	OwnerPlayerController = InOwnerPlayerController;
}

void UAGPWCIngameMenu::DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	OwnerPlayerController.Reset();
}

void UAGPWCIngameMenu::SetInGameMenuShow(bool bShow)
{
	const bool BeforeShowMenu = bShowMenu;
	bShowMenu = bShow;

	if (BeforeShowMenu != bShowMenu)
	{
		OnChangedIngameMenuShowFlag.Broadcast(bShowMenu);
	}
}

void UAGPWCIngameMenu::ReturnToMainMenu()
{
	AAGP_PlayerControllerBase* OwnerPC = OwnerPlayerController.Get();
	if (!OwnerPC)
	{
		return;
	}

	OwnerPC->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("Player Select ToMainMenu")));
}