// KJY All Rights Reserved


#include "FieldObject/AGPFieldStone.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

void AAGPFieldStone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bConsumedStone);
}

void AAGPFieldStone::ConsumeStone(UAbilitySystemComponent* InConsumeSourceASC)
{
	checkf(InConsumeSourceASC, TEXT("[%s], Invalid InConsumeSourceASC, Stone[%s]"), __FUNCTIONW__, *GetActorNameOrLabel());
	checkf(StoneGameplayEffectClass, TEXT("[%s], Invalid StoneGameplayEffectClass, Stone[%s]"), __FUNCTIONW__, *GetActorNameOrLabel());
	checkf(!bConsumedStone, TEXT("[%s], Already Consume Stone Consumed, Stone[%s]"), __FUNCTIONW__, *GetActorNameOrLabel());

	/* Stone Gameplay Effect 적용 */
	FGameplayEffectContextHandle ApplyConsumeGEContextHandle = InConsumeSourceASC->MakeEffectContext();
	FGameplayEffectContext* ApplyConsumeGEContext = ApplyConsumeGEContextHandle.Get();
	/* Stone Object의 위치 지정(Gameplay Cue에서 FX표시등에 사용) */
	ApplyConsumeGEContext->AddOrigin(GetActorLocation());
	checkf(ApplyConsumeGEContext, TEXT("[%s], Invalid ApplyConsumeGEContext"), __FUNCTIONW__);
	FGameplayEffectSpecHandle ApplyConsumeGESpecHandle = InConsumeSourceASC->MakeOutgoingSpec(StoneGameplayEffectClass, 1.f, ApplyConsumeGEContextHandle);
	InConsumeSourceASC->ApplyGameplayEffectSpecToSelf(*ApplyConsumeGESpecHandle.Data.Get());

	bConsumedStone = true;
	OnConsumed();

	/* 사용되었을때의 FX표시등을 위해 Destory Delay 지정 */
	SetLifeSpan(1.f);
}

void AAGPFieldStone::OnConsumed_Implementation()
{
	SetActorHiddenInGame(true);
}

void AAGPFieldStone::OnRep_ConsumedStone()
{
	if (bConsumedStone)
	{
		OnConsumed();
	}
}
