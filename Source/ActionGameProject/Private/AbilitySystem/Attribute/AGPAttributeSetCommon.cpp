// KJY All Rights Reserved


#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Characters/AGP_CharacterBase.h"
#include "Characters/Components/AGPCharPresentationComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/GameplayCue/AGPGCInterface.h"
#include "AGPGameplayTags.h"
#include "AbilitySystem/AGPAbilityTypes.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Log/AGPLogChannels.h"

void UAGPAttributeSetCommon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAGPAttributeSetCommon, CurrentHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAGPAttributeSetCommon, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAGPAttributeSetCommon, DamageValue, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAGPAttributeSetCommon, DamageApplyRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAGPAttributeSetCommon, DefenseRate, COND_None, REPNOTIFY_Always);
}

void UAGPAttributeSetCommon::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	//Base Value Clamp
	if (Attribute == GetCurrentHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UAGPAttributeSetCommon::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	//속성값 변경 직전에 호출되어, 이때 Clamp 적용(Current Value)
	//Epic 공식 권장 - Clamp용으로만 사용할것, 여기서 이벤트를 시작하거나 게임플레이 로직을 시작하지 말것
	//PreAttributeChange에서 Clamp할때는 GameplayEffect의 수정자로부터 변경될 값에 대해서만 Clamp가 이루어지며(CurrentValue에만 영향을 줌), BaseValue는 Clamp적용을 받지 않는다.
	if (Attribute == GetCurrentHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UAGPAttributeSetCommon::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UAGPAttributeSetCommon::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAGPAttributeSetCommon, CurrentHealth, OldCurrentHealth);
}

void UAGPAttributeSetCommon::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAGPAttributeSetCommon, MaxHealth, OldMaxHealth);
}

void UAGPAttributeSetCommon::OnRep_DamageValue(const FGameplayAttributeData& OldDamageValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAGPAttributeSetCommon, DamageValue, OldDamageValue);
}

void UAGPAttributeSetCommon::OnRep_DamageApplyRate(const FGameplayAttributeData& OldDamageApplyRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAGPAttributeSetCommon, DamageApplyRate, OldDamageApplyRate);
}

void UAGPAttributeSetCommon::OnRep_DefenseRate(const FGameplayAttributeData& OldDefenseRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAGPAttributeSetCommon, DefenseRate, OldDefenseRate);
}

void UAGPAttributeSetCommon::OnHandlePostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data, const FAGPGESourceTargetProperties& InGEProperties)
{
	/* GameplayEffect들이 적용될때 계산된 데미지 수치 수신 */
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		/* 받은 데미지 처리(Hit, Dead 등) */
		HandleIncomingDamage(InGEProperties, Data.EffectSpec);
	}
}

