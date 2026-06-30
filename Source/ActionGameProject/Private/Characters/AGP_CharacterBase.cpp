// KJY All Rights Reserved


#include "Characters/AGP_CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "Costume/AGPCostumeActorBase.h"
#include "Animation/AnimInstances/AGPCharacterAnimInstance.h"
#include "Item/Components/AGPItemSystemsFacadeComponent.h"
#include "Characters/Components/AGPCharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Characters/Components/AGPCharPresentationComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Collision/AGPCollisionHandleComponent.h"
#include "Item/Types/AGPItemSystemDelegate.h"
#include "Item/Slots/AGPItemSlot.h"
#include "Item/Instance/AGPItemInstance.h"
#include "Widget/AGPWidgetComponent.h"
#include "Characters/Components/AGPStateTagNotifierComponent.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

const FName AAGP_CharacterBase::DeathPawnCollisionProfileName = FName(TEXT("DeathPawnCollisionProfile"));

AAGP_CharacterBase::AAGP_CharacterBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UAGPCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	CharacterPresentationComponent = CreateDefaultSubobject<UAGPCharPresentationComponent>(TEXT("CharacterPresentationComponent"));
	CollisionHandleComponent = CreateDefaultSubobject<UAGPCollisionHandleComponent>(TEXT("CollisionHandleComponent"));
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	StateTagNotifierComponent = CreateDefaultSubobject<UAGPStateTagNotifierComponent>(TEXT("StateTagNotifierComponent"));

	CharacterMovementComponent = GetCharacterMovement<UAGPCharacterMovementComponent>();

	USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	checkf(SkeletalMeshComponent, TEXT("CharacterBase Constructor MeshComponent Invalid.."));
	SkeletalMeshComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SkeletalMeshComponent->bReceivesDecals = false;
	
	UCapsuleComponent* CharacterCapsuleComponent = GetCapsuleComponent();
	checkf(CharacterCapsuleComponent, TEXT("CharacterBase Constructor CapsuleComponent Invalid.."));
	CharacterCapsuleComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	TargetLockWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetLockWidgetComponent"));
	TargetLockWidgetComponent->SetDrawAtDesiredSize(true);
	TargetLockWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	TargetLockWidgetComponent->SetVisibility(false);
	TargetLockWidgetComponent->SetupAttachment(SkeletalMeshComponent);

	FloatingWidgetComponent = CreateDefaultSubobject<UAGPWidgetComponent>(TEXT("FloatingWidgetComponent"));
	FloatingWidgetComponent->SetDrawAtDesiredSize(true);
	FloatingWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	FloatingWidgetComponent->SetVisibility(false);
	FloatingWidgetComponent->SetupAttachment(SkeletalMeshComponent);
	
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AAGP_CharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();	

	checkf(CharacterPresentationComponent, TEXT("[%s] - [%s] Character Invalid CharacterPresentationComponent"), __FUNCTIONW__, *GetNameSafe(this));
	CharacterPresentationComponent->SetCostumeActorInstigator(this);

	USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	checkf(SkeletalMeshComponent, TEXT("CharacterBase Constructor MeshComponent Invalid.."));
	CharacterPresentationComponent->SetOwnerSkeletalMeshComponent(SkeletalMeshComponent);
}

void AAGP_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	/* BeginPlay호출시점에서 Intro상태인경우 SkeletalMesh Bone Refresh */
	if (HasTagInASC(AGPGameplayTags::State_SpawnIntro))
	{
		RefreshSkeletalMeshBone(true);
	}
}

void AAGP_CharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAGP_CharacterBase, CharacterDeathState, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AAGP_CharacterBase, IsMovementTargetLock, COND_SkipOwner);
}

void AAGP_CharacterBase::SetCharacterDeath_Dying(const FName& InDeathPoseName)
{
	if (ROLE_SimulatedProxy == GetLocalRole())
	{
		return;
	}

	CharacterDeathState.DeathState = EAGPCharacterDeath::Dying;
	CharacterDeathState.DeathPoseName = InDeathPoseName;

	ForceNetUpdate();

	OnChangeDeathState_Dying();
}

