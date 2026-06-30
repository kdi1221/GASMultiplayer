// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPCollisionEnum.generated.h"

UENUM(BlueprintType)
enum class EAGPCollisionType : uint8
{
	None = 0		UMETA(Hidden),
	CostumeActor,
	AttachCharacter,
};