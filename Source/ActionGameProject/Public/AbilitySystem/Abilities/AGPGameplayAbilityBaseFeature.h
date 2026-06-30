// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "Common/AGPCommonStructs.h"
#include "AGPGameplayAbilityBaseFeature.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPGameplayAbilityBaseFeature : public UAGPGameplayAbility
{
	GENERATED_BODY()
	
private:
	FAGPRequestPacketKey WaitingRequestPacketKey;
	
public:
	UAGPGameplayAbilityBaseFeature();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnReceiveBaseFeatureResult(const uint8 InResultCode);
};
