// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPCostumeEnum.generated.h"

UENUM(BlueprintType)
enum class EAGPCostumeType : uint8
{
	None = 0		UMETA(Hidden),
	FromEquipment,
};