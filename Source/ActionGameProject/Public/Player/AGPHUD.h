// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Common/AGPCommonStructs.h"
#include "AGPHUD.generated.h"

struct FAGPSurvialGameModeContext;
class UAGPUserWidget;
class UAGPWCStatusOverlay;
class UAGPGameEndWidgetController;
class UAGPWCIngameMenu;

/**
 * 
 */

UCLASS()
class ACTIONGAMEPROJECT_API AAGPHUD : public AHUD
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAGPUserWidget> StatusOverlayWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAGPUserWidget> GameEndWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAGPUserWidget> IngameMenuWidgetClass;
	
private:
	/* Status Overlay에 대한 Widget Controller */
	UPROPERTY()
	TObjectPtr<UAGPWCStatusOverlay> WC_StatusOverlay;

	UPROPERTY()
	TObjectPtr<UAGPUserWidget> StatusOverlayWidget;

	/* GameEnd Widget Controller*/
	UPROPERTY()
	TObjectPtr<UAGPGameEndWidgetController> WC_GameEnd;

	UPROPERTY()
	TObjectPtr<UAGPUserWidget> GameEndWidget;

	/* Ingame Menu Widget Controller */
	UPROPERTY()
	TObjectPtr<UAGPWCIngameMenu> WC_IngameMenu;

	UPROPERTY()
	TObjectPtr<UAGPUserWidget> IngameMenuWidget;

private:
	/* 현재 Survival Mode 상태 */
	UPROPERTY()
	FAGPSurvialGameModeContext SurvivalModeContext;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void ToggleIngameMenu();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnChangedCurrentSurvivalMode(const FAGPSurvialGameModeContext& CurrentContext);

private:
	void SetInputModeAndCursor(const FInputModeDataBase& InputMode, bool bShowCursor);

private:
	UFUNCTION()
	void OnChangeSurvialModeState(const FAGPSurvialGameModeContext& CurrentSurvialModeState);
};
