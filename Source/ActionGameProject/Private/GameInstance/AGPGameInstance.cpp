// KJY All Rights Reserved


#include "GameInstance/AGPGameInstance.h"
#include "MoviePlayer.h"
#include "Widget/AGPUserWidget.h"

void UAGPGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ThisClass::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnDestinationWorldLoaded);
}

void UAGPGameInstance::OnPreLoadMap(const FString& MapName)
{
	if (!IsDedicatedServerInstance())
	{
		/* 로딩 UMG 위젯 생성 */
		UAGPUserWidget* CreatedLoadingWidget = CreateWidget<UAGPUserWidget>(this, LoadingWidgetClass, TEXT("LoadingScreen"));

		/* 생성된 위젯 유효여부에 따라 표시할 위젯 결정 */
		TSharedPtr<class SWidget> WidgetLoadingScreenPtr = nullptr;
		if (CreatedLoadingWidget)
		{
			WidgetLoadingScreenPtr = CreatedLoadingWidget->TakeWidget();
		}
		else
		{
			WidgetLoadingScreenPtr = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
		}

		FLoadingScreenAttributes LoadingScreenAttributes;
		LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 2.f;
		LoadingScreenAttributes.WidgetLoadingScreen = WidgetLoadingScreenPtr;

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
	}
}

void UAGPGameInstance::OnDestinationWorldLoaded(UWorld* LoadedWorld)
{
	if (!IsDedicatedServerInstance())
	{
		GetMoviePlayer()->StopMovie();
	}
}