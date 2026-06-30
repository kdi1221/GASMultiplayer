// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AGPCostumeStruct.h"
#include "GameplayTagContainer.h"
#include "AGPCostumeFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPCostumeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "InEquipmentSlotTag", Categories = "Items.Slots.Equipment"))
	static FCostumeInstanceIdentifierHandle CreateCostumeIdentifierFromEquipment(const FGameplayTag& InEquipmentSlotTag);
	
};
