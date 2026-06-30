// KJY All Rights Reserved


#include "AbilitySystem/Abilities/AGPGameplayAbility_TargetLock.h"
#include "AGPGameplayTags.h"
#include "Characters/AGP_PlayerCharacter.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/AGPUserWidget.h"
#include "Widget/WidgetController/AGPTargetLockWidgetController.h"
#include "Player/AGP_PlayerControllerBase.h"
#include "Input/AGPInputConfig.h"
#include "AGPGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Common/AGPCommonFunctionLibrary.h"
#include "Log/AGPLogChannels.h"

UAGPGameplayAbility_TargetLock::UAGPGameplayAbility_TargetLock()
{
	//AbilityTags.AddTag(AGPGameplayTags::Ability_PlayerControl_TargetLock);
	SetAssetTags(FGameplayTagContainer(AGPGameplayTags::Ability_PlayerControl_TargetLock));

	ActivationBlockedTags.AddTag(AGPGameplayTags::State_Death);

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::Type::LocalOnly;
}

void UAGPGameplayAbility_TargetLock::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/* 1. 타겟락온이 가능한 대상이 있으면 설정, 없으면 Ability 취소 */
	AAGP_CharacterBase* NewTargetCharacter = GetBestAvailableTargetActor();
	if (NewTargetCharacter)
	{
		SetNewTargetCharacter(NewTargetCharacter);
		SwitchTargetInputMapping();
	}
	else
	{
		TargetLockAbilityCancel();
	}
}

void UAGPGameplayAbility_TargetLock::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	NotifyTargetLockEndToAvatar();
	ClearCurrentTargetCharacter();

	ResetSetTargetTime();
	ResetSwitchTargetInput();
	SwitchTargetInputUnmapping();
}

void UAGPGameplayAbility_TargetLock::OnNotifyInputTriggered(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	Super::OnNotifyInputTriggered(Handle, ActorInfo, ActivationInfo, InputActionValue, InInputTag);

	/* 타겟잠금 키 입력 시 종료 */
	if (InInputTag.MatchesTagExact(AGPGameplayTags::Input_Ability_Active_TargetLock))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	/* 타겟 전환 키 입력 처리 */
	if (InInputTag.MatchesTagExact(AGPGameplayTags::Input_Ability_Event_TargetLock_SwitchTarget))
	{
		OnSwitchTargetInputTriggered(InputActionValue.Get<FVector2D>());
		return;
	}
}

void UAGPGameplayAbility_TargetLock::OnNotifyInputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	Super::OnNotifyInputReleased(Handle, ActorInfo, ActivationInfo, InputActionValue, InInputTag);

	/* 타겟 전환 키 입력 처리 */
	if (InInputTag.MatchesTagExact(AGPGameplayTags::Input_Ability_Event_TargetLock_SwitchTarget))
	{
		OnSwitchTargetInputReleased();
		return;
	}
}

AAGP_CharacterBase* UAGPGameplayAbility_TargetLock::GetBestAvailableTargetActor(TFunctionRef<bool(AAGP_CharacterBase*)> InCondition) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return nullptr;
	}

	/* 1. 현재 카메라 정보(카메라 월드 위치, 월드 회전)로부터 뷰 행렬, 투영행렬 얻기 */
	FMinimalViewInfo ViewInfo;
	if (!GetCameraViewInfoFromAvatarCharacter(ViewInfo))
	{
		return nullptr;
	}

	const FVector& CameraLocation = ViewInfo.Location;
	const FVector CameraForward = ViewInfo.Rotation.Vector();

	/* 1. Sphere Trace를 통해 타겟가능한 대상들을 1차적으로 검출 */
	TArray<AActor*> IgnoreActors = { AvatarActor };
	TArray<AActor*> SphereTraceScanActors;
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		CameraLocation,
		TargetScanRadius,
		SphereTraceChannel,
		AAGP_CharacterBase::StaticClass(),
		IgnoreActors,
		SphereTraceScanActors
	);

	/* 현재 View정보를 바탕으로 Camera 프러스텀 구성 */
	FConvexVolume ViewFrustum;
	GetCurrentCameraViewFrustum(ViewFrustum);

	if (bDrawTraceForDebug)
	{
		/* 디버깅용 Sphere 그리기 */
		DrawDebugSphere(
			GetWorld(),
			CameraLocation,
			TargetScanRadius,
			DrawSphereDebugSegments,
			DrawSphereDebugColor,
			bDrawDebugPersistentLines,
			DrawDebugLifeTime
		);

		/* 디버깅용 프러스텀 출력 */
		UAGPCommonFunctionLibrary::DrawFrustumForDebug(GetWorld(), ViewFrustum, DrawFrustumDebugColor, bDrawDebugPersistentLines, DrawDebugLifeTime);
	}

	/* 2. Sphere Trace 된 대상들중 뷰 중앙에 가장 근접한 대상을 찾는다. */
	float BestCenterDot = -1.f;
	AAGP_CharacterBase* BestTargetCharacter = nullptr;
	for (AActor* SphereScanActor : SphereTraceScanActors)
	{
		AAGP_CharacterBase* ScanTargetCharacter = Cast<AAGP_CharacterBase>(SphereScanActor);
		if (!ScanTargetCharacter)
		{
			continue;
		}

		/* 자기 자신이거나 현재 타겟과 같은 대상이면 제외 */
		if (AvatarActor == ScanTargetCharacter || ScanTargetCharacter == CurrentLockedTargetCharacter)
		{
			continue;
		}

		/* 대상이 적대적인지 확인 */
		if (ETeamAttitude::Type::Hostile != UAGPAbilitySystemFunctionLibrary::GetTargetAttitudeTowards(this, ScanTargetCharacter))
		{
			continue;
		}

		/* 대상이 사망했는지 확인 */
		if (UAGPAbilitySystemFunctionLibrary::IsTargetDeath(ScanTargetCharacter))
		{
			continue;
		}

		/* 대상이 카메라의 뷰프러스텀내에 위치하는지 확인 */
		if (!ScanTargetCharacter->IsInsideFrustum(ViewFrustum))
		{
			continue;
		}

		/* 대상에 대한 추가 조건 검사 */
		if (!InCondition(ScanTargetCharacter))
		{
			continue;
		}

		/* 현재 뷰 중앙에서 가장 가까운 대상을 선택 */
		const FVector CameraToTargetActorDirection = (ScanTargetCharacter->GetActorLocation() - CameraLocation).GetSafeNormal();
		const float CenterDot = FVector::DotProduct(CameraForward, CameraToTargetActorDirection);
		if (CenterDot > BestCenterDot)
		{
			BestCenterDot = CenterDot;
			BestTargetCharacter = ScanTargetCharacter;
		}
	}

	return BestTargetCharacter;
}

void UAGPGameplayAbility_TargetLock::SetNewTargetCharacter(AAGP_CharacterBase* InNewTargetCharacter)
{
	checkf(InNewTargetCharacter, TEXT("[%s], Invalid InNewTargetCharacter"), __FUNCTIONW__);

	AAGP_CharacterBase* AvatarCharacter = GetAvatarCharacter();
	checkf(AvatarCharacter, TEXT("[%s], Invalid AvatarCharacter"), __FUNCTIONW__);

	CurrentLockedTargetCharacter = InNewTargetCharacter;

	//새로운 타겟의 Destroy 이벤트에 바인딩
	CurrentLockedTargetCharacter->OnDestroyed.AddDynamic(this, &UAGPGameplayAbility_TargetLock::OnTargetActorDestroyed);

	//새로운 타겟의 Death 이벤트에 바인딩
	CurrentLockedTargetCharacter->OnUpdateDeathState.AddDynamic(this, &UAGPGameplayAbility_TargetLock::OnTargetDeathStateChanged);

	//새로운 타겟의 이동 이벤트에 대한 델리게이트 바인딩
	CurrentLockedTargetCharacter->OnCharacterMovementUpdated.AddDynamic(this, &UAGPGameplayAbility_TargetLock::OnTargetMovementUpdated);

	//타겟에게 타겟잠금대상이 되었음을 알림
	CurrentLockedTargetCharacter->OnNotifyTargetLocked();

	/* Avatar Character 타겟잠금 설정 */
	AvatarCharacter->StartTargetLockMove();

	// 새로운 타겟의 위치 반영 
	OnChangedTargetLocation();

	LastSetTargetTime = UGameplayStatics::GetTimeSeconds(GetWorld());
}

