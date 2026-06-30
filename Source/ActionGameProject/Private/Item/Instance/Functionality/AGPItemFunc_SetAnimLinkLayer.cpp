// KJY All Rights Reserved


#include "Item/Instance/Functionality/AGPItemFunc_SetAnimLinkLayer.h"
#include "Misc/DataValidation.h"
#include "Item/Slots/AGPEquipmentSlot.h"
#include "Item/Instance/AGPItemInstance.h"
#include "AGPGameplayTags.h"
#include "Item/Interfaces/AGPItemSystemExternalInterface.h"
#include "Item/Components/AGPItemSystemsFacadeComponent.h"
#include "Log/AGPLogChannels.h"

#define LOCTEXT_NAMESPACE "AGPItemFunc_SetAnimLinkLayer"

UAGPItemFunc_SetAnimLinkLayer::UAGPItemFunc_SetAnimLinkLayer()
{
#if WITH_EDITORONLY_DATA
	EditorPropertyName = TEXT("SetAnimationLinkLayer");
#endif
}

void UAGPItemFunc_SetAnimLinkLayer::OnMountedInSlot(const UAGPItemSlot* InSlotInstance)
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

			SetOwnerAnimLinkLayer(EquipmentSlot->GetSlotOwner<IAGPItemSystemExternalInterface>());
		}
	}
}

void UAGPItemFunc_SetAnimLinkLayer::OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance)
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

			ResetOwnerAnimLinkLayer(EquipmentSlot->GetSlotOwner<IAGPItemSystemExternalInterface>());
		}
	}
}

void UAGPItemFunc_SetAnimLinkLayer::SetOwnerAnimLinkLayer(IAGPItemSystemExternalInterface* InItemSystemInterface)
{
	verifyf(InItemSystemInterface, TEXT("[%s] - Invalid InItemSystemInterface"), __FUNCTIONW__);

	UAGPItemSystemsFacadeComponent* ItemFacadeComponent = InItemSystemInterface->GetItemSystemFacadeComponent();
	verifyf(ItemFacadeComponent, TEXT("[%s] - Invalid ItemFacadeComponent, [%s]"), __FUNCTIONW__, *GetNameSafe(InItemSystemInterface->_getUObject()));

	ItemFacadeComponent->SetOwnerAnimLinkLayer(AnimLinkLayerName);
}

void UAGPItemFunc_SetAnimLinkLayer::ResetOwnerAnimLinkLayer(IAGPItemSystemExternalInterface* InItemSystemInterface)
{
	verifyf(InItemSystemInterface, TEXT("[%s] - Invalid InItemSystemInterface"), __FUNCTIONW__);

	UAGPItemSystemsFacadeComponent* ItemFacadeComponent = InItemSystemInterface->GetItemSystemFacadeComponent();
	verifyf(ItemFacadeComponent, TEXT("[%s] - Invalid ItemFacadeComponent, [%s]"), __FUNCTIONW__, *GetNameSafe(InItemSystemInterface->_getUObject()));

	ItemFacadeComponent->ResetOwnerAnimLinkLayer();
}

#if WITH_EDITOR
EDataValidationResult UAGPItemFunc_SetAnimLinkLayer::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (Result != EDataValidationResult::Valid)
	{
		return Result;
	}

	if (AnimLinkLayerName.IsNone())
	{
		Context.AddError(LOCTEXT("AnimLinkLayerName Empty", "AnimLinkLayerName Empty.."));
		Result = EDataValidationResult::Invalid;
		return Result;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE