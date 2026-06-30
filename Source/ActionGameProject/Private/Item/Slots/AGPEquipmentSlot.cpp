// KJY All Rights Reserved


#include "Item/Slots/AGPEquipmentSlot.h"
#include "Item/Instance/AGPItemInstance.h"
#include "AGPGameplayTags.h"

UAGPEquipmentSlot::UAGPEquipmentSlot()
{
	SetSlotCategoryTag(AGPGameplayTags::Items_Slots_Equipment);
}

void UAGPEquipmentSlot::SetEquipmentSlotTag(const FGameplayTag& InSlotTag)
{
	EquipmentSlotTag = InSlotTag;
}

FString UAGPEquipmentSlot::GetDebugString() const
{
	return TEXT("EquipmentSlot[") + EquipmentSlotTag.ToString() + TEXT("] - ") + Super::GetDebugString();
}
