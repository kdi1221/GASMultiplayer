// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "AGPWidgetComponent.generated.h"

class AAGP_CharacterBase;
class UAGPHeadUPWidgetController;

/**
 *  캐릭터에 표시되는 Widget Component
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UAGPHeadUPWidgetController> HeadUPWidgetController;

private:
	/* 컨트롤러 생성 여부 */
	bool IsCreatedWidgetController = false;

	/* WidgetObject에 컨트롤러 설정 여부 */
	bool IsControllerSetWidgetObject = false;

public:
	UAGPWidgetComponent();

public:
	virtual void InitWidget() override;
	
public:
	void InitFloatingWidgetComponent(AAGP_CharacterBase* InCharacter);
	void UninitializeFloatingWidgetComponent(AAGP_CharacterBase* InCharacter);
	void SetCharacterNameText(const FText& InNameText);

private:
	void SetWidgetControllerToWidgetObject();

public:
	FORCEINLINE bool GetCreatedWidgetController() const { return IsCreatedWidgetController; }
	FORCEINLINE bool GetControllerSetWidgetObject() const { return IsControllerSetWidgetObject; }
};
