// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AGPGEExecCalc_IncomingDamage.generated.h"

/* 
	대상에게 적용되는 Damage 수치 결정 
	이 클래스를 설정한 GameplayEffect를 대상에게 적용할때 호출
	방어력, 저항력등의 수치등을 계산하여 실제 적용할 데미지 수치를 결정한다.
*/
UCLASS()
class ACTIONGAMEPROJECT_API UAGPGEExecCalc_IncomingDamage : 
	public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UAGPGEExecCalc_IncomingDamage();

public:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
