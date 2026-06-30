// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "AGPProjectileBase.generated.h"

class UProjectileMovementComponent;

UCLASS()
class ACTIONGAMEPROJECT_API AAGPProjectileBase : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

protected:
	/* Projectile의 Damage Effect Handle */
	UPROPERTY()
	FGameplayEffectSpecHandle ApplyDamageGESpecHandle;
	
public:	
	AAGPProjectileBase();

public:
	UFUNCTION(BlueprintCallable)
	void SetDamageGESpecHandle(const FGameplayEffectSpecHandle& InSpecHandle);

	/* 대상에 대해 Damage 적용 */
	UFUNCTION(BlueprintCallable)
	void ApplyDamageToTarget(AActor* TargetActor);

	/* Projectile Destory*/
	UFUNCTION(BlueprintCallable)
	void DestroyProjectile();

public:
	UFUNCTION(BlueprintPure)
	ETeamAttitude::Type GetTargetTeamAttitude(const AActor* TargetActor) const;
};
