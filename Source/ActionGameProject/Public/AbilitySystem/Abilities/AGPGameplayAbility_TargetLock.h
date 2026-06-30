// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AGPGameplayAbility_TargetLock.generated.h"

class UCameraComponent;
class UAGPUserWidget;
class UAGPTargetLockWidgetController;
class AAGP_CharacterBase;
class UAGPInputConfig;

/**
 * 타겟 잠금 기능과 관련된 Ability
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPGameplayAbility_TargetLock : public UAGPGameplayAbility
{
	GENERATED_BODY()
	
private:
	enum class eGATargetLock_Direction : uint8
	{
		Left = 0,
		Right
	};
	
private:
	/* 캐릭터 주변 대상을 찾기위한 구체 크기 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace")
	float TargetScanRadius = 5000.f;

	/* 시야 내 대상 필터링을 위한 프러스텀 길이(Far) */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace")
	float TargetScanFrustumDistance = 4500.f;

	/* 타겟 대상 탐색 시 SphereTrace Query Type */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace")
	TArray<TEnumAsByte<EObjectTypeQuery>> SphereTraceChannel;

	/* 디버깅용 Trace Draw 표시 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace")
	bool bDrawTraceForDebug = false;

	/* 디버깅용 Sphere Draw 표시할때 Segments */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace", meta = (EditCondition = "bDrawSphereForDebug", EditConditionHides))
	int32 DrawSphereDebugSegments = 16;

	/* 디버깅용 Sphere Draw 표시할때 색상 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace", meta = (EditCondition = "bDrawSphereForDebug", EditConditionHides))
	FColor DrawSphereDebugColor = FColor::Green;

	/* 디버깅용 Frustum Draw 표시할때 색상 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace", meta = (EditCondition = "bDrawSphereForDebug", EditConditionHides))
	FColor DrawFrustumDebugColor = FColor::Red;

	/* 디버깅용 Draw 표시할때 계속표시여부 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace", meta = (EditCondition = "bDrawSphereForDebug", EditConditionHides))
	bool bDrawDebugPersistentLines = false;

	/* 디버깅용 Draw 표시할때 제한시간 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock|SphereTrace", meta = (EditCondition = "bDrawSphereForDebug", EditConditionHides))
	float DrawDebugLifeTime = 2.0f;

	/* 타겟 대상 탐색 활성화시 SwitchTarget을 위한 별도 입력 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAGPInputConfig> InputConfig;

	/* 타겟전환시 입력 민감도 */
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float LastInputThreshold = 0.5f;

	/* 타겟전환 제한 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.0", ClampMax = "3.0", UIMin = "0.0", UIMax = "3.0"))
	float SwitchTargetCooldownTime = 0.2f;

private:
	/* SwitchTarget 맵핑된 입력들에 대한 핸들 */
	TArray<uint32> BindingInputHandles;

	/* SwitchTarget 입력으로 들어온 XY 2D 값 */
	FVector2D LastSwitchTargetInput = FVector2D::ZeroVector;

	/* 마지막에 타겟설정된 시간 */
	double LastSetTargetTime = 0.0;

private:
	/* 현재 타겟 잠금 중인 대상 */
	UPROPERTY()
	TObjectPtr<AAGP_CharacterBase> CurrentLockedTargetCharacter;
	
public:
	UAGPGameplayAbility_TargetLock();

protected:
	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface

public:
	virtual void OnNotifyInputTriggered(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void OnNotifyInputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);

private:
	/* 카메라시야로부터 타겟가능한 대상들 중 가장 적절한 대상 반환 */
	AAGP_CharacterBase* GetBestAvailableTargetActor(TFunctionRef<bool(AAGP_CharacterBase*)> InCondition = [](AAGP_CharacterBase*) {return true;}) const;

	/* 새로운 타겟 설정 */
	void SetNewTargetCharacter(AAGP_CharacterBase* InNewTargetCharacter);

	/* 지정된 타겟 초기화 */
	void ClearCurrentTargetCharacter();

	/* 타겟의 위치가 갱신되었을 때 호출 */
	void OnChangedTargetLocation();

	/* Ability 실행 취소 */
	void TargetLockAbilityCancel();

	/* 컨트롤 중인 캐릭터에게 TargetLock 해제 알림 */
	void NotifyTargetLockEndToAvatar();

	/* 타겟 전환 입력 맵핑 */
	void SwitchTargetInputMapping();

	/* 타겟 전환 입력 맵핑 해제 */
	void SwitchTargetInputUnmapping();

	/* SwitchTarget Input이 Trigger 되었을때 호출 */
	void OnSwitchTargetInputTriggered(const FVector2D InNewInput);

	/* SwitchTarget Input이 Release되었을때 호출 */
	void OnSwitchTargetInputReleased();

	/* 타겟 전환 */
	void SwitchTarget();

	/* 마지막 SwitchTarget Input값 초기화 */
	void ResetSwitchTargetInput();

	/* LastSetTargetTime 초기화 */
	void ResetSetTargetTime();

private:
	/* 현재 AvatarCharacter로부터의 카메라정보(FMinimalViewInfo) 반환 */
	bool GetCameraViewInfoFromAvatarCharacter(FMinimalViewInfo& OutViewInfo) const;

	/* 현재 카메라시야로부터 뷰프러스텀 반환 */
	bool GetCurrentCameraViewFrustum(FConvexVolume& OutFrustum) const;

private:
	/* 타겟의 위치 이동에 대한 델리게이트 호출 함수 */
	UFUNCTION()
	void OnTargetMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	/* 타겟의 Destroy에 대한 델리게이트 호출 함수 */
	UFUNCTION()
	void OnTargetActorDestroyed(AActor* DestroyedActor);

	/* 타겟의 Death 이벤트에 대한 델리게이트 호출 함수 */
	UFUNCTION()
	void OnTargetDeathStateChanged(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState);
};