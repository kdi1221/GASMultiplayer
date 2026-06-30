// KJY All Rights Reserved


#include "Item/Instance/Functionality/AGPItemFunc_SpawnAttachCostume.h"
#include "Misc/DataValidation.h"
#include "Item/Slots/AGPEquipmentSlot.h"
#include "Item/Instance/AGPItemInstance.h"
#include "Costume/AttachCostumeBase.h"
#include "Item/Interfaces/AGPItemSystemExternalInterface.h"
#include "Item/Components/AGPItemSystemsFacadeComponent.h"
#include "Costume/AGPCostumeStruct.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

#define LOCTEXT_NAMESPACE "AGPItemFunc_SpawnAttachCostume"

UAGPItemFunc_SpawnAttachCostume::UAGPItemFunc_SpawnAttachCostume()
{
#if WITH_EDITORONLY_DATA
	EditorPropertyName = TEXT("SpawnAttachCostume");
#endif
}

void UAGPItemFunc_SpawnAttachCostume::OnMountedInSlot(const UAGPItemSlot* InSlotInstance)
{
	verifyf(InSlotInstance, TEXT("[%s] - Invalid InSlotInstance"), __FUNCTIONW__);

	/* 장착된 슬롯의 Role 확인 - 아이템 인스턴스 자체는 World를 Outer로 가지기 때문에 정확한 NetRole을 확인하기 어렵다. */
	if (ROLE_Authority == InSlotInstance->GetOwnerRole())
	{
		//장비 슬롯일때 처리
		if (InSlotInstance->GetSlotCategory().MatchesTagExact(AGPGameplayTags::Items_Slots_Equipment))
		{
			const UAGPEquipmentSlot* EquipmentSlot = Cast<const UAGPEquipmentSlot>(InSlotInstance);
			verifyf(EquipmentSlot, TEXT("[%s] - Invalid EquipmentSlot"), __FUNCTIONW__);

			AddItemCostumeToOwner(EquipmentSlot);
		}
	}
}

void UAGPItemFunc_SpawnAttachCostume::OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance)
{
	verifyf(InSlotInstance, TEXT("[%s] - Invalid InSlotInstance"), __FUNCTIONW__);

	/* 장착된 슬롯의 Role 확인 - 아이템 인스턴스 자체는 World를 Outer로 가지기 때문에 정확한 NetRole을 확인하기 어렵다. */
	if (ROLE_Authority == InSlotInstance->GetOwnerRole())
	{
		//장비 슬롯일때 처리
		if (InSlotInstance->GetSlotCategory().MatchesTagExact(AGPGameplayTags::Items_Slots_Equipment))
		{
			const UAGPEquipmentSlot* EquipmentSlot = Cast<const UAGPEquipmentSlot>(InSlotInstance);
			verifyf(EquipmentSlot, TEXT("[%s] - Invalid EquipmentSlot"), __FUNCTIONW__);

			RemoveItemCostumeFromOwner(EquipmentSlot);
		}
	}
}

void UAGPItemFunc_SpawnAttachCostume::AddItemCostumeToOwner(const UAGPEquipmentSlot* InMountedEquipSlot)
{
	IAGPItemSystemExternalInterface* InItemSystemInterface = InMountedEquipSlot->GetSlotOwner<IAGPItemSystemExternalInterface>();
	verifyf(InItemSystemInterface, TEXT("[%s] - Invalid InItemSystemInterface"), __FUNCTIONW__);

	UAGPItemSystemsFacadeComponent* ItemFacadeComponent = InItemSystemInterface->GetItemSystemFacadeComponent();
	verifyf(ItemFacadeComponent, TEXT("[%s] - Invalid ItemFacadeComponent, [%s]"), __FUNCTIONW__, *GetNameSafe(InItemSystemInterface->_getUObject()));

	FCostumeInstanceIdentifierHandle CostumeIdentifierHandle(MakeShared<FCostumeInstanceEquipmentIdentifier>(InMountedEquipSlot->GetEquipmentSlotTag()));
	ItemFacadeComponent->AddCostumeToOwner(CostumeIdentifierHandle, AttachCostumeClass);
}

void UAGPItemFunc_SpawnAttachCostume::RemoveItemCostumeFromOwner(const UAGPEquipmentSlot* InUnmountedEquipSlot)
{
	IAGPItemSystemExternalInterface* InItemSystemInterface = InUnmountedEquipSlot->GetSlotOwner<IAGPItemSystemExternalInterface>();
	verifyf(InItemSystemInterface, TEXT("[%s] - Invalid InItemSystemInterface"), __FUNCTIONW__);

	UAGPItemSystemsFacadeComponent* ItemFacadeComponent = InItemSystemInterface->GetItemSystemFacadeComponent();
	verifyf(ItemFacadeComponent, TEXT("[%s] - Invalid ItemFacadeComponent, [%s]"), __FUNCTIONW__, *GetNameSafe(InItemSystemInterface->_getUObject()));

	FCostumeInstanceIdentifierHandle CostumeIdentifierHandle(MakeShared<FCostumeInstanceEquipmentIdentifier>(InUnmountedEquipSlot->GetEquipmentSlotTag()));
	ItemFacadeComponent->RemoveCostumeInstance(CostumeIdentifierHandle);
}

#if WITH_EDITOR
EDataValidationResult UAGPItemFunc_SpawnAttachCostume::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (Result != EDataValidationResult::Valid)
	{
		return Result;
	}

	if (!AttachCostumeClass)
	{
		Context.AddError(LOCTEXT("AttachCostumeClass Invalid", "AttachCostumeClass Invalid.."));
		Result = EDataValidationResult::Invalid;
		return Result;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE