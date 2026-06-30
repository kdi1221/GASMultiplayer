// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AGPItemSystemExternalInterface.generated.h"

class UAGPItemSystemsFacadeComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAGPItemSystemExternalInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ACTIONGAMEPROJECT_API IAGPItemSystemExternalInterface
{
	GENERATED_BODY()
	
public:
	virtual UAGPItemSystemsFacadeComponent* GetItemSystemFacadeComponent() const = 0;
};
