// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AGPAbilitySystemGlobals.generated.h"

/**
 * AbilitySystem 전역 설정 및 구성 관련 클래스
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
	//별도의 GameplayEffectContext 클래스를 사용하려면 이 함수를 재정의 해줘야 한다.
	//UAbilitySystemComponent::MakeEffectContext에서 호출된다.
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
