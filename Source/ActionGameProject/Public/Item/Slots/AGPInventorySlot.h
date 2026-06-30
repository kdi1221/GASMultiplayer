// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Item/Slots/AGPItemSlot.h"
#include "Item/Types/AGPItemSystemConstDefine.h"
#include "AGPInventorySlot.generated.h"

/**
 * 인벤토리 내 아이템 슬롯 Class
 */

UCLASS(DefaultToInstanced, CollapseCategories)
class ACTIONGAMEPROJECT_API UAGPInventorySlot : public UAGPItemSlot
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleInstanceOnly)
	int32 SlotIndex = AGPItemConstDefine::INVALID_ITEM_SLOT_INDEX;

public:
	UAGPInventorySlot();

public:
	void SetInventorySlotIndex(const int32 InSlotIndex);

public:
	FORCEINLINE int32 GetInventorySlotIndex() const { return SlotIndex; }

public:
	virtual FString GetDebugString() const;
};
