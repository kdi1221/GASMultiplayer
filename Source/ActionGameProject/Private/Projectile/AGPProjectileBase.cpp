// KJY All Rights Reserved


#include "Projectile/AGPProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AAGPProjectileBase::AAGPProjectileBase()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 700.f;
	ProjectileMovementComponent->MaxSpeed = 900.f;
	ProjectileMovementComponent->Velocity = FVector(1.f, 0.f, 0.f);
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;

	bReplicates = true;
	InitialLifeSpan = 4.f;

	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AAGPProjectileBase::SetDamageGESpecHandle(const FGameplayEffectSpecHandle& InSpecHandle)
{
	ApplyDamageGESpecHandle = InSpecHandle;
}

void AAGPProjectileBase::ApplyDamageToTarget(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!SourceASC)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	checkf(ApplyDamageGESpecHandle.IsValid(), TEXT("[%s], Projectile ApplyDamageGESpecHandle Invalid, ProjectileActor : %s"), __FUNCTIONW__, *GetActorNameOrLabel());
	SourceASC->ApplyGameplayEffectSpecToTarget(*ApplyDamageGESpecHandle.Data, TargetASC);
}

void AAGPProjectileBase::DestroyProjectile()
{
	if (HasAuthority())
	{
		Destroy();
	}
	else
	{
		/* Authority가 아닌 클라이언트는 서버로부터 Projectile의 Destroy가 수신되기전까지 비활성화한다. */

		/* Projectile 가리 */
		SetActorHiddenInGame(true);

		/* Projectile 이동 정지 */
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->StopMovementImmediately();
		}

		/* Projectile의 모든 Collsion을 OFF한다. */
		TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
		GetComponents(UPrimitiveComponent::StaticClass(), PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

ETeamAttitude::Type AAGPProjectileBase::GetTargetTeamAttitude(const AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return ETeamAttitude::Type::Neutral;
	}

	if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetInstigator()))
	{
		return OwnerTeamInterface->GetTeamAttitudeTowards(*TargetActor);
	}
	return ETeamAttitude::Type::Neutral;

}
