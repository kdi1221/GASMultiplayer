// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widget/WidgetController/AGPWidgetController.h"
#include "AGPWCIngameMenu.generated.h"

class AAGP_PlayerControllerBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangedIngameMenuShowFlagSignature, bool, IsShow);

/**
 * 인게임내 표시되는 메뉴에 대한 위젯 컨트롤러 클래스
 */
UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPWCIngameMenu : public UAGPWidgetController
{
	GENERATED_BODY()
	
private:
	/* 인게임 메뉴 표시 여부 */
	UPROPERTY()
	bool bShowMenu = false;

private:
	TWeakObjectPtr<AAGP_PlayerControllerBase> OwnerPlayerController;

public:
	UPROPERTY(BlueprintAssignable)
	FOnChangedIngameMenuShowFlagSignature OnChangedIngameMenuShowFlag;

public:
	virtual void InitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController);
	virtual void DeinitializeWidgetController(AAGP_PlayerControllerBase* InOwnerPlayerController);

public:
	UFUNCTION(BlueprintCallable)
	void SetInGameMenuShow(bool bShow);

	UFUNCTION(BlueprintCallable)
	void ReturnToMainMenu();

public:
	UFUNCTION(BlueprintPure)
	bool GetCurrentShowMenuFlag() const { return bShowMenu; }
};
