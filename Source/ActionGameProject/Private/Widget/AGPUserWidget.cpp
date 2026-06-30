// KJY All Rights Reserved


#include "Widget/AGPUserWidget.h"

void UAGPUserWidget::SetWidgetController(UAGPWidgetController* InWidgetController)
{
	checkf(InWidgetController, TEXT("[%s] - WidgetController Invalid"), __FUNCTIONW__);

	WidgetController = InWidgetController;

	OnWidgetControllerSet();
}

void UAGPUserWidget::ResetWidgetController()
{
	WidgetController = nullptr;

	OnWidgetControllerReset();
}
