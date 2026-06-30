// KJY All Rights Reserved


#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "Characters/AGP_CharacterBase.h"
#include "Characters/Components/AGPCharPresentationComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AGPAbilityTypes.h"
#include "Projectile/AGPProjectileBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Subsystem/AGPGameDataTableSubsystem.h"

UAnimMontage* UAGPAbilitySystemFunctionLibrary::GetMontageFromAvatarActor(UAGPGameplayAbility* InAbility, const FName& InMontageName)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityAvatarCharacter)
	{
		return nullptr;
	}

	if (UAGPCharPresentationComponent* CharacterPresentationComponent = AbilityAvatarCharacter->GetCharacterPresentationComponent())
	{
		return CharacterPresentationComponent->GetMontageWithKey(InMontageName);
	}

	return nullptr;
}

UAnimMontage* UAGPAbilitySystemFunctionLibrary::GetHitReactRandomMontageFromAvatarActor(UAGPGameplayAbility* InAbility, const FName& InMontageName)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityAvatarCharacter)
	{
		return nullptr;
	}

	if (UAGPCharPresentationComponent* CharacterPresentationComponent = AbilityAvatarCharacter->GetCharacterPresentationComponent())
	{
		return CharacterPresentationComponent->GetRandomHitReactMontageWithKey(InMontageName);
	}

	return nullptr;
}

UAnimMontage* UAGPAbilitySystemFunctionLibrary::GetHitReactDirectionMontageFromAvatarActor(UAGPGameplayAbility* InAbility, const EAGPDirection InDirection)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityAvatarCharacter)
	{
		return nullptr;
	}

	if (UAGPCharPresentationComponent* CharacterPresentationComponent = AbilityAvatarCharacter->GetCharacterPresentationComponent())
	{
		return CharacterPresentationComponent->GetDirectionHitReactMontage(InDirection);
	}

	return nullptr;
}

UAnimMontage* UAGPAbilitySystemFunctionLibrary::GetDeathMontageFromAvatarActor(UAGPGameplayAbility* InAbility)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	return AbilityAvatarCharacter ? AbilityAvatarCharacter->GetDeathMontageFromDeathPose() : nullptr;
}

UAnimMontage* UAGPAbilitySystemFunctionLibrary::GetRebirthMontageFromAvatarActor(UAGPGameplayAbility* InAbility)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	return AbilityAvatarCharacter ? AbilityAvatarCharacter->GetRebirthMontageFromDeathPose() : nullptr;
}

AAGPCostumeActorBase* UAGPAbilitySystemFunctionLibrary::GetAttachedCostumeActorEquipment(UAGPGameplayAbility* InAbility, const FGameplayTag& InEquipmentSlotTag, const FName& InCostumeActorName)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityAvatarCharacter)
	{
		return nullptr;
	}

	if (UAGPCharPresentationComponent* CharacterPresentationComponent = AbilityAvatarCharacter->GetCharacterPresentationComponent())
	{
		FCostumeInstanceIdentifierHandle CostumeIdentifierHandle;
		CostumeIdentifierHandle.SetCostumeInstanceIdentifier(MakeShared<FCostumeInstanceEquipmentIdentifier>(InEquipmentSlotTag));
		return CharacterPresentationComponent->GetAttachedCostumeActor(CostumeIdentifierHandle, InCostumeActorName);
	}

	return nullptr;
}


ETeamAttitude::Type UAGPAbilitySystemFunctionLibrary::GetTargetAttitudeTowards(const UAGPGameplayAbility* InAbility, const AActor* InOtherActor)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	if (!InOtherActor)
	{
		return ETeamAttitude::Type::Neutral;
	}

	if (IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(InAbility->GetOwningActorFromActorInfo()))
	{
		return OwnerTeamInterface->GetTeamAttitudeTowards(*InOtherActor);
	}

	return ETeamAttitude::Type::Neutral;
}

void UAGPAbilitySystemFunctionLibrary::CheckApplyDamageTypeToTarget(AActor* InTargetActor, EAGPDamageApplyType& OutDamageApplyType)
{
	/* 대상이 존재하고 ASC존재 => GameplayEffect Apply 처리 */
	if (InTargetActor && UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InTargetActor))
	{
		OutDamageApplyType = EAGPDamageApplyType::ApplyGameplayEffect;
		return;
	}

	/* 그외는 기본적인 데미지 Apply 처리 */
	OutDamageApplyType = EAGPDamageApplyType::ApplyStandardDamageType;
}

EAGPDirection UAGPAbilitySystemFunctionLibrary::ComputeDirectionSourceToTarget(AActor* InSourceActor, AActor* InTargetActor, float& OutAngleDifference)
{
	checkf(InSourceActor, TEXT("[%s], Invalid InSourceActor"), __FUNCTIONW__);
	checkf(InTargetActor, TEXT("[%s], Invalid InTargetActor"), __FUNCTIONW__);

	/* Target의 Forward Vector */
	const FVector TargetForward = InTargetActor->GetActorForwardVector();

	/* Target으로부터 Source로 향하는 Vector */
	const FVector TargetToSource = (InSourceActor->GetActorLocation() - InTargetActor->GetActorLocation()).GetSafeNormal();

	/* 두 벡터간의 각도 계산 1 : 내적 */
	const float DotResult = FVector::DotProduct(TargetForward, TargetToSource);
	float AngleDifference = UKismetMathLibrary::DegAcos(DotResult);

	//외적 : 언리얼은 왼손법칙을 사용
	//두번째벡터가 오른쪽을 향할때(3번째 손가락) 결과벡터(엄지손가락)은 위로 향하게 된다. 
	const FVector CrossUpVector = FVector::CrossProduct(TargetForward, TargetToSource);

	//결과 벡터가 아래를 향하게 된다면(z가 0미만) 내적으로 얻은 각도 차이를 음수로 뒤집는다.
	AngleDifference = (CrossUpVector.Z >= 0.f) ? AngleDifference : -AngleDifference;

	/* 45도 범위에 따라 방향(Front, Left, Back, Right) 결정 */
	EAGPDirection ResultDirection = EAGPDirection::Front;

	if (AngleDifference >= -45.f && AngleDifference <= 45.f)
	{
		ResultDirection = EAGPDirection::Front;
	}
	else if (AngleDifference < -45.f && AngleDifference >= -135.f)
	{
		ResultDirection = EAGPDirection::Left;
	}
	else if (AngleDifference < -135.f || AngleDifference > 135.f)
	{
		ResultDirection = EAGPDirection::Back;
	}
	else// if (AngleDifference > 45.f || AngleDifference <= 135.f)
	{
		ResultDirection = EAGPDirection::Right;
	}

	OutAngleDifference = AngleDifference;

	return ResultDirection;
	
}

bool UAGPAbilitySystemFunctionLibrary::IsTargetDeath(AActor* InTargetActor)
{
	/* 1. Character의 Death State 확인 */
	if (AAGP_CharacterBase* TargetCharacter = Cast<AAGP_CharacterBase>(InTargetActor))
	{
		return TargetCharacter->IsDeath();
	}

	/* 2. State_Death Tag여부로 판단 */
	return HasTargetActorTag(InTargetActor, AGPGameplayTags::State_Death);
}

bool UAGPAbilitySystemFunctionLibrary::HasTargetActorTag(AActor* InTargetActor, const FGameplayTag& InCheckTag)
{
	if (IAbilitySystemInterface* TargetAbilitySystemInterface = Cast<IAbilitySystemInterface>(InTargetActor))
	{
		if (UAbilitySystemComponent* TargetASC = TargetAbilitySystemInterface->GetAbilitySystemComponent())
		{
			return TargetASC->HasMatchingGameplayTag(InCheckTag);
		}
	}

	return false;
}

void UAGPAbilitySystemFunctionLibrary::RotateAvatarActorToTarget(UAGPGameplayAbility* InAbility, AActor* InTargetActor)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	if (!InTargetActor)
	{
		return;
	}

	AActor* AvatarActor = InAbility->GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	const FRotator AvatarActorRotation = UKismetMathLibrary::FindLookAtRotation(AvatarActor->GetActorLocation(), InTargetActor->GetActorLocation());
	AvatarActor->SetActorRotation(AvatarActorRotation);
}

bool UAGPAbilitySystemFunctionLibrary::CheckHitReactFlagAvatarCharacter(UAGPGameplayAbility* InAbility, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ActionGameProject.EAGPHitReactFlags")) int32 InBitMask)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityAvatarCharacter)
	{
		return false;
	}

	if (UAGPCharPresentationComponent* CharacterPresentationComponent = AbilityAvatarCharacter->GetCharacterPresentationComponent())
	{
		return CharacterPresentationComponent->CheckHitReactFlag(InBitMask);
	}

	return false;
}

EAGPHitReactPlayMontageType UAGPAbilitySystemFunctionLibrary::GetHitReactPlayMontageType(UAGPGameplayAbility* InAbility)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityAvatarCharacter)
	{
		return EAGPHitReactPlayMontageType::NotPlayMontage;
	}

	if (UAGPCharPresentationComponent* CharacterPresentationComponent = AbilityAvatarCharacter->GetCharacterPresentationComponent())
	{
		return CharacterPresentationComponent->GetHitReactMontageType();
	}

	return EAGPHitReactPlayMontageType::NotPlayMontage;
}

FName UAGPAbilitySystemFunctionLibrary::GetHitReactRandomMontageName(const FGameplayAbilityTargetDataHandle& TargetData, int32 Index)
{
	/* Index 확인 */
	if (!TargetData.Data.IsValidIndex(Index))
	{
		return FName();
	}

	/* FGameplayAbilityTargetData_HitReactRandom로 부터 Montage Name 반환 */
	FGameplayAbilityTargetData_HitReactRandom* HitReact = static_cast<FGameplayAbilityTargetData_HitReactRandom*>(TargetData.Data[Index].Get());
	if (HitReact)
	{
		return HitReact->RandomHitReactMontageKeyName;
	}

	return FName();
}

EAGPDirection UAGPAbilitySystemFunctionLibrary::GetHitReactDirectionFromTargetData(const FGameplayAbilityTargetDataHandle& TargetData, int32 Index)
{
	/* Index 확인 */
	if (!TargetData.Data.IsValidIndex(Index))
	{
		return EAGPDirection::Front;
	}

	/* FGameplayAbilityTargetData_HitReact로 부터 HitDirection반환 */
	FGameplayAbilityTargetData_HitReactDirection* HitReact = static_cast<FGameplayAbilityTargetData_HitReactDirection*>(TargetData.Data[Index].Get());
	if (HitReact)
	{
		return HitReact->HitDirection;
	}

	return EAGPDirection::Front;
}

bool UAGPAbilitySystemFunctionLibrary::CheckDeathFlagAvatarCharacter(UAGPGameplayAbility* InAbility, int32 InBitMask)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	AAGP_CharacterBase* AbilityAvatarCharacter = Cast<AAGP_CharacterBase>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityAvatarCharacter)
	{
		return false;
	}

	if (UAGPCharPresentationComponent* CharacterPresentationComponent = AbilityAvatarCharacter->GetCharacterPresentationComponent())
	{
		return CharacterPresentationComponent->CheckDeathFlag(InBitMask);
	}

	return false;
}


FName UAGPAbilitySystemFunctionLibrary::GetDeathPoseNameFromTargetData(const FGameplayAbilityTargetDataHandle& TargetData, int32 Index)
{
	/* Index 확인 */
	if (!TargetData.Data.IsValidIndex(Index))
	{
		return NAME_None;
	}

	/* FGameplayAbilityTargetData_DeathAbility로부터 DeathPoseName반환 */
	FGameplayAbilityTargetData_DeathAbility* TargetData_DeathAbility = static_cast<FGameplayAbilityTargetData_DeathAbility*>(TargetData.Data[Index].Get());
	if (TargetData_DeathAbility)
	{
		return TargetData_DeathAbility->DeathPoseName;
	}

	return NAME_None;
}

