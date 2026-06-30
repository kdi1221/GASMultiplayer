// KJY All Rights Reserved


#include "AbilitySystem/Attribute/AGPAttributeSetWarrior.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AGPGameplayTags.h"

void UAGPAttributeSetWarrior::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAGPAttributeSetWarrior, CurrentRage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAGPAttributeSetWarrior, MaxRage, COND_None, REPNOTIFY_Always);
}

void UAGPAttributeSetWarrior::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	//Base Value Clamp
	if (Attribute == GetCurrentRageAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxRage());
	}
}

void UAGPAttributeSetWarrior::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetCurrentRageAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxRage());
	}
}

void UAGPAttributeSetWarrior::OnRep_CurrentRage(const FGameplayAttributeData& OldCurrentRage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAGPAttributeSetWarrior, CurrentRage, OldCurrentRage);
}

void UAGPAttributeSetWarrior::OnRep_MaxRage(const FGameplayAttributeData& OldMaxRage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAGPAttributeSetWarrior, MaxRage, OldMaxRage);
}

void UAGPAttributeSetWarrior::OnHandlePostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data, const FAGPGESourceTargetProperties& InGEProperties)
{
	if (Data.EvaluatedData.Attribute == GetCurrentRageAttribute())
	{
		HandleRageDecrease(InGEProperties, Data.EffectSpec);
	}
}

void UAGPAttributeSetWarrior::HandleRageDecrease(const FAGPGESourceTargetProperties& InGEProperties, const FGameplayEffectSpec& InEffectSpec)
{
	if (GetCurrentRage() > 0.f)
	{
		return;
	}

	/* 현재 Rage가 0 이하면 Rage 종료 처리 */
	AActor* TargetAvatarActor = InGEProperties.TargetContext.AvaterActor.Get();
	FGameplayEventData PayloadEventData;
	PayloadEventData.EventTag = AGPGameplayTags::Event_Character_Rage_Depleted;
	PayloadEventData.Target = TargetAvatarActor;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetAvatarActor, PayloadEventData.EventTag, PayloadEventData);
}
