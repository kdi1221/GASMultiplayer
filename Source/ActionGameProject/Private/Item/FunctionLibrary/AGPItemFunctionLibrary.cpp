// KJY All Rights Reserved


#include "Item/FunctionLibrary/AGPItemFunctionLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AGPGameplayTags.h"
#include "AbilitySystem/AGPAbilityTypes.h"


UAGPItemInstance* UAGPItemFunctionLibrary::CreateAGPItemInstance(UObject* InWorldContext, TSubclassOf<UAGPItemInstance> InItemInstanceClass, const int32 InItemNum)
{
	verifyf(InItemInstanceClass, TEXT("CreateAGPItemInstance - AddItemClass Invalid.."));
	verifyf(InItemNum >= 1, TEXT("CreateAGPItemInstance - ItemNum [%d].."), InItemNum);
	verifyf(InWorldContext, TEXT("CreateAGPItemInstance - InWorldContext Invalid.."));

	UWorld* CurrentWorld = InWorldContext->GetWorld();
	verifyf(CurrentWorld, TEXT("CreateAGPItemInstance - CurrentWorld Invalid.."));

	UAGPItemInstance* NewItemInstance = NewObject<UAGPItemInstance>(CurrentWorld, InItemInstanceClass);
	verifyf(NewItemInstance, TEXT("CreateAGPItemInstance - NewItemInstance Invalid.."));

	NewItemInstance->SetStackNum(InItemNum);

	return NewItemInstance;
}

void UAGPItemFunctionLibrary::InventoryItemToEquipment(AActor* InTarget, const int32 InInventorySlotIndex, const FGameplayTag& InTagEquipSlot)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InTarget);
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayEventData GameplayEventPayload;
		GameplayEventPayload.TargetData.Add(new FGameplayAbilityTargetData_ReuqestInventoryItemEquip(InInventorySlotIndex, InTagEquipSlot));

		FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
		AbilitySystemComponent->HandleGameplayEvent(AGPGameplayTags::Ability_Interaction_Item_EquipItem, &GameplayEventPayload);
	} 
}

void UAGPItemFunctionLibrary::DisrobeEquipmentToInventory(AActor* InTarget, const FGameplayTag& InTagEquipSlot, const int32 InInventorySlotIndex)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InTarget);
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayEventData GameplayEventPayload;
		GameplayEventPayload.TargetData.Add(new FGameplayAbilityTargetData_DisrobeEquipmentToInventory(InTagEquipSlot, InInventorySlotIndex));

		FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
		AbilitySystemComponent->HandleGameplayEvent(AGPGameplayTags::Ability_Interaction_Item_DisrobeEquipment, &GameplayEventPayload);
	}
}
