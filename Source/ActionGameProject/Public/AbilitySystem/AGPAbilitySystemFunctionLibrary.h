// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Common/AGPCommonEnums.h"
#include "AGPAbilitySystemFunctionLibrary.generated.h"

class UAnimMontage;
class UAGPGameplayAbility;
class AAGPProjectileBase;
struct FAGPAbilityData;

/**
 *  Ability System 내에서 공통적으로 사용하는 Function 모음
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPAbilitySystemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/* Ability의 AvatarActor에서 UAGPCharPresentationComponent를 확인해서 Montage를 찾아서 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility", AutoCreateRefTerm = "InMontageName"))
	static UAnimMontage* GetMontageFromAvatarActor(UAGPGameplayAbility* InAbility, const FName& InMontageName);

	/* Ability의 AvatarActor에서 UAGPCharPresentationComponent를 확인해서 HitReactMontage(Random)를 찾아서 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility", AutoCreateRefTerm = "InMontageName"))
	static UAnimMontage* GetHitReactRandomMontageFromAvatarActor(UAGPGameplayAbility* InAbility, const FName& InMontageName);

	/* Ability의 AvatarActor에서 UAGPCharPresentationComponent를 확인해서 HitReactMontage(Direction)를 찾아서 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility", AutoCreateRefTerm = "InMontageName"))
	static UAnimMontage* GetHitReactDirectionMontageFromAvatarActor(UAGPGameplayAbility* InAbility, const EAGPDirection InDirection);

	/* Ability의 AvatarActor에서 UAGPCharPresentationComponent를 확인해서 DeathMontage를 찾아서 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility", AutoCreateRefTerm = "InDeathPoseName"))
	static UAnimMontage* GetDeathMontageFromAvatarActor(UAGPGameplayAbility* InAbility);

	/* Ability의 AvatarActor에서 UAGPCharPresentationComponent를 확인해서 RebirthMontage를 찾아서 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility", AutoCreateRefTerm = "InDeathPoseName"))
	static UAnimMontage* GetRebirthMontageFromAvatarActor(UAGPGameplayAbility* InAbility);

	/* 장비 아이템으로부터 부착된 CostumeActor 반환(InAbility의 AvatarCharacter) */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility", AutoCreateRefTerm = "InEquipmentSlotTag, InCostumeActorName", Categories = "Items.Slots.Equipment"))
	static AAGPCostumeActorBase* GetAttachedCostumeActorEquipment(UAGPGameplayAbility* InAbility, const FGameplayTag& InEquipmentSlotTag, const FName& InCostumeActorName);

	/* Ability내에서 특정 Actor에 대한 Team 반응(우호적, 적대적, 중립)을 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility"))
	static ETeamAttitude::Type GetTargetAttitudeTowards(const UAGPGameplayAbility* InAbility, const AActor* InOtherActor);

	/* TargetActor에 대한 데미지 처리 타입 결정 */
	UFUNCTION(BlueprintCallable, Category = "AGP|AbilitySystem", meta = (ExpandEnumAsExecs = "OutDamageApplyType"))
	static void CheckApplyDamageTypeToTarget(AActor* InTargetActor, EAGPDamageApplyType& OutDamageApplyType);

	/* Target기준 Source의 방향 계산 (Front, Left, Back, Right) */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static EAGPDirection ComputeDirectionSourceToTarget(AActor* InSourceActor, AActor* InTargetActor, float& OutAngleDifference);

	/* 대상의 Death 여부 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static bool IsTargetDeath(AActor* InTargetActor);

	/* 대상의 특정 Tag존재 여부 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (AutoCreateRefTerm = "InCheckTag"))
	static bool HasTargetActorTag(AActor* InTargetActor, const FGameplayTag& InCheckTag);

	/* GameplayAbility내에서 특정 대상을 향해 AvatarActor 회전 */
	UFUNCTION(BlueprintCallable, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility"))
	static void RotateAvatarActorToTarget(UAGPGameplayAbility* InAbility, AActor* InTargetActor);

	/* GameplayAbility내에서 호출, Avatar Character의 Hit Flag설정 여부 확인*/
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility"))
	static bool CheckHitReactFlagAvatarCharacter(UAGPGameplayAbility* InAbility, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ActionGameProject.EAGPHitReactFlags")) int32 InBitMask);

	/* GameplayAbility내에서 호출, Avatar Character의 HitReact MontageType 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility"))
	static EAGPHitReactPlayMontageType GetHitReactPlayMontageType(UAGPGameplayAbility* InAbility);

	/* FGameplayAbilityTargetData_HitReact로부터 RandomHitReactMontage Name 반환(EAGPHitReactPlayMontageType::RandomMontage) */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static FName GetHitReactRandomMontageName(const FGameplayAbilityTargetDataHandle& TargetData, int32 Index);

	/* FGameplayAbilityTargetData_HitReact로부터 HitDirection 반환 (EAGPHitReactPlayMontageType::DirectionalMontage) */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static EAGPDirection GetHitReactDirectionFromTargetData(const FGameplayAbilityTargetDataHandle& TargetData, int32 Index);

	/* GameplayAbility내에서 호출, Avatar Character의 Death Flag설정 여부 확인*/
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility"))
	static bool CheckDeathFlagAvatarCharacter(UAGPGameplayAbility* InAbility, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ActionGameProject.EAGPDeathFlags")) int32 InBitMask);

	/* FGameplayAbilityTargetData_DeathAbility로부터 DeathPoseName 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static FName GetDeathPoseNameFromTargetData(const FGameplayAbilityTargetDataHandle& TargetData, int32 Index);

	/* FAGPGameplayEffectContext, GC_AttackTypeTag 설정 */
	UFUNCTION(BlueprintCallable, Category = "AGP|AbilitySystem", Meta = (AutoCreateRefTerm = "InAttackType", Categories = "GameplayCue.AttackType"))
	static FGameplayEffectContextHandle SetGCAttackType(FGameplayEffectContextHandle EffectContextHandle, const FGameplayTag& InAttackType);

	/* FAGPGameplayEffectContext, GC_AttackTypeTag 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static const FGameplayTag& GetGCAttackType(FGameplayEffectContextHandle EffectContextHandle);

	/* Ability내에서 Projectile 생성 */
	UFUNCTION(BlueprintCallable, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility", rereateRefTerm = "AttackTypeTag", Categories = "GameplayCue.AttackType"))
	static void SpawnProjectile(UAGPGameplayAbility* InAbility, 
								TSubclassOf<AAGPProjectileBase> InPrjectileClass,
								const FTransform& Trnasform, 
								ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, 
								ESpawnActorScaleMethod TransformScaleMethod, 
								TSubclassOf<UGameplayEffect> ApplyDamageGEClass, 
								float ApplyDamageGELevel,
								const FGameplayTag& AttackTypeTag);

	/* 특정 Ability의 AvatarPawn으로부터 LastMovementInput 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility"))
	static FVector GetLastMovementInputFromAvatar(UAGPGameplayAbility* InAbility);

	/* 대상을 향한 공격이 방어되었는지 체크 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static bool IsBlockedByTarget(AActor* InSource, AActor* InTarget, const float BlockAngleDegree = 50.f);
	
	/* 대상과 서로 마주 보고 있는지 체크 */
	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InAbility"))
	static bool IsFacingTarget(UAGPGameplayAbility* InAbility, AActor* InTarget, const float CheckAngleDegree = 135.f);

	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem", Meta = (DefaultToSelf = "InContext", AutoCreateRefTerm = "AbilityTag"))
	static const FAGPAbilityData& GetAbilityData(UObject* InContext, const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static FGameplayAbilityTargetDataHandle	AbilityTargetDataFromDeathCharacter(AAGP_CharacterBase* DeathCharacter);

	UFUNCTION(BlueprintPure, Category = "AGP|AbilitySystem")
	static FName GetDeathPoseNameFromCharacter(AAGP_CharacterBase* DeathCharacter);
};
