// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Item/Slots/AGPItemSlot.h"
#include "GameplayTagContainer.h"
#include "AGPEquipmentSlot.generated.h"

/**
 * 장비 장착 슬롯 Class
 */

UCLASS(DefaultToInstanced, CollapseCategories)
class ACTIONGAMEPROJECT_API UAGPEquipmentSlot : public UAGPItemSlot
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleInstanceOnly)
	FGameplayTag EquipmentSlotTag;
	
public:
	UAGPEquipmentSlot();

public:
	void SetEquipmentSlotTag(const FGameplayTag& InSlotTag);

public:
	FORCEINLINE const FGameplayTag& GetEquipmentSlotTag() const { return EquipmentSlotTag; }

public:
	virtual FString GetDebugString() const;
};