void UAGPGameplayAbility_TargetLock::ClearCurrentTargetCharacter()
{
	if (!CurrentLockedTargetCharacter)
	{
		return;
	}
	
	//타겟의 Destroy 이벤트 언바인딩
	CurrentLockedTargetCharacter->OnDestroyed.RemoveAll(this);

	//타겟의 Death 이벤트 언바인딩
	CurrentLockedTargetCharacter->OnUpdateDeathState.RemoveAll(this);

	//타겟의 이동 이벤트 언바인딩
	CurrentLockedTargetCharacter->OnCharacterMovementUpdated.RemoveAll(this);

	//타겟에게 타겟잠금대상에서 해제되었음을 알림
	CurrentLockedTargetCharacter->OnNotifyTargetLockRelease();

	CurrentLockedTargetCharacter = nullptr;
}

void UAGPGameplayAbility_TargetLock::OnChangedTargetLocation()
{
	if (!CurrentLockedTargetCharacter)
	{
		TargetLockAbilityCancel();
		return;
	}

	AAGP_CharacterBase* AvatarCharacter = GetAvatarCharacter();
	if (!AvatarCharacter)
	{
		TargetLockAbilityCancel();
		return;
	}

	//Avatar Character에 Target잠금 활성화 알림
	AvatarCharacter->UpdateTargetLockPostion(CurrentLockedTargetCharacter->GetActorLocation());
}

void UAGPGameplayAbility_TargetLock::TargetLockAbilityCancel()
{
	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

void UAGPGameplayAbility_TargetLock::NotifyTargetLockEndToAvatar()
{
	if (AAGP_CharacterBase* AvatarCharacter = GetAvatarCharacter())
	{
		AvatarCharacter->StopTargetLockMove();
	}
}

void UAGPGameplayAbility_TargetLock::SwitchTargetInputMapping()
{
	AAGP_PlayerControllerBase* AvatarPlayerController = GetAvatarPlayerController();
	if (!AvatarPlayerController)
	{
		return;
	}

	AvatarPlayerController->BindInputConfig(InputConfig, BindingInputHandles);
}

void UAGPGameplayAbility_TargetLock::SwitchTargetInputUnmapping()
{
	AAGP_PlayerControllerBase* AvatarPlayerController = GetAvatarPlayerController();
	if (!AvatarPlayerController)
	{
		return;
	}

	AvatarPlayerController->UnbindInputConfig(InputConfig, BindingInputHandles);
}

void UAGPGameplayAbility_TargetLock::OnSwitchTargetInputTriggered(const FVector2D InNewInput)
{
	LastSwitchTargetInput = InNewInput;
}

void UAGPGameplayAbility_TargetLock::OnSwitchTargetInputReleased()
{
	/* 타겟전환 쿨다운 타임 충족 여부 */
	const double NowTime = UGameplayStatics::GetTimeSeconds(GetWorld());
	const double DelayTime = NowTime - LastSetTargetTime;
	const bool bIsTargetSwitchCooldownReady = DelayTime >= SwitchTargetCooldownTime;

	/* 입력 크기 만족 여부 */
	const bool bIsInputAboveThreshold = FMath::Abs(LastSwitchTargetInput.X) >= LastInputThreshold;

	if(bIsTargetSwitchCooldownReady && bIsInputAboveThreshold)
	{
		/* 타겟전환 제한 시간(마지막 타겟 지정 이후 CooldownTime) */
		SwitchTarget();
	}
	
	ResetSwitchTargetInput();
}

void UAGPGameplayAbility_TargetLock::SwitchTarget()
{
	/* 마지막 입력 */
	const eGATargetLock_Direction CurrentSwitchDirection = LastSwitchTargetInput.X > 0.f ? eGATargetLock_Direction::Right : eGATargetLock_Direction::Left;

	FMinimalViewInfo ViewInfo;
	if (!GetCameraViewInfoFromAvatarCharacter(ViewInfo))
	{
		return;
	}

	const FVector CameraLocation = ViewInfo.Location;
	const FVector CameraRight = ViewInfo.Rotation.RotateVector(FVector::RightVector);

	AAGP_CharacterBase* BestTargetCharacter = GetBestAvailableTargetActor([CurrentSwitchDirection, CameraLocation, CameraRight](AAGP_CharacterBase* InAvailableTarget)
		->bool
		{
			if (!InAvailableTarget)
			{
				return false;
			}

			const FVector CameraToTargetActorDirection = (InAvailableTarget->GetActorLocation() - CameraLocation).GetSafeNormal();
			const float SideDot = FVector::DotProduct(CameraRight, CameraToTargetActorDirection);
			const eGATargetLock_Direction AvailableTargetActorDirection = SideDot < 0.f ? eGATargetLock_Direction::Left : eGATargetLock_Direction::Right;

			return AvailableTargetActorDirection == CurrentSwitchDirection;
		}
	);

	if (BestTargetCharacter)
	{
		/* 기존 타겟 해제 */
		ClearCurrentTargetCharacter();

		/* 새로운 타겟 설정 */
		SetNewTargetCharacter(BestTargetCharacter);
	}
}

void UAGPGameplayAbility_TargetLock::ResetSwitchTargetInput()
{
	LastSwitchTargetInput = FVector2D::ZeroVector;
}

void UAGPGameplayAbility_TargetLock::ResetSetTargetTime()
{
	LastSetTargetTime = 0.f;
}

bool UAGPGameplayAbility_TargetLock::GetCameraViewInfoFromAvatarCharacter(FMinimalViewInfo& OutViewInfo) const
{
	AAGP_PlayerCharacter* AvatarCharacter = Cast<AAGP_PlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarCharacter)
	{
		return false;
	}

	UCameraComponent* CameraComponent = AvatarCharacter->GetCameraComponent();
	if (!CameraComponent)
	{
		return false;
	}

	CameraComponent->GetCameraView(0.f, OutViewInfo);

	return true;
}

