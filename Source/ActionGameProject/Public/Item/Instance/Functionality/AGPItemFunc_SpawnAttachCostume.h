// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Item/Instance/Functionality/AGPItemFunctionality.h"
#include "AGPItemFunc_SpawnAttachCostume.generated.h"

class UAttachCostumeBase;
class UAGPEquipmentSlot;
class IAGPItemSystemExternalInterface;

/**
 * 아이템의 장비, 소유등의 이벤트에 대한 AttachCostumeActor Spawn
 */

UCLASS(DisplayName = "SpawnAttachCostume")
class ACTIONGAMEPROJECT_API UAGPItemFunc_SpawnAttachCostume : public UAGPItemFunctionality
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttachCostumeBase> AttachCostumeClass;

public:
	UAGPItemFunc_SpawnAttachCostume();

public:
	virtual void OnMountedInSlot(const UAGPItemSlot* InSlotInstance) override;
	virtual void OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance) override;

private:
	void AddItemCostumeToOwner(const UAGPEquipmentSlot* InMountedEquipSlot);
	void RemoveItemCostumeFromOwner(const UAGPEquipmentSlot* InUnmountedEquipSlot);

#if WITH_EDITOR
public:
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif	
};
