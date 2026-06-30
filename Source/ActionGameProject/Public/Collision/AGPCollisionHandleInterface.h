// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AGPCollisionHandleInterface.generated.h"

class UAGPCollisionHandleComponent;

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UAGPCollisionHandleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ACTIONGAMEPROJECT_API IAGPCollisionHandleInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UAGPCollisionHandleComponent* GetCollisionHandleComponent() const;

	virtual UAGPCollisionHandleComponent* GetCollisionHandleComponent_Implementation() const { return nullptr; }
};
