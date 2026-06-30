// KJY All Rights Reserved


#include "Item/Slots/AGPInventorySlot.h"
#include "Item/Instance/AGPItemInstance.h"
#include "AGPGameplayTags.h"


UAGPInventorySlot::UAGPInventorySlot()
{
	SetSlotCategoryTag(AGPGameplayTags::Items_Slots_Inventory);
}

void UAGPInventorySlot::SetInventorySlotIndex(const int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
}

FString UAGPInventorySlot::GetDebugString() const
{
	return TEXT("InventorySlot[") + FString::FromInt(SlotIndex) + TEXT("] - ") + Super::GetDebugString();
}
