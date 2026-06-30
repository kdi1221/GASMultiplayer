// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AGPMonsterData.generated.h"


USTRUCT(BlueprintType)
struct FAGPMonsterNameData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	FText MonsterName;
};