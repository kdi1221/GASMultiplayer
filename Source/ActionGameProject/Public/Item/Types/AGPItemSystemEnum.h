// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPItemSystemEnum.generated.h"

UENUM()
enum class EAGPSlotEvent : int8
{
	MountedInSlot = 0,
	RemoveFromSlot
};