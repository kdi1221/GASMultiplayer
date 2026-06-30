// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPItemSystemStruct.generated.h"

class UAGPItemInstance;

/* 초기 인벤토리 아이템에 대한 구조체 */
USTRUCT(BlueprintType)
struct FInitializeInventoryItemData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAGPItemInstance> AddItemClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ItemNum = 1;
};