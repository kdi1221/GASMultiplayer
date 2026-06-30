// KJY All Rights Reserved


#include "AbilitySystem/Abilities/AGPGameplayAbilityBaseFeature.h"
#include "Item/Interfaces/AGPItemSystemExternalInterface.h"
#include "Item/Components/AGPItemSystemsFacadeComponent.h"
#include "AGPGameplayTags.h"

UAGPGameplayAbilityBaseFeature::UAGPGameplayAbilityBaseFeature()
{
	/*AbilityTags.AddTag(AGPGameplayTags::Ability_Utility_Interaction_Item);
	BlockAbilitiesWithTag.AddTag(AGPGameplayTags::Ability);
	ActivationOwnedTags.AddTag(AGPGameplayTags::State_Interaction_Item);
	ActivationBlockedTags.AddTag(AGPGameplayTags::State_ActiveAbility);
	ActivationBlockedTags.AddTag(AGPGameplayTags::State_Death);
	ActivationBlockedTags.AddTag(AGPGameplayTags::State_DamageResponse);*/

	InstancingPolicy = EGameplayAbilityInstancingPolicy::Type::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalOnly;
}

void UAGPGameplayAbilityBaseFeature::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	checkf(ActorInfo, TEXT("[%s], Invalid ActorInfo"), __FUNCTIONW__);
	checkf(TriggerEventData, TEXT("[%s], Invalid TriggerEventData"), __FUNCTIONW__);

	IAGPItemSystemExternalInterface* ItemSystemExternalInterace = Cast<IAGPItemSystemExternalInterface>(ActorInfo->OwnerActor);
	if (!ItemSystemExternalInterace)
	{
		return;
	}

	UAGPItemSystemsFacadeComponent* FacadeComponent = ItemSystemExternalInterace->GetItemSystemFacadeComponent();
	if (!FacadeComponent)
	{
		return;
	}

	/* 기본 기능 동작 요청 */
	const FAGPRequestPacketKey RequestPacketKey = FacadeComponent->RequestBaseFeature(TriggerEventData->EventTag, TriggerEventData->TargetData, FItemBaseFeatureResultDelegate::CreateUObject(this, &UAGPGameplayAbilityBaseFeature::OnReceiveBaseFeatureResult));
	if (RequestPacketKey.IsValid())
	{
		WaitingRequestPacketKey = RequestPacketKey;
	}
}

void UAGPGameplayAbilityBaseFeature::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	/* 델리게이트 연결 해제 */
	if (WaitingRequestPacketKey.IsValid())
	{
		checkf(ActorInfo, TEXT("[%s], Invalid ActorInfo"), __FUNCTIONW__);

		IAGPItemSystemExternalInterface* ItemSystemExternalInterace = Cast<IAGPItemSystemExternalInterface>(ActorInfo->OwnerActor);
		if (!ItemSystemExternalInterace)
		{
			return;
		}

		UAGPItemSystemsFacadeComponent* FacadeComponent = ItemSystemExternalInterace->GetItemSystemFacadeComponent();
		if (!FacadeComponent)
		{
			return;
		}

		FacadeComponent->RemoveRequestPacketWaitInfo(WaitingRequestPacketKey);
	}
}

void UAGPGameplayAbilityBaseFeature::OnReceiveBaseFeatureResult(const uint8 InResultCode)
{
	checkf(CurrentActorInfo, TEXT("[%s], Invalid CurrentActorInfo"), __FUNCTIONW__);

	//TODO : InResultCode 안내 및 처리

	/* Request Packet Key 초기화 */
	WaitingRequestPacketKey.Reset();

	const bool bReplicateEndAbility = false;
	const bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}