void UAGPAttributeSetCommon::HandleIncomingDamage(const FAGPGESourceTargetProperties& InGEProperties, const FGameplayEffectSpec& InEffectSpec)
{
	/* 들어온 데미지를 저장한 뒤 초기화 */
	const float DamageIncoming = GetIncomingDamage();
	SetIncomingDamage(0.f);

	if (DamageIncoming > 0.f)
	{
		AActor* SourceAvatarActor = InGEProperties.SourceContext.AvaterActor.Get();
		AActor* EffectCauser = InEffectSpec.GetEffectContext().GetEffectCauser();
		AActor* TargetAvatarActor = InGEProperties.TargetContext.AvaterActor.Get();
		UAbilitySystemComponent* TargetASC = InGEProperties.TargetContext.ASC.Get();

		//Hit Direction 계산
		float AngleDifference = 0.f;
		EAGPDirection HitDirection = EAGPDirection::Front;

		/* EffectCauser, Target Avatar가 명확할때 HitDirection 계산 */
		/* EffectCauser의 경우 현재 근접공격에 대해서는 AvatarActor가, 투사체의 경우 해당 Projectile이 지정되어 있다. */
		if (EffectCauser && TargetAvatarActor)
		{
			HitDirection = UAGPAbilitySystemFunctionLibrary::ComputeDirectionSourceToTarget(EffectCauser, TargetAvatarActor, AngleDifference);
		}

		/* AISense_Damage, AI에 지정된 Damage Sense의 경우 해당 함수로 피격 이벤트를 알려줘야 한다. */
		UAISense_Damage::ReportDamageEvent(this, TargetAvatarActor, SourceAvatarActor, DamageIncoming, SourceAvatarActor->GetActorLocation(), TargetAvatarActor->GetActorLocation());

		/* 체력 감소 */
		const float OldHealth = GetCurrentHealth();
		const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageIncoming, 0.f, GetMaxHealth());
		SetCurrentHealth(NewCurrentHealth);

		/* 체력 감소 후 0이하인지 확인 */
		const bool bFatal = GetCurrentHealth() <= 0.f;

		/* Death 테스트 */
		//const bool bFatal = true;

		if (bFatal)
		{
			//체력이 0이하 => 캐릭터 사망
			//AGP_NET_LOG(this, LogAGPAttribute, Log, TEXT("[%s] Dead"), *GetNameSafe(GetOwningActor()));

			FGameplayEventData PayloadEventData;
			PayloadEventData.EventTag = AGPGameplayTags::Event_Character_Reaction_Death_Dying;
			PayloadEventData.Instigator = SourceAvatarActor;
			PayloadEventData.Target = TargetAvatarActor;

			/* Target의 DeathMontage를 선택(TODO : AbilityTargetDataFromDeathCharacter와 로직 합치거나..?) */
			if (AAGP_CharacterBase* TargetCharacter = InGEProperties.TargetContext.CachedCharacter.Get())
			{
				if (UAGPCharPresentationComponent* CharacterPresentationComponent = TargetCharacter->GetCharacterPresentationComponent())
				{
					/* 랜덤하게 선택한 Death 몽타주 이름 추가 */
					PayloadEventData.TargetData.Add(new FGameplayAbilityTargetData_DeathAbility(CharacterPresentationComponent->GetRandomDeathPoseName()));
				}
			}

			//Death Event 알림
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetAvatarActor, PayloadEventData.EventTag, PayloadEventData);
		}
		else
		{
			//체력이 0 초과 => HitReact, KnockBack 등 처리
			//AGP_NET_LOG(this, LogAGPAttribute, Log, TEXT("[%s] Hit, Health[%.3f => %.3f], Damage[%f], HitDirection[%s], Angle[%f]"), *GetNameSafe(GetOwningActor()), OldHealth, NewCurrentHealth, DamageIncoming, *UEnum::GetValueAsString(HitDirection), AngleDifference);

			/* Target의 HitReact를 활성화하기위한 GameplayEventData 구성 */
			/* Instigator의 경우 SourceAvatarActor, OptionalObject로 EffectCauser를 넘긴다. */
			FGameplayEventData PayloadEventData;
			PayloadEventData.EventTag = AGPGameplayTags::Event_Character_Reaction_HitReact;
			PayloadEventData.Instigator = SourceAvatarActor;
			PayloadEventData.Target = TargetAvatarActor;
			PayloadEventData.OptionalObject = EffectCauser;

			/* Target의 HitReact Montage 및 Flag 설정 */
			if (AAGP_CharacterBase* TargetCharacter = InGEProperties.TargetContext.CachedCharacter.Get())
			{
				if (UAGPCharPresentationComponent* CharacterPresentationComponent = TargetCharacter->GetCharacterPresentationComponent())
				{
					/* Montage Type에 따라 다른 정보를 저장한다. */
					switch (CharacterPresentationComponent->GetHitReactMontageType())
					{
					case EAGPHitReactPlayMontageType::NotPlayMontage:
						{
							/* 몽타주 플레이 안함 */
						}
						break;

					case EAGPHitReactPlayMontageType::RandomMontage:
						{
							/* 랜덤하게 하나의 몽타주를 선택해서 보내기 */
							PayloadEventData.TargetData.Add(new FGameplayAbilityTargetData_HitReactRandom(CharacterPresentationComponent->GetRandomHitReactMontageKey()));
						}
						break;

					case EAGPHitReactPlayMontageType::DirectionalMontage:
						{
							/* Hit Direciton 정보 넘겨주기(방향에 따른 Hit React 몽타주 플레이) */
							PayloadEventData.TargetData.Add(new FGameplayAbilityTargetData_HitReactDirection(HitDirection));
						}
						break;
					}
				}
			}

			/* HitReact를 GameplayEvent를 통해 활성화한다 */
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetAvatarActor, PayloadEventData.EventTag, PayloadEventData);
		}

		/* HitReact - GameplayCue 활성화 */
		if (IAGPGCInterface* TargetGCInterface = Cast<IAGPGCInterface>(TargetAvatarActor))
		{
			/* Target의 HitReact Type을 가져와서 그에 맞는 GameplayCue를 활성화한다. */
			const FGameplayTag& GCHitReactTag = TargetGCInterface->GetGCHitReactTag();
			if (GCHitReactTag.IsValid())
			{
				if (TargetASC)
				{
					TargetASC->ExecuteGameplayCue(GCHitReactTag, InEffectSpec.GetContext());
				}

				//TargetASC가 유효하지 않은 경우? 전역 호출 필요?

			}
		}
	}
}
