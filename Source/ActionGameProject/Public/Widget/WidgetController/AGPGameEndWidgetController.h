// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPSurvivalModeWCBase.h"
#include "AGPGameEndWidgetController.generated.h"

/**
 * 게임 종료 위젯 컨트롤러
 */
UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPGameEndWidgetController : public UAGPSurvivalModeWCBase
{
	GENERATED_BODY()

private:
	TWeakObjectPtr<AAGP_PlayerControllerBase> OwnerPlayerController;

public:
	virtual void InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController) override;
	virtual void DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController) override;

public:
	UFUNCTION(BlueprintCallable)
	void ReturnToMainMenu();
};