FGameplayEffectContextHandle UAGPAbilitySystemFunctionLibrary::SetGCAttackType(FGameplayEffectContextHandle EffectContextHandle, const FGameplayTag& InAttackType)
{
	if (FAGPGameplayEffectContext* AGPEffectContext = static_cast<FAGPGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AGPEffectContext->SetGCAttackType(InAttackType);
	}

	return EffectContextHandle;
}

const FGameplayTag& UAGPAbilitySystemFunctionLibrary::GetGCAttackType(FGameplayEffectContextHandle EffectContextHandle)
{
	if (const FAGPGameplayEffectContext* AGPEffectContext = static_cast<const FAGPGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AGPEffectContext->GetGCAttackType();
	}

	return FGameplayTag::EmptyTag;
}

void UAGPAbilitySystemFunctionLibrary::SpawnProjectile(UAGPGameplayAbility* InAbility, 
														TSubclassOf<AAGPProjectileBase> InPrjectileClass, 
														const FTransform& Trnasform, 
														ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, 
														ESpawnActorScaleMethod TransformScaleMethod,
														TSubclassOf<UGameplayEffect> ApplyDamageGEClass,
														float ApplyDamageGELevel,
														const FGameplayTag& AttackTypeTag)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	UWorld* CurrentWorld = InAbility->GetWorld();
	checkf(CurrentWorld, TEXT("[%s], Invalid CurrentWorld"), __FUNCTIONW__);

	AActor* AbilityOwnerActor = InAbility->GetOwningActorFromActorInfo();
	if (!AbilityOwnerActor)
	{
		return;
	}

	UAbilitySystemComponent* OwnerASC = InAbility->GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC)
	{
		return;
	}

	APawn* AbilityInstigator = Cast<APawn>(InAbility->GetAvatarActorFromActorInfo());
	if (!AbilityInstigator)
	{
		return;
	}

	AAGPProjectileBase* SpawnedProjectile = CurrentWorld->SpawnActorDeferred<AAGPProjectileBase>(InPrjectileClass,
																								Trnasform,
																								AbilityOwnerActor,
																								AbilityInstigator,
																								CollisionHandlingOverride,
																								TransformScaleMethod);

	checkf(SpawnedProjectile, TEXT("[%s], Invalid SpawnedProjectile"), __FUNCTIONW__);

	/* GameplayEffectContext 생성 및 EffectCauser, AttackType 지정 */
	FGameplayEffectContextHandle ApplyDamageGEContextHandle = OwnerASC->MakeEffectContext();
	FGameplayEffectContext* ApplyDamageGEContext = ApplyDamageGEContextHandle.Get();
	checkf(ApplyDamageGEContext, TEXT("[%s], Invalid ApplyDamageGEContext"), __FUNCTIONW__);
	ApplyDamageGEContext->SetEffectCauser(SpawnedProjectile);
	SetGCAttackType(ApplyDamageGEContextHandle, AttackTypeTag);
	
	/* GameplayEffect Spec 생성 */
	FGameplayEffectSpecHandle ApplyDamageGESpecHandle = OwnerASC->MakeOutgoingSpec(ApplyDamageGEClass, ApplyDamageGELevel, ApplyDamageGEContextHandle);
	checkf(ApplyDamageGESpecHandle.IsValid(), TEXT("[%s], Invalid ApplyDamageGESpecHandle"), __FUNCTIONW__);

	/* Projectile에 Apply Damage GameplayEffect Spec 지정 */
	SpawnedProjectile->SetDamageGESpecHandle(ApplyDamageGESpecHandle);

	UGameplayStatics::FinishSpawningActor(SpawnedProjectile, Trnasform);
}

