// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "FieldObject/AGPFieldObjectBase.h"
#include "AGPFieldStone.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

/**
 * 맵상에 배치되는 Stone Object의 Base 클래스
 */
UCLASS(Abstract)
class ACTIONGAMEPROJECT_API AAGPFieldStone : public AAGPFieldObjectBase
{
	GENERATED_BODY()
	
protected:
	/* Consume 될때 적용할 GE */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> StoneGameplayEffectClass;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ConsumedStone)
	bool bConsumedStone = false;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	void ConsumeStone(UAbilitySystemComponent* InConsumeSourceASC);

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnConsumed();

private:
	UFUNCTION()
	void OnRep_ConsumedStone();
};
