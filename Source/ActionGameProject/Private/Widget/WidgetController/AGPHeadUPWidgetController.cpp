// KJY All Rights Reserved


#include "Widget/WidgetController/AGPHeadUPWidgetController.h"
#include "Characters/AGP_CharacterBase.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "Log/AGPLogChannels.h"

void UAGPHeadUPWidgetController::SetOwnerCharacter(AAGP_CharacterBase* InCharacter)
{
	checkf(InCharacter, TEXT("[%s] - Invalid Character"), __FUNCTIONW__);

	UAGP_AbilitySystemComponent* OwnerASC = InCharacter->GetAGPAbilitySystemComponent();
	checkf(OwnerASC, TEXT("[%s] - Invalid OwnerASC"), __FUNCTIONW__);

	UAGPAttributeSetCommon* CommonAttribute = InCharacter->GetCommonAttributeSet();
	checkf(CommonAttribute, TEXT("[%s] - Invalid CommonAttribute"), __FUNCTIONW__);

	/* 현재, 최대 체력치 값 저장 */
	CurrentHealthValue = CommonAttribute->GetCurrentHealth();
	MaxHealthValue = CommonAttribute->GetMaxHealth();

	/* 현재 체력, 최대 체력 변동에 대한 델리게이트에 바인딩 */
	OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetCurrentHealthAttribute()).AddUObject(this, &UAGPHeadUPWidgetController::OnChangeCurrentHealth);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetMaxHealthAttribute()).AddUObject(this, &UAGPHeadUPWidgetController::OnChangeMaxHealth);	
}

void UAGPHeadUPWidgetController::UninitializeOwnerChracter(AAGP_CharacterBase* InCharacter)
{
	checkf(InCharacter, TEXT("[%s] - Invalid Character"), __FUNCTIONW__);

	UAGP_AbilitySystemComponent* OwnerASC = InCharacter->GetAGPAbilitySystemComponent();
	checkf(OwnerASC, TEXT("[%s] - Invalid OwnerASC"), __FUNCTIONW__);

	UAGPAttributeSetCommon* CommonAttribute = InCharacter->GetCommonAttributeSet();
	checkf(CommonAttribute, TEXT("[%s] - Invalid CommonAttribute"), __FUNCTIONW__);

	/* 현재 체력, 최대 체력 변동에 대한 델리게이트 언바인딩 */
	OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetCurrentHealthAttribute()).RemoveAll(this);
	OwnerASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetMaxHealthAttribute()).RemoveAll(this);

	/* 델리게이트 초기화 */
	OnHealthValueChanged.Clear();
}

void UAGPHeadUPWidgetController::SetCurrentDisplayName(const FText& NewName)
{
	FText BeforeName = CurrentStringDisplayName;
	CurrentStringDisplayName = NewName;

	if (!CurrentStringDisplayName.EqualTo(BeforeName))
	{
		OnCurrentDisplayNameChanged.Broadcast(CurrentStringDisplayName);
	}
}

void UAGPHeadUPWidgetController::OnChangeCurrentHealth(const FOnAttributeChangeData& Data)
{
	//AGP_NET_LOG(this, LogAGPAttribute, Log, TEXT("CurrentHealth : [%f]"), Data.NewValue);

	CurrentHealthValue = Data.NewValue;

	OnHealthValueChanged.Broadcast(CurrentHealthValue, MaxHealthValue);
}

void UAGPHeadUPWidgetController::OnChangeMaxHealth(const FOnAttributeChangeData& Data)
{
	//AGP_NET_LOG(this, LogAGPAttribute, Log, TEXT("MaxHealth : [%f]"), Data.NewValue);

	MaxHealthValue = Data.NewValue;

	OnHealthValueChanged.Broadcast(CurrentHealthValue, MaxHealthValue);
}
