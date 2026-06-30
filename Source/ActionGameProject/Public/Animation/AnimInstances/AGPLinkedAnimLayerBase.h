// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstances/AGPAnimInstanceBase.h"
#include "AGPLinkedAnimLayerBase.generated.h"

class UAGPCharacterAnimInstance;
/**
 * Link Layer AnimInstance들의 Base Class
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPLinkedAnimLayerBase : public UAGPAnimInstanceBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	UAGPCharacterAnimInstance* GetOwnerCharacterAnimInstance() const;
	
};
