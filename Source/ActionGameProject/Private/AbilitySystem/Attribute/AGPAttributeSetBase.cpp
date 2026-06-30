// KJY All Rights Reserved


#include "AbilitySystem/Attribute/AGPAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Characters/AGP_CharacterBase.h"

void UAGPAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	/* Data로부터 Source, Target 정보 확인 */
	FAGPGESourceTargetProperties GEProperties;
	GenerateGEPropertiesFromCallbackData(Data, GEProperties);

	/* GE에 의해 변경된 Attribute에 대한 후처리 */
	OnHandlePostGameplayEffectExecute(Data, GEProperties);
}

void UAGPAttributeSetBase::GenerateGEPropertiesFromCallbackData(const FGameplayEffectModCallbackData& InCallbackData, FAGPGESourceTargetProperties& OutGEProperties)
{
	OutGEProperties.GEContextHandle = InCallbackData.EffectSpec.GetContext();

	auto FillPropertyContextFromAbilityActorInfo =
		[&](UAbilitySystemComponent& InASC, FAGPGEProperties& OutProperties)
		{
			OutProperties.ASC = &InASC;

			const FGameplayAbilityActorInfo* AbilityActorInfo = InASC.AbilityActorInfo.Get();
			if (!AbilityActorInfo)
			{
				return;
			}

			AActor* OwnerActor = AbilityActorInfo->OwnerActor.Get();
			OutProperties.OwnerActor = OwnerActor;

			AActor* AvatarActor = AbilityActorInfo->AvatarActor.Get();
			OutProperties.AvaterActor = AvatarActor;

			APawn* AvatarPawn = Cast<APawn>(AvatarActor);
			OutProperties.CachedPawn = AvatarPawn;

			AAGP_CharacterBase* AvatarCharacrer = Cast<AAGP_CharacterBase>(AvatarPawn);
			OutProperties.CachedCharacter = AvatarCharacrer;

			OutProperties.Controller = AbilityActorInfo->PlayerController.Get();
			if (!OutProperties.Controller.IsValid() && AvatarPawn)
			{
				OutProperties.Controller = AvatarPawn->GetController();
			}
		};

	/* Source 정보 채우기 */
	if (UAbilitySystemComponent* SourceASC = OutGEProperties.GEContextHandle.GetOriginalInstigatorAbilitySystemComponent())
	{
		FillPropertyContextFromAbilityActorInfo(*SourceASC, OutGEProperties.SourceContext);
	}

	/* Target 정보 채우기 */
	FillPropertyContextFromAbilityActorInfo(InCallbackData.Target, OutGEProperties.TargetContext);
}