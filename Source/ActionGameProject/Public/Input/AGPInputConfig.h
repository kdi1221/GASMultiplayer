// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputTriggers.h"
#include "AGPInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UENUM()
enum class EMappingLayout : int32
{
	BaseInput = 0,
	WeaponInput,
	AbilityContextInput
};


USTRUCT(BlueprintType)
struct FAGPInputBindInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "/Script/EnhancedInput.ETriggerEvent"))
	uint8 BindTriggerEventFlags = static_cast<uint8>(ETriggerEvent::None);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputAction> BindInputAction = nullptr;

public:
	inline bool IsTriggerEventFlagOn(const ETriggerEvent InCheckEvent) const
	{
		return EnumHasAnyFlags(static_cast<ETriggerEvent>(BindTriggerEventFlags), InCheckEvent);
	}
};


/**
 * 
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPInputConfig : public UDataAsset
{
	GENERATED_BODY()
		
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMappingLayout MappingPriority;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input.Native"))
	TMap<FGameplayTag, FAGPInputBindInfo> NativeInputActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input.Ability"))
	TMap<FGameplayTag, FAGPInputBindInfo> AbilityInputActions;
};
