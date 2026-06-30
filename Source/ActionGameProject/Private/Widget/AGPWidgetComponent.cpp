// KJY All Rights Reserved


#include "Widget/AGPWidgetComponent.h"
#include "Widget/WidgetController/AGPHeadUPWidgetController.h"
#include "Widget/AGPUserWidget.h"

UAGPWidgetComponent::UAGPWidgetComponent()
{

}

void UAGPWidgetComponent::InitWidget()
{
	Super::InitWidget();

	/* InitWidget전에 WidgetController이 설정된 경우 이때 마저 호출해준다. */
	if (HeadUPWidgetController)
	{
		SetWidgetControllerToWidgetObject();
	}
}

void UAGPWidgetComponent::InitFloatingWidgetComponent(AAGP_CharacterBase* InCharacter)
{
	checkf(InCharacter, TEXT("[%s] - Invalid Character"), __FUNCTIONW__);

	HeadUPWidgetController = NewObject<UAGPHeadUPWidgetController>(this);
	checkf(HeadUPWidgetController, TEXT("[%s] - Invalid HeadUPWidgetController"), __FUNCTIONW__);

	/* Widget Controller에 Owner Character정보 알림 */
	HeadUPWidgetController->SetOwnerCharacter(InCharacter);

	IsCreatedWidgetController = true;

	/* User Widget이 이미 생성된 뒤라면(InitWidget호출 이후) Controller 지정 */
	SetWidgetControllerToWidgetObject();
}

void UAGPWidgetComponent::UninitializeFloatingWidgetComponent(AAGP_CharacterBase* InCharacter)
{
	checkf(InCharacter, TEXT("[%s] - Invalid Character"), __FUNCTIONW__);

	/* 위젯에 알림 */
	if (UAGPUserWidget* StatusWidgetObject = Cast<UAGPUserWidget>(GetUserWidgetObject()))
	{
		StatusWidgetObject->ResetWidgetController();
	}

	/* 기존 Widget Controller 리셋 */
	if (HeadUPWidgetController)
	{
		HeadUPWidgetController->UninitializeOwnerChracter(InCharacter);
		HeadUPWidgetController->MarkAsGarbage();
		HeadUPWidgetController = nullptr;
	}

	/* 플래그들 초기화 */
	IsCreatedWidgetController = false;
	IsControllerSetWidgetObject = false;
}

void UAGPWidgetComponent::SetCharacterNameText(const FText& InNameText)
{
	if (!HeadUPWidgetController)
	{
		return;
	}

	HeadUPWidgetController->SetCurrentDisplayName(InNameText);
}

void UAGPWidgetComponent::SetWidgetControllerToWidgetObject()
{
	if (GetControllerSetWidgetObject())
	{
		return;
	}

	if (UAGPUserWidget* StatusWidgetObject = Cast<UAGPUserWidget>(GetUserWidgetObject()))
	{
		StatusWidgetObject->SetWidgetController(HeadUPWidgetController);
		IsControllerSetWidgetObject = true;
	}
}
