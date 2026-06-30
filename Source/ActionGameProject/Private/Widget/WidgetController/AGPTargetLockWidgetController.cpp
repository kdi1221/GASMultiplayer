// KJY All Rights Reserved


#include "Widget/WidgetController/AGPTargetLockWidgetController.h"

void UAGPTargetLockWidgetController::SetTargetActor(AActor* InTargetActor)
{
	CurrentTargetActor = InTargetActor;

	OnTargetChanged.Broadcast(CurrentTargetActor);
}
