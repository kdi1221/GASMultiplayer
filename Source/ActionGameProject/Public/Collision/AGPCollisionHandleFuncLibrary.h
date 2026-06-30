// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AGPCollisionStruct.h"
#include "AGPCollisionHandleFuncLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPCollisionHandleFuncLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "InCollisionName, InCostumeActorName"))
	static FAGPCollisionIdentifierHandle CreateCollisionIdentifierCostumeActor(const FName& InCollisionName, const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, const FName& InCostumeActorName);

	UFUNCTION(BlueprintPure, meta = (AutoCreateRefTerm = "InCollisionName"))
	static FAGPCollisionIdentifierHandle CreateCollisionIdentifierAttachCharacter(const FName& InCollisionName);
};