void AAGP_CharacterBase::SetCharacterDeath_Death()
{
	if (ROLE_SimulatedProxy == GetLocalRole())
	{
		return;
	}

	CharacterDeathState.DeathState = EAGPCharacterDeath::Death;

	ForceNetUpdate();

	OnChangeDeathState_Death();
}

void AAGP_CharacterBase::SetCharacterDeath_Destroy()
{
	if (ROLE_SimulatedProxy == GetLocalRole())
	{
		return;
	}

	CharacterDeathState.DeathState = EAGPCharacterDeath::Destroy;

	ForceNetUpdate();

	OnChangeDeathState_Destroy(true);
}

void AAGP_CharacterBase::SetCharacterDeath_Rebirth()
{
	if (ROLE_SimulatedProxy == GetLocalRole())
	{
		return;
	}

	CharacterDeathState.DeathState = EAGPCharacterDeath::Rebirth;

	ForceNetUpdate();

	OnChangeDeathState_Rebirth();
}

void AAGP_CharacterBase::SetCharacterDeath_None()
{
	if (ROLE_SimulatedProxy == GetLocalRole())
	{
		return;
	}

	CharacterDeathState.DeathState = EAGPCharacterDeath::None;
	CharacterDeathState.DeathPoseName = NAME_None;

	ForceNetUpdate();

	OnChangeDeathState_None();
}

void AAGP_CharacterBase::OnAnimNotify_DeathEventCall(const FGameplayTag& InEventTag)
{
	if (HasAuthority())
	{
		/* Server, Local Client는 GameplayEvent 호출 */
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, InEventTag, FGameplayEventData());
	}
	else if (IsLocallyControlled())
	{
		/* Server, Local Client는 GameplayEvent 호출 */
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, InEventTag, FGameplayEventData());
	}
	else
	{
		/* Simulate는 Death(Dying Finish), None(Rebirth Finish)일때 강제로 DeathState 설정(서버로부터 DeathState 업데이트까지 딜레이 고려) */
		if (InEventTag.MatchesTagExact(AGPGameplayTags::Event_Character_Reaction_Death_Death))
		{
			if (EAGPCharacterDeath::Death != GetDeathState())
			{
				SetCharacterDeath_Death();
			}
		}
		else if (InEventTag.MatchesTagExact(AGPGameplayTags::Event_Character_Reaction_Death_None))
		{
			if (EAGPCharacterDeath::None != GetDeathState())
			{
				SetCharacterDeath_None();
			}
		}
	}
}

void AAGP_CharacterBase::SetMotionWarpTargetLocation(const FName& InMotionWarpTargetName, const FVector& InTargetLocation)
{
	checkf(MotionWarpingComponent, TEXT("[%s], MotionWarpingComponent Invalid"), __FUNCTIONW__);

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(InMotionWarpTargetName, InTargetLocation);
}

void AAGP_CharacterBase::SetMotionWarpTargetLocationAndRotation(const FName& InMotionWarpTargetName, const FVector& InTargetLocation, const FRotator& InTargetRotation)
{
	checkf(MotionWarpingComponent, TEXT("[%s], MotionWarpingComponent Invalid"), __FUNCTIONW__);

	MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(InMotionWarpTargetName, InTargetLocation, InTargetRotation);
}

void AAGP_CharacterBase::RemoveWarpTarget(const FName& InMotionWarpTargetName)
{
	checkf(MotionWarpingComponent, TEXT("[%s], MotionWarpingComponent Invalid"), __FUNCTIONW__);

	MotionWarpingComponent->RemoveWarpTarget(InMotionWarpTargetName);
}

UAbilitySystemComponent* AAGP_CharacterBase::GetAbilitySystemComponent() const
{
	return GetAGPAbilitySystemComponent();
}

UAGPCollisionHandleComponent* AAGP_CharacterBase::GetCollisionHandleComponent_Implementation() const
{
	return CollisionHandleComponent;
}

const FGameplayTag& AAGP_CharacterBase::GetGCHitReactTag() const
{
	return GCHitReactTag;
}

void AAGP_CharacterBase::OnHitEvent_Implementation()
{
	/* CostumeActor들에게 HitEvent 알림 */
	if (CharacterPresentationComponent)
	{
		CharacterPresentationComponent->OnOwnerHitEvent();
	}
}

void AAGP_CharacterBase::InitializeAbilitySystem(AActor* InOwnerActor)
{
	UAGP_AbilitySystemComponent* AbilitySystemComponent = GetAGPAbilitySystemComponent();
	checkf(AbilitySystemComponent, TEXT("AbilitySystemComponent Invalid"));

	UAGPAttributeSetCommon* AttributeSet = GetCommonAttributeSet();
	checkf(AttributeSet, TEXT("AttributeSet Invalid"));

	/* 캐릭터 고유 AttributeSet 추가(HasAuthority) */
	if (HasAuthority())
	{
		for (TSubclassOf<UAGPAttributeSetBase> DynamicAttributeSetClass : DynamicAttributeSetClasses)
		{
			AbilitySystemComponent->AddDynamicAttributeSet(DynamicAttributeSetClass);
		}
	}

	/* ASC Ability Actor 초기화 */
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, this);

	/* 소유 컴포넌트들에게 ASC 알림 */
	checkf(CharacterPresentationComponent, TEXT("[%s], CharacterPresentationComponent Invalid, Character[%s]"), __FUNCTIONW__, *GetNameSafe(this));
	CharacterPresentationComponent->InitializeAbilitySystem(AbilitySystemComponent);

	checkf(CollisionHandleComponent, TEXT("[%s], CollisionHandleComponent Invalid, Character[%s]"), __FUNCTIONW__, *GetNameSafe(this));
	CollisionHandleComponent->InitializeOnPossedToOwner(AbilitySystemComponent);

	checkf(StateTagNotifierComponent, TEXT("[%s], StateTagNotifierComponent Invalid, Character[%s]"), __FUNCTIONW__, *GetNameSafe(this));
	StateTagNotifierComponent->InitializeAbilitySystem(AbilitySystemComponent);
}

void AAGP_CharacterBase::UninitializeAbilitySystem()
{
	/* AbilitySystem Avatar Reset */
	UAGP_AbilitySystemComponent* AbilitySystemComponent = GetAGPAbilitySystemComponent();
	if (!AbilitySystemComponent || (this != AbilitySystemComponent->GetAvatarActor()))
	{
		return;
	}

	/* 캐릭터 고유 AttributeSet 해제 */
	if (HasAuthority())
	{
		for (TSubclassOf<UAGPAttributeSetBase> DynamicAttributeSetClass : DynamicAttributeSetClasses)
		{
			AbilitySystemComponent->RemoveDynamicAttributeSet(DynamicAttributeSetClass);
		}
	}

	/* 소유 컴포넌트들의 ASC 해제 */
	checkf(CharacterPresentationComponent, TEXT("[%s], CharacterPresentationComponent Invalid, Character[%s]"), __FUNCTIONW__, *GetNameSafe(this));
	CharacterPresentationComponent->UninitializeAbilitySystem(AbilitySystemComponent);

	checkf(CollisionHandleComponent, TEXT("[%s], CollisionHandleComponent Invalid, Character[%s]"), __FUNCTIONW__, *GetNameSafe(this));
	CollisionHandleComponent->OnUnpossedFromOwner();

	checkf(StateTagNotifierComponent, TEXT("[%s], StateTagNotifierComponent Invalid, Character[%s]"), __FUNCTIONW__, *GetNameSafe(this));
	StateTagNotifierComponent->UninitializeAbilitySystem(AbilitySystemComponent);

	AbilitySystemComponent->SetAvatarActor(nullptr);
}

/* 캐릭터의 초기 Attribute 초기화 */
void AAGP_CharacterBase::InitializeAttribute()
{
	checkf(HasAuthority(), TEXT("[%s], Must Call Authority"), __FUNCTIONW__);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	checkf(ASC, TEXT("[%s], Invalid AbilitySystemComponent"), __FUNCTIONW__);

	/* Base Attribute Initialize */
	for (TSubclassOf<UGameplayEffect> InitAttribute_BaseGE : InitAttribute_BaseGEs)
	{
		checkf(InitAttribute_BaseGE, TEXT("[%s], InitAttribute_Base Invalid"), __FUNCTIONW__);

		FGameplayEffectContextHandle AttributeBaseEffectContextHandle = ASC->MakeEffectContext();
		AttributeBaseEffectContextHandle.AddSourceObject(ASC->GetAvatarActor());
		const FGameplayEffectSpecHandle AttributeBaseEffectSpecHandle = ASC->MakeOutgoingSpec(InitAttribute_BaseGE, 1.f, AttributeBaseEffectContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*AttributeBaseEffectSpecHandle.Data.Get());
	}

	/* AffectedByBase Attribute Initialize */
	for (TSubclassOf<UGameplayEffect> InitAttribute_AffectedByBase : InitAttribute_AffectedByBaseGEs)
	{
		checkf(InitAttribute_AffectedByBase, TEXT("[%s], InitAttribute_AffectedByBase Invalid"), __FUNCTIONW__);

		FGameplayEffectContextHandle AttributeAffectedByBaseEffectContextHandle = ASC->MakeEffectContext();
		AttributeAffectedByBaseEffectContextHandle.AddSourceObject(ASC->GetAvatarActor());
		const FGameplayEffectSpecHandle AttributeAffectedByBaseEffectSpecHandle = ASC->MakeOutgoingSpec(InitAttribute_AffectedByBase, 1.f, AttributeAffectedByBaseEffectContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*AttributeAffectedByBaseEffectSpecHandle.Data.Get());
	}
}

/* 캐릭터의 초기상태 관련 Tag 초기화 */
void AAGP_CharacterBase::InitializeState()
{
	checkf(HasAuthority(), TEXT("[%s], Must Call Authority"), __FUNCTIONW__);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	checkf(ASC, TEXT("[%s], Invalid AbilitySystemComponent"), __FUNCTIONW__);

	/* State 관련 초기에 적용할 GameplayEffects */
	for (TSubclassOf<UGameplayEffect> InitState_GE : InitStateGEs)
	{
		checkf(InitState_GE, TEXT("[%s], InitAttribute_Base Invalid"), __FUNCTIONW__);

		FGameplayEffectContextHandle ApplyStateEffectContextHandle = ASC->MakeEffectContext();
		ApplyStateEffectContextHandle.AddSourceObject(ASC->GetAvatarActor());
		const FGameplayEffectSpecHandle ApplyStateEffectSpecHandle = ASC->MakeOutgoingSpec(InitState_GE, 1.f, ApplyStateEffectContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*ApplyStateEffectSpecHandle.Data.Get());
	}
}

/* 캐릭터의 초기 활성화될 Ability들 */
void AAGP_CharacterBase::InitializeActivateAbility()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	checkf(ASC, TEXT("[%s], Invalid AbilitySystemComponent"), __FUNCTIONW__);

	ASC->TryActivateAbilitiesByTag(PersistentAbilityTags);
}

void AAGP_CharacterBase::InitailizeCharacterWidget()
{
	//AGP_NET_LOG(this, LogAGPWidget, Log, TEXT("Character[%s]"), *GetNameSafe(this));

	checkf(FloatingWidgetComponent, TEXT("[%s] - Invalid FloatingWidgetComponent"), __FUNCTIONW__);

	FloatingWidgetComponent->InitFloatingWidgetComponent(this);
	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_CharacterBase::UninitializeCharacterWidget()
{
	//AGP_NET_LOG(this, LogAGPWidget, Log, TEXT("Character[%s]"), *GetNameSafe(this));

	checkf(FloatingWidgetComponent, TEXT("[%s] - Invalid FloatingWidgetComponent"), __FUNCTIONW__);

	FloatingWidgetComponent->UninitializeFloatingWidgetComponent(this);
	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_CharacterBase::SetCharacterCapsuleCollisionProfile(const FName& InCollisionProfile)
{
	UCapsuleComponent* CharacterCapsuleComponent = GetCapsuleComponent();
	if (!CharacterCapsuleComponent)
	{
		return;
	}

	if (InCollisionProfile != CharacterCapsuleComponent->GetCollisionProfileName())
	{
		CharacterCapsuleComponent->SetCollisionProfileName(InCollisionProfile);
	}
}

void AAGP_CharacterBase::SetCharacterMoveLock(const bool IsLock)
{
	if (IsLock)
	{
		if (AController* OwnerController = GetController())
		{
			/* 캐릭터 이동 입력 막기 */
			OwnerController->SetIgnoreMoveInput(true);
		}

		if (CharacterMovementComponent)
		{
			/* 이동 즉시 멈춤 */
			CharacterMovementComponent->StopMovementImmediately();
		}
	}
	else
	{
		if (AController* OwnerController = GetController())
		{
			/* 캐릭터 이동 입력 다시 활성화 */
			OwnerController->SetIgnoreMoveInput(false);
		}
	}
}

void AAGP_CharacterBase::SetFloatingWidgetVisiblityIfAlive()
{
	checkf(FloatingWidgetComponent, TEXT("[%s] - Invalid FloatingWidgetComponent"), __FUNCTIONW__);

	/* Floating Widget Visible 결정(캐릭터 Alive 상태, 로컬 캐릭터 등..) */
	FloatingWidgetComponent->SetVisibility(IsVisibleHeadUPFloatingWidget());
}

void AAGP_CharacterBase::BroadcastChangeDeathState()
{
	OnUpdateDeathState.Broadcast(this, GetDeathState());
}

void AAGP_CharacterBase::OnRep_CharacterDeathState()
{
	switch (CharacterDeathState.DeathState)
	{
	case EAGPCharacterDeath::Dying:
		OnChangeDeathState_Dying();
		break;

	case EAGPCharacterDeath::Death:
		OnChangeDeathState_Death();
		break;

		/* Simulate에서 리플리케이션에 의해 생성되었을때 이미 Destroy 상태면 FX 표시 X */
	case EAGPCharacterDeath::Destroy:
		OnChangeDeathState_Destroy(HasActorBegunPlay());
		break;

	case EAGPCharacterDeath::Rebirth:
		OnChangeDeathState_Rebirth();
		break;

	case EAGPCharacterDeath::None:
		OnChangeDeathState_None();
		break;
	}
}

/* 캐릭터 사망 상태 진입 */
void AAGP_CharacterBase::OnChangeDeathState_Dying_Implementation()
{
	/* 이동 잠금 처리 */
	SetCharacterMoveLock(true);

	/* Costume등을 통해 Attach된 Collision(EX : Weapoon)들 모두 비활성화 */
	if (CollisionHandleComponent)
	{
		CollisionHandleComponent->AllCollisionDisable();
	}

	/* Character Collision 처리 */
	SetCharacterCapsuleCollisionProfile(DeathPawnCollisionProfileName);

	/* 변경된 Death 상태 알림 */
	BroadcastChangeDeathState();

	/* Floating Widget Visiblity 결정 */
	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_CharacterBase::OnChangeDeathState_Death_Implementation()
{
	/* Character Collision 처리 */
	SetCharacterCapsuleCollisionProfile(DeathPawnCollisionProfileName);

	/* 변경된 Death 상태 알림 */
	BroadcastChangeDeathState();

	/* Floating Widget Visiblity 결정 */
	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_CharacterBase::OnChangeDeathState_Destroy_Implementation(bool IsShowDissolveFX)
{
	/* Character Collision 처리 */
	SetCharacterCapsuleCollisionProfile(DeathPawnCollisionProfileName);

	/* 변경된 Death 상태 알림 */
	BroadcastChangeDeathState();

	/* CostumeActor에 DeathDestroy 알림 */
	if (CharacterPresentationComponent)
	{
		CharacterPresentationComponent->OnOwnerDeathDestroy(IsShowDissolveFX);
	}

	/* Floating Widget Visiblity 결정 */
	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_CharacterBase::OnChangeDeathState_Rebirth_Implementation()
{
	/* Character Collision 처리(다시 활성화) */
	SetCharacterCapsuleCollisionProfile(UCollisionProfile::Pawn_ProfileName);

	/* 변경된 Death 상태 알림 */
	BroadcastChangeDeathState();

	/* Floating Widget Visiblity 결정 */
	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_CharacterBase::OnChangeDeathState_None_Implementation()
{
	/* 이동 활성화 처리 */
	SetCharacterMoveLock(false);

	/* Character Collision 처리(다시 활성화) */
	SetCharacterCapsuleCollisionProfile(UCollisionProfile::Pawn_ProfileName);

	/* 변경된 Death 상태 알림 */
	BroadcastChangeDeathState();

	/* Floating Widget Visiblity 결정 */
	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_CharacterBase::StartTargetLockMove()
{
	/* 사망한 상태에서는 TargetLock설정하지 않음 */
	if (IsDeath())
	{
		return;
	}

	IsMovementTargetLock = true;
}

void AAGP_CharacterBase::UpdateTargetLockPostion(const FVector& InTargetLocation)
{
	if (!IsMovementTargetLock)
	{
		return;
	}

	TargetLockLocation = InTargetLocation;
}

void AAGP_CharacterBase::StopTargetLockMove()
{
	TargetLockLocation = FVector::ZeroVector;
	IsMovementTargetLock = false;
}

void AAGP_CharacterBase::OnNotifyTargetLocked()
{
	if (TargetLockWidgetComponent)
	{
		TargetLockWidgetComponent->SetVisibility(true);
	}
}

void AAGP_CharacterBase::OnNotifyTargetLockRelease()
{
	if (TargetLockWidgetComponent)
	{
		TargetLockWidgetComponent->SetVisibility(false);
	}
}

void AAGP_CharacterBase::RefreshSkeletalMeshBone(bool bNeedsValidRootMotion)
{
	if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
	{
		SkeletalMeshComponent->TickAnimation(0.f, bNeedsValidRootMotion);
		SkeletalMeshComponent->RefreshBoneTransforms();
	}
}

bool AAGP_CharacterBase::IsInsideFrustum(const FConvexVolume& InViewFrustum) const
{
	UCapsuleComponent* CharacterCapsuleComponent = GetCapsuleComponent();
	if (!CharacterCapsuleComponent)
	{
		return false;
	}

	/* 캡슐 위치와 크기등을 가지고 AABB Box 계산 */
	const FVector CapsuleLocation = CharacterCapsuleComponent->GetComponentLocation();
	const float CapsuleHalfHeight = CharacterCapsuleComponent->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = CharacterCapsuleComponent->GetScaledCapsuleRadius();

	/* 프러스텀 내 AABB Box 존재 유무 반환 */
	const FVector BoxExtent(CapsuleRadius, CapsuleRadius, CapsuleHalfHeight);
	return InViewFrustum.IntersectBox(CapsuleLocation, BoxExtent);
}

bool AAGP_CharacterBase::HasTagInASC(const FGameplayTag& InCheckTag) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(InCheckTag);
}


UAnimMontage* AAGP_CharacterBase::GetDeathMontageFromDeathPose() const
{
	if (!CharacterPresentationComponent)
	{
		return nullptr;
	}

	if (EAGPCharacterDeath::None == CharacterDeathState.DeathState || CharacterDeathState.DeathPoseName == NAME_None)
	{
		return nullptr;
	}

	return CharacterPresentationComponent->GetDeathMontageFromDeathPose(CharacterDeathState.DeathPoseName);
}

UAnimMontage* AAGP_CharacterBase::GetRebirthMontageFromDeathPose() const
{
	if (!CharacterPresentationComponent)
	{
		return nullptr;
	}

	if (EAGPCharacterDeath::None == CharacterDeathState.DeathState || CharacterDeathState.DeathPoseName == NAME_None)
	{
		return nullptr;
	}

	return CharacterPresentationComponent->GetRebirthMontageFromDeathPose(CharacterDeathState.DeathPoseName);
}

bool AAGP_CharacterBase::IsDeath() const
{
	/* Character의 Death State 확인 */
	return EAGPCharacterDeath::None != GetDeathState();
}

bool AAGP_CharacterBase::IsVisibleHeadUPFloatingWidget() const
{
	//1. 플레이어-로컬은 표시하지않음
	const bool IsPlayerLocal = IsPlayerControl() && IsLocallyControlled();

	//2. Alive 여부
	const bool IsAlive = !IsDeath();

	//3. 위젯컨트롤러 설정 여부
	const bool IsHeadUPWidgetControllerSet = FloatingWidgetComponent && FloatingWidgetComponent->GetCreatedWidgetController();

	//4. ShowFloatingWidget(보스몬스터와 같이 머리 위 HP Widget 표시 X하는 경우), 모든 조건 만족해야함
	return !IsPlayerLocal && IsAlive && IsHeadUPWidgetControllerSet && ShowFloatingWidget;
}

bool AAGP_CharacterBase::IsPlayerControl() const
{
	return Controller && Controller->IsPlayerController();
}
