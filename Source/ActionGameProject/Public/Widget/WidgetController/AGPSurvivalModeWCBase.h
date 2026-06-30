// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widget/WidgetController/AGPWidgetController.h"
#include "Common/AGPCommonStructs.h"
#include "AGPSurvivalModeWCBase.generated.h"

class AAGPGameState_Survial;
class AAGP_PlayerControllerBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotifySurvialModeChangeSignature, const FAGPSurvialGameModeContext&, CurrentContext);

/**
 * Survial Mode로 진행되는 게임에서 표시되는 위젯들의 컨트롤러 Base Class
 */

UCLASS(Abstract, BlueprintType)
class ACTIONGAMEPROJECT_API UAGPSurvivalModeWCBase : public UAGPWidgetController
{
	GENERATED_BODY()
	
private:
	/* SurvialGameMode - 현재 상태 */
	UPROPERTY()
	FAGPSurvialGameModeContext CurrentSurvialModeContext;

public:
	/* SurvialGameMode - 현재 상태 업데이트 */
	UPROPERTY(BlueprintAssignable)
	FOnNotifySurvialModeChangeSignature OnNotifySurvialModeContext;

public:
	virtual void InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController);
	virtual void DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController);

private:
	/* Survial GameState 설정될때 호출 */
	void OnSetSurvialGameState(AAGPGameState_Survial* InSurvialGameState);

private:
	/* World에서 GameState 설정될때 호출 */
	UFUNCTION()
	void OnGameStateSet(AGameStateBase* const GameStateBase);

	/* Survial GameMode - 상태 전환시 호출 */
	UFUNCTION()
	void OnChangeSurvialGameModeContext(const FAGPSurvialGameModeContext& NewContext);

public:
	UFUNCTION(BlueprintPure)
	const FAGPSurvialGameModeContext& GetCurrentSurvialMoeContext() const { return CurrentSurvialModeContext; }
};
