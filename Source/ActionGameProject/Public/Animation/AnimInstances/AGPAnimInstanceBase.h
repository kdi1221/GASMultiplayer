// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AGPAnimInstanceBase.generated.h"

struct FGameplayTag;
class UAGPCharPresentationComponent;

/**
 * 
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
	
private:
	TWeakObjectPtr<UAGPCharPresentationComponent> OwnerCharacterPresentationComponent;
	
public:
	virtual void NativeInitializeAnimation() override;
	
protected:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, AutoCreateRefTerm = "InTagToCheck"))
	bool DoesOwnerHaveTag(const FGameplayTag& InTagToCheck) const;
};