FVector UAGPAbilitySystemFunctionLibrary::GetLastMovementInputFromAvatar(UAGPGameplayAbility* InAbility)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	APawn* AvatarPawn = Cast<APawn>(InAbility->GetAvatarActorFromActorInfo());

	return AvatarPawn ? AvatarPawn->GetLastMovementInputVector() : FVector::ZeroVector;
}

bool UAGPAbilitySystemFunctionLibrary::IsBlockedByTarget(AActor* InSource, AActor* InTarget, const float BlockAngleDegree)
{
	checkf(InSource, TEXT("[%s], Invalid InSource"), __FUNCTIONW__);

	/* 대상이 방어중이 아니면 검사하지 않음 */
	if (!InTarget || !HasTargetActorTag(InTarget, AGPGameplayTags::State_ActiveAbility_Defending))
	{
		return false;
	}

	const FVector& AttackerLocation = InSource->GetActorLocation();
	const FVector& TargetLocation = InTarget->GetActorLocation();
	const FVector ToAttackerDirection = (AttackerLocation - TargetLocation).GetSafeNormal();
	const FVector& TargetForward = InTarget->GetActorForwardVector();

	/* 대상의 방향과 대상 => 공격자를 향하는 방향을 비교한다.(각도 이내 전방 판단) */
	const float DotResult = FVector::DotProduct(TargetForward, ToAttackerDirection);
	return DotResult >= FMath::Cos(FMath::DegreesToRadians(BlockAngleDegree));
}

bool UAGPAbilitySystemFunctionLibrary::IsFacingTarget(UAGPGameplayAbility* InAbility, AActor* InTarget, const float CheckAngleDegree)
{
	checkf(InAbility, TEXT("[%s], Invalid InAbility"), __FUNCTIONW__);

	if (!InTarget)
	{
		return false;
	}

	AActor* AvatarActor = InAbility->GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	const float DotResult = FVector::DotProduct(AvatarActor->GetActorForwardVector(), InTarget->GetActorForwardVector());
	return DotResult <= FMath::Cos(FMath::DegreesToRadians(CheckAngleDegree));
}

const FAGPAbilityData& UAGPAbilitySystemFunctionLibrary::GetAbilityData(UObject* InContext, const FGameplayTag& AbilityTag)
{
	checkf(InContext, TEXT("[%s], Invalid InContext"), __FUNCTIONW__);

	UWorld* WorldContext = InContext->GetWorld();
	checkf(WorldContext, TEXT("[%s], Invalid WorldContext"), __FUNCTIONW__);

	UAGPGameDataTableSubsystem* GameDataTableSubsystem = UGameInstance::GetSubsystem<UAGPGameDataTableSubsystem>(WorldContext->GetGameInstance());
	checkf(GameDataTableSubsystem, TEXT("[%s], Invalid GameDataTableSubsystem"), __FUNCTIONW__);

	return GameDataTableSubsystem->GetAbilityData(AbilityTag);
}

FGameplayAbilityTargetDataHandle UAGPAbilitySystemFunctionLibrary::AbilityTargetDataFromDeathCharacter(AAGP_CharacterBase* DeathCharacter)
{
	FGameplayAbilityTargetData_DeathAbility* TargetData = new FGameplayAbilityTargetData_DeathAbility(GetDeathPoseNameFromCharacter(DeathCharacter));
	checkf(TargetData, TEXT("[%s], Invalid TargetData"), __FUNCTIONW__);

	FGameplayAbilityTargetDataHandle Handle;
	Handle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(TargetData));

	return Handle;
}

FName UAGPAbilitySystemFunctionLibrary::GetDeathPoseNameFromCharacter(AAGP_CharacterBase* DeathCharacter)
{
	if (!DeathCharacter)
	{
		return NAME_None;
	}

	UAGPCharPresentationComponent* CharacterPresentationComponent = DeathCharacter->GetCharacterPresentationComponent();
	if (!CharacterPresentationComponent)
	{
		return NAME_None;
	}

	return CharacterPresentationComponent->GetRandomDeathPoseName();
}