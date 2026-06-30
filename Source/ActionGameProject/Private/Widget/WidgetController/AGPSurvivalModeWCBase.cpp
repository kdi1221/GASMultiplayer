// KJY All Rights Reserved


#include "Widget/WidgetController/AGPSurvivalModeWCBase.h"
#include "GameMode/AGPGameState_Survial.h"


void UAGPSurvivalModeWCBase::InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	UWorld* CurrentWorld = GetWorld();
	checkf(CurrentWorld, TEXT("[%s], Invalid CurrentWorld"), __FUNCTIONW__);
	CurrentWorld->GameStateSetEvent.AddUObject(this, &UAGPSurvivalModeWCBase::OnGameStateSet);

	/* 현재 게임상태 정보 확인(웨이브, 단계 등) 및 변경에 대한 델리게이트 바인딩 */
	if (AAGPGameState_Survial* CurrentGameState = CurrentWorld->GetGameState<AAGPGameState_Survial>())
	{
		OnSetSurvialGameState(CurrentGameState);
	}
}

void UAGPSurvivalModeWCBase::DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController)
{
	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld)
	{
		CurrentWorld->GameStateSetEvent.RemoveAll(this);

		if (AAGPGameState_Survial* GameStateSurvial = CurrentWorld->GetGameState<AAGPGameState_Survial>())
		{
			GameStateSurvial->OnChangeSurvialModeState.RemoveAll(this);
		}
	}
}

void UAGPSurvivalModeWCBase::OnSetSurvialGameState(AAGPGameState_Survial* InSurvialGameState)
{
	//GameState의 SurvialGameMode Context 변경에 대한 이벤트 델리게이트 바인딩
	InSurvialGameState->OnChangeSurvialModeState.AddUniqueDynamic(this, &UAGPSurvivalModeWCBase::OnChangeSurvialGameModeContext);

	//현재 SurvialGameMode Context에 대한 변경사항 적용
	OnChangeSurvialGameModeContext(InSurvialGameState->GetCurrentModeContext());
}

void UAGPSurvivalModeWCBase::OnGameStateSet(AGameStateBase* const GameStateBase)
{
	AAGPGameState_Survial* CurrentGameState = Cast<AAGPGameState_Survial>(GameStateBase);
	if (CurrentGameState)
	{
		OnSetSurvialGameState(CurrentGameState);
	}
}

void UAGPSurvivalModeWCBase::OnChangeSurvialGameModeContext(const FAGPSurvialGameModeContext& NewContext)
{
	/* 동일한 상태 중복 호출에 대해서는 처리하지 않음 */
	if (CurrentSurvialModeContext.ModeState == NewContext.ModeState)
	{
		return;
	}

	CurrentSurvialModeContext = NewContext;

	OnNotifySurvialModeContext.Broadcast(CurrentSurvialModeContext);
}