// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AGPAbilityDataPrimaryAsset.generated.h"

class UMaterialInterface;

USTRUCT(BlueprintType)
struct FAGPAbilityData
{
	GENERATED_BODY()

public:
	/* Ability 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Name;

	/* Ability 설명 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Description;

	/* Ability의 Icon Material*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UMaterialInterface> IconMaterial;
};

/**
 * Ability에 대한 DataAsset 클래스
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPAbilityDataPrimaryAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ForceInlineRow, Categories = "Ability"))
	TMap<FGameplayTag, FAGPAbilityData> AbilityDataMap;
};
