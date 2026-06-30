// KJY All Rights Reserved


#include "Item/Instance/Functionality/AGPItemFunc_GrantAbility.h"
#include "Item/Slots/AGPEquipmentSlot.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

#define LOCTEXT_NAMESPACE "AGPItemFunc_GrantAbility"

UAGPItemFunc_GrantAbility::UAGPItemFunc_GrantAbility()
{
#if WITH_EDITORONLY_DATA
	EditorPropertyName = TEXT("GrantAbility");
#endif
}

void UAGPItemFunc_GrantAbility::OnMountedInSlot(const UAGPItemSlot* InSlotInstance)
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

			//Slow의 Owner의 AbilitySystem에 접근해서 Ability부여
			GrantAbilityToOwner(Cast<IAbilitySystemInterface>(InSlotInstance->GetSlotOwner<IAbilitySystemInterface>()));
		}
	}
}

void UAGPItemFunc_GrantAbility::OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance)
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

			//Slow의 Owner의 AbilitySystem에 접근해서 부여되었던 Ability제거
			RevokeAbilityFromOwner(Cast<IAbilitySystemInterface>(InSlotInstance->GetSlotOwner<IAbilitySystemInterface>()));
		}
	}
}

void UAGPItemFunc_GrantAbility::GrantAbilityToOwner(IAbilitySystemInterface* OwnerAbilitySystemInterface)
{
	if (!OwnerAbilitySystemInterface)
	{
		return;
	}

	UAbilitySystemComponent* OwnerASC = OwnerAbilitySystemInterface->GetAbilitySystemComponent();
	if (!OwnerASC)
	{
		return;
	}

	GrantAbilitiesSpecHandles.Empty();
	for (const FAGPGrantAbilitySet& AbilitySet : GrantAbilities)
	{
		if (!AbilitySet.AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityClass, AbilitySet.GrantAbilityLevel);
		//AbilitySpec.DynamicAbilityTags.AppendTags(AbilitySet.GrantAbilityToDynamicTag);
		AbilitySpec.GetDynamicSpecSourceTags().AppendTags(AbilitySet.GrantAbilityToDynamicTag);


		FGameplayAbilitySpecHandle GrantAbilitySpecHandle = OwnerASC->GiveAbility(AbilitySpec);
		GrantAbilitiesSpecHandles.Add(GrantAbilitySpecHandle);
	}
}

void UAGPItemFunc_GrantAbility::RevokeAbilityFromOwner(IAbilitySystemInterface* OwnerAbilitySystemInterface)
{
	if (!OwnerAbilitySystemInterface)
	{
		return;
	}

	UAbilitySystemComponent* OwnerASC = OwnerAbilitySystemInterface->GetAbilitySystemComponent();
	if (!OwnerASC)
	{
		return;
	}

	//OwnerASC->FindAbilitySpecFromHandle(

	for (const FGameplayAbilitySpecHandle& SpecHandle : GrantAbilitiesSpecHandles)
	{
		/* 테스트 : Clear이전에 CancelAbility를 먼저 호출? */
		//OwnerASC->CancelAbilityHandle(SpecHandle);

		OwnerASC->ClearAbility(SpecHandle);
	}

	/* 테스트 : ForceReplication.. */
	//OwnerASC->ForceReplication();

	GrantAbilitiesSpecHandles.Empty();
}

#if WITH_EDITOR
EDataValidationResult UAGPItemFunc_GrantAbility::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (Result != EDataValidationResult::Valid)
	{
		return Result;
	}

	/*if (!AttachCostumeClass)
	{
		Context.AddError(LOCTEXT("AttachCostumeClass Invalid", "AttachCostumeClass Invalid.."));
		Result = EDataValidationResult::Invalid;
		return Result;
	}*/

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE