// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "Item/Interfaces/AGPItemSystemExternalInterface.h"
#include "Collision/AGPCollisionHandleInterface.h"
#include "AbilitySystem/GameplayCue/AGPGCInterface.h"
#include "GenericTeamAgentInterface.h"
#include "AGPGameplayTags.h"
#include "Common/AGPCommonStructs.h"
#include "AGP_CharacterBase.generated.h"

class UAGPItemSlot;
class UAGPItemInstance;
class UAGP_AbilitySystemComponent;
class UAGPAttributeSetCommon;
class UAGPAttributeSetBase;
class UAGPItemSystemsFacadeComponent;
class UAGPCharPresentationComponent;
class UAGPLinkedAnimLayerBase;
class UAGPCollisionHandleComponent;
class UAGPCharacterMovementComponent;
class UAGPStateTagNotifierComponent;
class UGameplayEffect;
class UAGPWidgetComponent;
class UWidgetComponent;
class UMotionWarpingComponent;


/* 캐릭터 사망 상태 업데이트에 대한 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateDeathStateSignature, AAGP_CharacterBase*, UpdatedCharacter, EAGPCharacterDeath, UpdateState);

/**
 * 게임 내 모든 캐릭터들의 Base 클래스
 */

UCLASS(Abstract, NotBlueprintable)
class ACTIONGAMEPROJECT_API AAGP_CharacterBase : public ACharacter
												, public IAbilitySystemInterface
												, public IAGPItemSystemExternalInterface
												, public IAGPCollisionHandleInterface
												, public IAGPGCInterface
												, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
private:
	static const FName DeathPawnCollisionProfileName;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UAGPCharPresentationComponent> CharacterPresentationComponent;

	/* 캐릭터에 추가되는 여러 Collision 관리 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UAGPCollisionHandleComponent> CollisionHandleComponent;

	/* 캐릭터마다 표시되는 상태 위젯 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UAGPWidgetComponent> FloatingWidgetComponent;

	/* 캐릭터마다 표시되는 타겟잠금 위젯 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> TargetLockWidgetComponent;

	/* 캐릭터 무브먼트 컴포넌트(커스터마이징) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UAGPCharacterMovementComponent> CharacterMovementComponent;

	/* 모션워핑 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	/* 캐릭터에 추가/삭제되는 여러 State Tag 이벤트 수신 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TObjectPtr<UAGPStateTagNotifierComponent> StateTagNotifierComponent;

protected:
	/* Attribute 초기화 - Base 지정(ex : 최대 체력) */
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData")
	TArray<TSubclassOf<UGameplayEffect>> InitAttribute_BaseGEs;

	/* Attribute 초기화 - Base에 영향을 받는 Attribute들(ex : 현재 체력)*/
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData")
	TArray<TSubclassOf<UGameplayEffect>> InitAttribute_AffectedByBaseGEs;

	/* State초기화 GameplayEffects */
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData")
	TArray<TSubclassOf<UGameplayEffect>> InitStateGEs;

	/* 캐릭터 고유 AttributeSet Class */
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData")
	TSet<TSubclassOf<UAGPAttributeSetBase>> DynamicAttributeSetClasses;

	/* 캐릭터가 살아있는동안 활성화되는 Ability Tags */
	UPROPERTY(EditDefaultsOnly, Category = "StartUpData")
	FGameplayTagContainer PersistentAbilityTags;

	/* GameplayCue - 해당 캐릭터의 HitReact반응시 발생시킬 GameplayCue Tag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayCue", meta = (Categories = "GameplayCue.HitType"))
	FGameplayTag GCHitReactTag;

	/* 머리 위 Widget 표시 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget")
	bool ShowFloatingWidget = true;

protected:
	/* 캐릭터의 사망 상태 구분 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CharacterDeathState)
	FAGPCharacterDeathStatus CharacterDeathState;

	/* 타겟 잠금 이동 여부 */
	UPROPERTY(VisibleInstanceOnly, Replicated)
	bool IsMovementTargetLock = false;

	/* 타겟 잠금 대상 위치 */
	UPROPERTY(VisibleInstanceOnly)
	FVector TargetLockLocation;

public:
	/* 캐릭터 사망 상태에 대한 델리게이트 */
	UPROPERTY(BlueprintAssignable)
	FUpdateDeathStateSignature OnUpdateDeathState;

public:
	AAGP_CharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const override;

public:
	/* 캐릭터 사망 시작(Dying 설정)*/
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "InDeathPoseName"))
	void SetCharacterDeath_Dying(const FName& InDeathPoseName);

	/* 캐릭터 사망 상태 유지 (Death 설정) */
	UFUNCTION(BlueprintCallable)
	void SetCharacterDeath_Death();

	/* 캐릭터 사망 => Destroy 설정 */
	UFUNCTION(BlueprintCallable)
	void SetCharacterDeath_Destroy();

	/* 캐릭터 사망 => Rebirth 설정 */
	UFUNCTION(BlueprintCallable)
	void SetCharacterDeath_Rebirth();

	/* 캐릭터 사망 상태 리셋 (None 설정) */
	UFUNCTION(BlueprintCallable)
	void SetCharacterDeath_None();

	/* 애니메이션 블루프린트의 FinishDeath Event */
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "InEventTag"))
	void OnAnimNotify_DeathEventCall(const FGameplayTag& InEventTag);

	/* 모션워핑 타겟 위치 지정 */
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "InMotionWarpTargetName, InTargetLocation"))
	void SetMotionWarpTargetLocation(const FName& InMotionWarpTargetName, const FVector& InTargetLocation);

	/* 모션워핑 타겟 위치 및 회전 지정 */
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "InMotionWarpTargetName, InTargetLocation, InTargetRotation"))
	void SetMotionWarpTargetLocationAndRotation(const FName& InMotionWarpTargetName, const FVector& InTargetLocation, const FRotator& InTargetRotation);

	/* 모션워핑 타겟 제거 */
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "InMotionWarpTargetName"))
	void RemoveWarpTarget(const FName& InMotionWarpTargetName);

protected:
	virtual void InitializeAbilitySystem(AActor* InOwnerActor);
	virtual void UninitializeAbilitySystem();

	virtual void InitializeAttribute();
	virtual void InitializeState();
	virtual void InitializeActivateAbility();

	/* 캐릭터와 연결된 UI 초기화 및 연결 해제 */
	virtual void InitailizeCharacterWidget();
	virtual void UninitializeCharacterWidget();

protected:
	void SetCharacterCapsuleCollisionProfile(const FName& InCollisionProfile);
	void SetCharacterMoveLock(const bool IsLock);
	void SetFloatingWidgetVisiblityIfAlive();
	
protected:
	/* 변경된 Death 상태 델리게이트 알림 */
	void BroadcastChangeDeathState();

protected:
	UFUNCTION()
	virtual void OnRep_CharacterDeathState();

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnChangeDeathState_Dying();

	UFUNCTION(BlueprintNativeEvent)
	void OnChangeDeathState_Death();

	UFUNCTION(BlueprintNativeEvent)
	void OnChangeDeathState_Destroy(bool IsShowDissolveFX);

	UFUNCTION(BlueprintNativeEvent)
	void OnChangeDeathState_Rebirth();

	UFUNCTION(BlueprintNativeEvent)
	void OnChangeDeathState_None();

public:
	/* 대상 위치를 향한 타겟잠금이동 활성화 */
	void StartTargetLockMove();

	/* 타겟잠금 위치 갱신 */
	void UpdateTargetLockPostion(const FVector& InTargetLocation);

	/* 타겟잠금이동 초기화 */
	void StopTargetLockMove();

	/* 타겟잠금 대상이 되었음을 알림 */
	void OnNotifyTargetLocked();

	/* 타겟잠금 대상에서 해제되었음을 알림 */
	void OnNotifyTargetLockRelease();

	/* Skeletal Mesh 강제 갱신 */
	void RefreshSkeletalMeshBone(bool bNeedsValidRootMotion);

	/* 뷰프러스텀 내에 캐릭터 포함 여부 반환 */
	bool IsInsideFrustum(const FConvexVolume& InViewFrustum) const;

	/* ASC에 지정한 태그 존재 여부 반환 */
	bool HasTagInASC(const FGameplayTag& InCheckTag) const;

public:
	virtual UAGP_AbilitySystemComponent* GetAGPAbilitySystemComponent() const PURE_VIRTUAL(AAGP_CharacterBase::GetAGPAbilitySystemComponent, return nullptr;);
	virtual UAGPAttributeSetCommon* GetCommonAttributeSet() const PURE_VIRTUAL(AAGP_CharacterBase::GetCommonAttributeSet, return nullptr;);

#pragma region[IAbilitySystemInterface]
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
#pragma endregion

#pragma region[IAGPItemSystemExternalInterface]
public:
	virtual UAGPItemSystemsFacadeComponent* GetItemSystemFacadeComponent() const override PURE_VIRTUAL(AAGP_CharacterBase::GetItemSystemFacadeComponent, return nullptr;);
#pragma endregion

#pragma region[IAGPItemSystemExternalInterface]
public:
	virtual UAGPCollisionHandleComponent* GetCollisionHandleComponent_Implementation() const override;
#pragma endregion

#pragma region[IAGPGCInterface]
public:
	virtual const FGameplayTag& GetGCHitReactTag() const override;

public:
	/* Hit Event 알림 수신 */
	virtual void OnHitEvent_Implementation() override;
#pragma endregion

#pragma region[IGenericTeamAgentInterface]
public:
	virtual FGenericTeamId GetGenericTeamId() const override PURE_VIRTUAL(AAGP_CharacterBase::GetGenericTeamId, return FGenericTeamId::NoTeam;);
#pragma endregion

public:
	/* 현재 DeathPose에 설정한 Death 몽타주 반환 */
	UAnimMontage* GetDeathMontageFromDeathPose() const;

	/* 현재 DeathPose에 설정한 Rebirth 몽타주 반환 */
	UAnimMontage* GetRebirthMontageFromDeathPose() const;

	/* Death 여부 반환 */
	bool IsDeath() const;
	
public:
	FORCEINLINE UAGPCharPresentationComponent* GetCharacterPresentationComponent() const { return CharacterPresentationComponent; }
	FORCEINLINE UAGPCharacterMovementComponent* GetCharacterMovementComponent() const { return CharacterMovementComponent; }

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE EAGPCharacterDeath GetDeathState() const { return CharacterDeathState.DeathState; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FName GetDeathPoseName() const { return CharacterDeathState.DeathPoseName; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsTargetLockMove() const { return IsMovementTargetLock; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FVector GetTargetLockLocation() const { return TargetLockLocation; }

private:
	/* 머리 위 Widget 표시 조건 검사 */
	bool IsVisibleHeadUPFloatingWidget() const;

	/* 플레이어 컨트롤 여부 반환 */
	bool IsPlayerControl() const;

};