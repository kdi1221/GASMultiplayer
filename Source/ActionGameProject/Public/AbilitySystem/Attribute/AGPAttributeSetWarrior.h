// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/AGPAttributeSetBase.h"
#include "AGPAttributeSetWarrior.generated.h"

/**
 * Warrior 전용 속성값들
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPAttributeSetWarrior : public UAGPAttributeSetBase
{
	GENERATED_BODY()
	
public:
	/* 현재 분노 수치 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentRage, Category = "Rage")
	FGameplayAttributeData CurrentRage;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetWarrior, CurrentRage);

	/* 최대 분노 수치 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxRage, Category = "Rage")
	FGameplayAttributeData MaxRage;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetWarrior, MaxRage);

public:
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

public:
	UFUNCTION()
	void OnRep_CurrentRage(const FGameplayAttributeData& OldCurrentRage) const;

	UFUNCTION()
	void OnRep_MaxRage(const FGameplayAttributeData& OldMaxRage) const;

protected:
	virtual void OnHandlePostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data, const FAGPGESourceTargetProperties& InGEProperties) override;

private:
	void HandleRageDecrease(const FAGPGESourceTargetProperties& InGEProperties, const FGameplayEffectSpec& InEffectSpec);
};
