// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Item/Instance/AGPItemInstance.h"
#include "AGPItemFunctionLibrary.generated.h"

class AAGP_CharacterBase;
struct FGameplayTag;

/**
 * 아이템 시스템 관련 Blueprint Function Library
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/* 아이템 인스턴스 생성 */
	UFUNCTION(Category = "AGP|Item", meta = (WorldContext = "InWorldContext"))
	static UAGPItemInstance* CreateAGPItemInstance(UObject* InWorldContext, TSubclassOf<UAGPItemInstance> InItemInstanceClass, const int32 InItemNum);

public:
	UFUNCTION(BlueprintCallable, Category = "AGP|Item", Meta = (DefaultToSelf = "InTarget", AutoCreateRefTerm = "InTagEquipSlot", Categories = "Items.Slots.Equipment"))
	static void InventoryItemToEquipment(AActor* InTarget, const int32 InInventorySlotIndex, const FGameplayTag& InTagEquipSlot);

	UFUNCTION(BlueprintCallable, Category = "AGP|Item", Meta = (DefaultToSelf = "InTarget", AutoCreateRefTerm = "InTagEquipSlot", Categories = "Items.Slots.Equipment"))
	static void DisrobeEquipmentToInventory(AActor* InTarget, const FGameplayTag& InTagEquipSlot, const int32 InInventorySlotIndex);
};
