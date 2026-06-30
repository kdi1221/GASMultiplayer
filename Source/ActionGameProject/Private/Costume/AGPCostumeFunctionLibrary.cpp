// KJY All Rights Reserved


#include "Costume/AGPCostumeFunctionLibrary.h"

FCostumeInstanceIdentifierHandle UAGPCostumeFunctionLibrary::CreateCostumeIdentifierFromEquipment(const FGameplayTag& InEquipmentSlotTag)
{
	return FCostumeInstanceIdentifierHandle(MakeShared<FCostumeInstanceEquipmentIdentifier>(InEquipmentSlotTag));
}
