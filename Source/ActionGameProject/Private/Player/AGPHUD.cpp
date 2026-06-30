// KJY All Rights Reserved


#include "Player/AGPHUD.h"
#include "Player/AGP_PlayerControllerBase.h"
#include "Widget/WidgetController/AGPWCStatusOverlay.h"
#include "Widget/WidgetController/AGPGameEndWidgetController.h"
#include "Widget/WidgetController/AGPWCIngameMenu.h"
#include "Widget/AGPUserWidget.h"
#include "GameMode/AGPGameState_Survial.h"
#include "Log/AGPLogChannels.h"

void AAGPHUD::BeginPlay()
{
	Super::BeginPlay();

	AAGPGameState_Survial* CurrentGameState = GetWorld()->GetGameState<AAGPGameState_Survial>();
	if (CurrentGameState)
	{
		/* Survival Mode 변화에 따른 이벤트 수신 */
		CurrentGameState->OnChangeSurvialModeState.AddUniqueDynamic(this, &AAGPHUD::OnChangeSurvialModeState);

		/* 현재 Survival Mode 정보 저장 */
		SurvivalModeContext = CurrentGameState->GetCurrentModeContext();
		OnChangeSurvialModeState(SurvivalModeContext);
	}

	AAGP_PlayerControllerBase* OwnerPC = Cast<AAGP_PlayerControllerBase>(GetOwningPlayerController());
	checkf(OwnerPC, TEXT("[%s] - Invalid OwnerPC"), __FUNCTIONW__);

	//Status Widget 컨트롤러 생성
	WC_StatusOverlay = NewObject<UAGPWCStatusOverlay>(this);
	checkf(WC_StatusOverlay, TEXT("[%s] - Invalid WC_StatusOverlay"), __FUNCTIONW__);
	WC_StatusOverlay->InitializeWidgetController(OwnerPC);

	//Status Widget 생성 및 위젯 컨트롤러 설정
	checkf(StatusOverlayWidgetClass, TEXT("[%s] - Invalid StatusOverlayWidgetClass"), __FUNCTIONW__);
	StatusOverlayWidget = CreateWidget<UAGPUserWidget>(GetWorld(), StatusOverlayWidgetClass);
	checkf(StatusOverlayWidget, TEXT("[%s] - Invalid StatusOverlayWidget"), __FUNCTIONW__);
	StatusOverlayWidget->SetWidgetController(WC_StatusOverlay);
	StatusOverlayWidget->AddToViewport();

	//GameEnd Widget 컨트롤러 생성
	WC_GameEnd = NewObject<UAGPGameEndWidgetController>(this);
	checkf(WC_GameEnd, TEXT("[%s] - Invalid WC_GameEnd"), __FUNCTIONW__);
	WC_GameEnd->InitializeWidgetController(OwnerPC);

	//GameEnd Widget 생성 및 위젯 컨트롤러 설정
	checkf(GameEndWidgetClass, TEXT("[%s] - Invalid GameEndWidgetClass"), __FUNCTIONW__);
	GameEndWidget = CreateWidget<UAGPUserWidget>(GetWorld(), GameEndWidgetClass);
	checkf(GameEndWidget, TEXT("[%s] - Invalid GameEndWidget"), __FUNCTIONW__);
	GameEndWidget->SetWidgetController(WC_GameEnd);
	GameEndWidget->AddToViewport();

	//Ingame Menu Widget 생성 및 위젯 컨트롤러 설정
	WC_IngameMenu = NewObject<UAGPWCIngameMenu>(this);
	checkf(WC_IngameMenu, TEXT("[%s] - Invalid WC_IngameMenu"), __FUNCTIONW__);
	WC_IngameMenu->InitializeWidgetController(OwnerPC);

	checkf(IngameMenuWidgetClass, TEXT("[%s] - Invalid IngameMenuWidgetClass"), __FUNCTIONW__);
	IngameMenuWidget = CreateWidget<UAGPUserWidget>(GetWorld(), IngameMenuWidgetClass);
	checkf(IngameMenuWidget, TEXT("[%s] - Invalid IngameMenuWidget"), __FUNCTIONW__);
	IngameMenuWidget->SetWidgetController(WC_IngameMenu);
	IngameMenuWidget->AddToViewport();
}

void AAGPHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AAGP_PlayerControllerBase* OwnerPC = Cast<AAGP_PlayerControllerBase>(GetOwningPlayerController());

	if (IngameMenuWidget)
	{
		IngameMenuWidget->RemoveFromParent();
		IngameMenuWidget = nullptr;
	}

	if (WC_IngameMenu)
	{
		WC_IngameMenu->DeinitializeWidgetController(OwnerPC);
		WC_IngameMenu->MarkAsGarbage();
		WC_IngameMenu = nullptr;
	}

	if (GameEndWidget)
	{
		GameEndWidget->RemoveFromParent();
		GameEndWidget = nullptr;
	}

	if (WC_GameEnd)
	{
		WC_GameEnd->DeinitializeWidgetController(OwnerPC);
		WC_GameEnd->MarkAsGarbage();
		WC_GameEnd = nullptr;
	}
	
	//Status Widget 제거
	if (StatusOverlayWidget)
	{
		StatusOverlayWidget->RemoveFromParent();
		StatusOverlayWidget = nullptr;
	}

	// Status WidgetController 리셋
	if (WC_StatusOverlay)
	{
		WC_StatusOverlay->DeinitializeWidgetController(OwnerPC);
		WC_StatusOverlay->MarkAsGarbage();
		WC_StatusOverlay = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AAGPHUD::ToggleIngameMenu()
{
	/* 게임이 이미 종료되어 종료위젯이 표시되는 상태에서는 인게임메뉴 표시하지 않음 */
	if (EAGPSurvialGameModeState::AllWavesClear == SurvivalModeContext.ModeState ||
		EAGPSurvialGameModeState::PlayersDefeat == SurvivalModeContext.ModeState)
	{
		return;
	}

	/* 인게임 메뉴 Show/Hide */
	if (WC_IngameMenu)
	{
		WC_IngameMenu->SetInGameMenuShow(!WC_IngameMenu->GetCurrentShowMenuFlag());

		if (WC_IngameMenu->GetCurrentShowMenuFlag())
		{
			SetInputModeAndCursor(FInputModeGameAndUI(), true);
		}
		else
		{
			SetInputModeAndCursor(FInputModeGameOnly(), false);
		}
	}
}

void AAGPHUD::SetInputModeAndCursor(const FInputModeDataBase& InputMode, bool bShowCursor)
{
	APlayerController* OwnerPlayerController = GetOwningPlayerController();
	if (!OwnerPlayerController)
	{
		return;
	}

	OwnerPlayerController->SetInputMode(InputMode);
	OwnerPlayerController->SetShowMouseCursor(bShowCursor);
}

void AAGPHUD::OnChangeSurvialModeState(const FAGPSurvialGameModeContext& CurrentSurvialModeState)
{
	//AGP_LOG(LogAGPGameState, Log, TEXT("Current Mode State - %s"), *CurrentSurvialModeState.ToString());

	SurvivalModeContext = CurrentSurvialModeState;

	switch (SurvivalModeContext.ModeState)
	{
	case EAGPSurvialGameModeState::AllWavesClear:
	case EAGPSurvialGameModeState::PlayersDefeat:
		{
			/* 인게임 메뉴 가리기 */
			if (WC_IngameMenu)
			{
				WC_IngameMenu->SetInGameMenuShow(false);
			}

			SetInputModeAndCursor(FInputModeUIOnly(), true);
		}
		break;

	default:
		{	
			SetInputModeAndCursor(FInputModeGameOnly(), false);
		}
		break;
	}

	OnChangedCurrentSurvivalMode(SurvivalModeContext);
}