bool UAGPGameplayAbility_TargetLock::GetCurrentCameraViewFrustum(FConvexVolume& OutFrustum) const
{
	/* 1. 현재 카메라 정보(카메라 월드 위치, 월드 회전)로부터 뷰 행렬, 투영행렬 얻기 */
	FMinimalViewInfo ViewInfo;
	if (!GetCameraViewInfoFromAvatarCharacter(ViewInfo))
	{
		return false;
	}

	FMatrix ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;
	UGameplayStatics::GetViewProjectionMatrix(ViewInfo, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);

	const FVector CameraLocation = ViewInfo.Location;
	const FVector CameraForward = ViewInfo.Rotation.Vector();

	/* 2. 임의의 Far 평면을 생성(타겟잠금 범위 제한) */
	const FVector FarPlanePos = CameraLocation + (CameraForward * TargetScanFrustumDistance);
	const FPlane CustomFarPlane = FPlane(FarPlanePos, CameraForward);

	/* 3. 현재 뷰-투영행렬 + 커스텀 Far평면을 더한 뷰프러스텀 생성 */
	GetViewFrustumBounds(OutFrustum, ViewProjectionMatrix, CustomFarPlane, true, true);

	return true;
}

void UAGPGameplayAbility_TargetLock::OnTargetMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	OnChangedTargetLocation();
}

void UAGPGameplayAbility_TargetLock::OnTargetActorDestroyed(AActor* DestroyedActor)
{
	/* 현재 타게팅중인 대상이 Destroy된 경우 Ability 취소 */
	TargetLockAbilityCancel();
}

void UAGPGameplayAbility_TargetLock::OnTargetDeathStateChanged(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState)
{
	if (UpdateState == EAGPCharacterDeath::None)
	{
		return;
	}

	/* 현재 타게팅중인 대상이 사망한 경우 Ability 취소 */
	TargetLockAbilityCancel();
}
