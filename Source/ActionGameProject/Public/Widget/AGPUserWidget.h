// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AGPUserWidget.generated.h"

class UAGPWidgetController;

/**
 * 프로젝트 내 위젯들의 Base Class
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	/* 디자인되는 위젯들이 참조하는 위젯컨트롤러 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAGPWidgetController> WidgetController;

public:
	/* 위젯컨트롤러 설정 */
	void SetWidgetController(UAGPWidgetController* InWidgetController);

	/* 설정된 위젯 컨트롤러 초기화 */
	void ResetWidgetController();

protected:
	/* 위젯 컨트롤러가 설정될때 호출되는 이벤트 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnWidgetControllerSet();

	/* 위젯 컨트롤러가 초기화될 때 호출되는 이벤트 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnWidgetControllerReset();
};
