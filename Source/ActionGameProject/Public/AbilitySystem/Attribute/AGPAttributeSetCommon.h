// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/AGPAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "AGPAttributeSetCommon.generated.h"

/**
 *  캐릭터에게 공통적으로 부여되는 공용속성값들
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPAttributeSetCommon : public UAGPAttributeSetBase
{
	GENERATED_BODY()
	
public:
	/* 현재 체력 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetCommon, CurrentHealth);

	/* 최대 체력 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetCommon, MaxHealth);

	/* 공격력 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageValue, Category = "Damage")
	FGameplayAttributeData DamageValue;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetCommon, DamageValue);

	/* 방어력 대비 공격력 적용 배율 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageApplyRate, Category = "Damage")
	FGameplayAttributeData DamageApplyRate;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetCommon, DamageApplyRate);

	/* 방어력 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DefenseRate, Category = "Defense")
	FGameplayAttributeData DefenseRate;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetCommon, DefenseRate);

public:
	/* 들어온 데미지 */
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAGPAttributeSetCommon, IncomingDamage);

public:
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

	//특정 속성값(Base)이 변경되기 전에 호출되는 함수
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	//특정 속성값(Current)이 변경되기 전에 호출되는 함수
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//특정 속성값이 변경된 후 호출되는 함수
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

public:
	UFUNCTION()
	void OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_DamageValue(const FGameplayAttributeData& OldDamageValue) const;

	UFUNCTION()
	void OnRep_DamageApplyRate(const FGameplayAttributeData& OldDamageApplyRate) const;

	UFUNCTION()
	void OnRep_DefenseRate(const FGameplayAttributeData& OldDefenseRate) const;

protected:
	virtual void OnHandlePostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data, const FAGPGESourceTargetProperties& InGEProperties) override;

private:
	void HandleIncomingDamage(const FAGPGESourceTargetProperties& InGEProperties, const FGameplayEffectSpec& InEffectSpec);
};
