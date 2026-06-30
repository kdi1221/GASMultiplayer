// KJY All Rights Reserved


#include "Characters/AGP_PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/Components/AGPCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player/AGP_PlayerStateBase.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "Characters/Components/AGPCharPresentationComponent.h"
#include "Characters/Components/AGPScanFieldObjectComponent.h"
#include "AGPGameplayTags.h"
#include "AI/AGP_AIController.h"
#include "Net/UnrealNetwork.h"
#include "Widget/AGPWidgetComponent.h"
#include "Log/AGPLogChannels.h"

#pragma region[FAGPPerceptionNPCInfo]

bool FAGPPerceptionNPCInfo::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return BossStatusInfo.NetSerialize(Ar, Map, bOutSuccess);
}

bool FAGPPerceptionNPCInfo::Identical(const FAGPPerceptionNPCInfo* Other, uint32 PortFlags) const
{
	if (!Other)
	{
		return false;
	}

	return BossStatusInfo.Identical(&Other->BossStatusInfo, PortFlags);
}

#pragma endregion

#pragma region[FAGPPerceptionNPCInfoContainer]

void FAGPPerceptionNPCInfoContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (AAGP_PlayerCharacter* OwnerCharacter = CachedOwnerCharacter.Get())
	{
		OwnerCharacter->OnPreReplicatedRemove(RemovedIndices, FinalSize);
	}
}

void FAGPPerceptionNPCInfoContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (AAGP_PlayerCharacter* OwnerCharacter = CachedOwnerCharacter.Get())
	{
		OwnerCharacter->OnPostReplicatedAdd(AddedIndices, FinalSize);
	}
}

void FAGPPerceptionNPCInfoContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (AAGP_PlayerCharacter* OwnerCharacter = CachedOwnerCharacter.Get())
	{
		OwnerCharacter->OnPostReplicatedChange(ChangedIndices, FinalSize);
	}
}

bool FAGPPerceptionNPCInfoContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FAGPPerceptionNPCInfo, FAGPPerceptionNPCInfoContainer>(PerceptionNPCInfos, DeltaParms, *this);
}

void FAGPPerceptionNPCInfoContainer::InitializeContainer(AAGP_PlayerCharacter* InOwnerCharacter)
{
	CachedOwnerCharacter = InOwnerCharacter;
}

#pragma endregion



AAGP_PlayerCharacter::AAGP_PlayerCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 65.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	ScanFieldObjectComponent = CreateDefaultSubobject<UAGPScanFieldObjectComponent>(TEXT("ScanFieldObjectComponent"));

	verifyf(CharacterMovementComponent, TEXT("Constructor MovementComponent Invalid.."));
	CharacterMovementComponent->bOrientRotationToMovement = true;
	CharacterMovementComponent->RotationRate = FRotator(0.f, 500.f, 0.f);
	CharacterMovementComponent->MaxWalkSpeed = 400.f;
	CharacterMovementComponent->BrakingDecelerationWalking = 2000.f;
}

#pragma region[Parent Class Interface]
void AAGP_PlayerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	//AGP_NET_LOG(LogTemp, Log, TEXT("Net Log Test..[%d]"), GetUniqueID());
	//AGP_LOG(LogTemp, Log, TEXT("Base Log Test..[%d]"), GetUniqueID());
	//AGP_SCREEN_DEBUG_MSG(-1, 5.f, FColor::Green, TEXT("Base Log Test..[%d]"), GetUniqueID());

	if (NewPlayerState)
	{
		/* 특정 Controller에 Possess 되었을때 */
		OnPossesPlayerController();
	}
	else
	{
		/* Unpossess 되었을때 */
		OnUnpossessPlayerController();
	}
}

void AAGP_PlayerCharacter::DetachFromControllerPendingDestroy()
{
	/* Pawn 자체가 Unpossed와는 별도로 Destroy되는 상황에 호출됨 */
	/* (Ex : Role이 Simulate일때 네트워크 허용 범위를 넘어서 Pawn자체가 Destroy되는 상황) */
	OnUnpossessPlayerController();

	Super::DetachFromControllerPendingDestroy();
}

void AAGP_PlayerCharacter::SetPlayerDefaults()
{
	AAGP_PlayerStateBase* CurrentPlayerState = GetPlayerState<AAGP_PlayerStateBase>();
	checkf(CurrentPlayerState, TEXT("[%s], PlayerState Invalid.."), __FUNCTIONW__);

	CurrentPlayerState->InitializePlayerAbilities();

	CurrentPlayerState->InitializePlayerItems();

	/* Attribute 초기화 */
	InitializeAttribute();

	/* State 초기화 */
	InitializeState();

	/* 초기 Ability 활성화 */
	InitializeActivateAbility();

	/* 초기화된 Attribute 갱신(Widget, Replicate) */
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->ForceReplication();
	}

	SetFloatingWidgetVisiblityIfAlive();
}

void AAGP_PlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AAGP_PlayerCharacter, PerceptionNPCInfoContainer, COND_OwnerOnly);
}

void AAGP_PlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && EWorldType::EditorPreview != CurrentWorld->WorldType)
	{
		PerceptionNPCInfoContainer.InitializeContainer(this);
	}
}

#pragma endregion

#pragma region[AAGP_CharacterBase Interface]
void AAGP_PlayerCharacter::InitializeState()
{
	Super::InitializeState();

	UAGP_AbilitySystemComponent* AbilitySystemComponent = GetAGPAbilitySystemComponent();
	checkf(AbilitySystemComponent, TEXT("[%s] - AbilitySystemComponent Invalid"), __FUNCTIONW__);

	/* Weapon Handling 초기화(Holstered) => TODO : GE기반으로 변경 */
	AbilitySystemComponent->RemoveLooseGameplayTag(AGPGameplayTags::State_CombatMode_ON);
	AbilitySystemComponent->SetLooseGameplayTag(AGPGameplayTags::State_CombatMode_OFF);
}

void AAGP_PlayerCharacter::InitailizeCharacterWidget()
{
	Super::InitailizeCharacterWidget();

	checkf(FloatingWidgetComponent, TEXT("[%s] - Invalid FloatingWidgetComponent"), __FUNCTIONW__);

	AAGP_PlayerStateBase* OwnerPlayerState = GetPlayerState<AAGP_PlayerStateBase>();
	checkf(OwnerPlayerState, TEXT("[%s] - Invalid OwnerPlayerState"), __FUNCTIONW__);

	/* Player State의 플레이어 이름을 표시 */
	FloatingWidgetComponent->SetCharacterNameText(FText::FromString(OwnerPlayerState->GetPlayerName()));
}

UAGP_AbilitySystemComponent* AAGP_PlayerCharacter::GetAGPAbilitySystemComponent() const
{
	return CachedAbilitySystemComponent.Get();
}

UAGPAttributeSetCommon* AAGP_PlayerCharacter::GetCommonAttributeSet() const
{
	return CachedCommonAttributeSet.Get();
}
#pragma endregion

#pragma region[IAGPItemSystemExternalInterface]
UAGPItemSystemsFacadeComponent* AAGP_PlayerCharacter::GetItemSystemFacadeComponent() const
{
	AAGP_PlayerStateBase* OwnerPlayerState = GetPlayerState<AAGP_PlayerStateBase>();
	return OwnerPlayerState ? OwnerPlayerState->GetItemSystemFacadeComponent() : nullptr;
}
#pragma endregion

#pragma region[IGenericTeamAgentInterface]
FGenericTeamId AAGP_PlayerCharacter::GetGenericTeamId() const
{
	/* PlayerState의 Team 반환 */
	AAGP_PlayerStateBase* OwnerPlayerState = GetPlayerState<AAGP_PlayerStateBase>();
	return OwnerPlayerState ? OwnerPlayerState->GetGenericTeamId() : FGenericTeamId::NoTeam;
}
#pragma endregion

void AAGP_PlayerCharacter::OnPerceptionByNPC(bool bIsPerception, AAGP_AIController* AIController)
{
	checkf(AIController, TEXT("[%s], Invalid AIController"), __FUNCTIONW__);

	//AGP_NET_LOG(this, LogAGPCharacter, Log, TEXT("IsPerception[%s], AIController[%s], AI Pawn[%s]"), bIsPerception ? TEXT("TRUE") : TEXT("FALSE"), *GetNameSafe(AIController), *GetNameSafe(AIController->GetPawn()));

	if (bIsPerception)
	{
		AIController->OnAIPawnHealthUpdated.AddUObject(this, &AAGP_PlayerCharacter::OnPerceptionNPCChangeHealth);

		FAGPPerceptionNPCInfo& AddedNPCInfo = PerceptionNPCInfoContainer.PerceptionNPCInfos.AddDefaulted_GetRef();
		AddedNPCInfo.BossStatusInfo.AIControllerUniqueID = AIController->GetUniqueID();
		AddedNPCInfo.BossStatusInfo.CurrentHealth = AIController->GetPawnCurrentHealth();
		AddedNPCInfo.BossStatusInfo.MaxHealth = AIController->GetPawnMaxHealth();
		AddedNPCInfo.BossStatusInfo.NameID = AIController->GetMonsterNameID();

		//변경된 요소에 대해 마킹
		PerceptionNPCInfoContainer.MarkItemDirty(AddedNPCInfo);

		OnAddedNPCPerceptionInfo(AddedNPCInfo);

	}
	else
	{
		AIController->OnAIPawnHealthUpdated.RemoveAll(this);

		const int32 PerceptionNPCInfoNum = PerceptionNPCInfoContainer.PerceptionNPCInfos.Num();
		for (int32 ElementIndex = 0; ElementIndex < PerceptionNPCInfoNum; ++ElementIndex)
		{
			FAGPPerceptionNPCInfo& CheckNPCInfo = PerceptionNPCInfoContainer.PerceptionNPCInfos[ElementIndex];
			if (CheckNPCInfo.BossStatusInfo.AIControllerUniqueID == AIController->GetUniqueID())
			{
				OnRemovedNPCPerceptionInfo(CheckNPCInfo);

				PerceptionNPCInfoContainer.PerceptionNPCInfos.RemoveAt(ElementIndex);

				//특정 요소가 지워졌으므로 Array 마킹
				PerceptionNPCInfoContainer.MarkArrayDirty();

				break;
			}
		}
	}
}

void AAGP_PlayerCharacter::OnPerceptionNPCChangeHealth(AAGP_AIController* AIController, float CurrentHealth, float MaxHealth)
{
	checkf(AIController, TEXT("[%s], Invalid AIController"), __FUNCTIONW__);
	//AGP_NET_LOG(this, LogAGPCharacter, Log, TEXT("AIController[%s], Health[%f / %f]"), *GetNameSafe(AIController), CurrentHealth, MaxHealth);

	const int32 PerceptionNPCInfoNum = PerceptionNPCInfoContainer.PerceptionNPCInfos.Num();
	for (int32 ElementIndex = 0; ElementIndex < PerceptionNPCInfoNum; ++ElementIndex)
	{
		FAGPPerceptionNPCInfo& CheckNPCInfo = PerceptionNPCInfoContainer.PerceptionNPCInfos[ElementIndex];
		if (CheckNPCInfo.BossStatusInfo.AIControllerUniqueID == AIController->GetUniqueID())
		{
			CheckNPCInfo.BossStatusInfo.CurrentHealth = CurrentHealth;
			CheckNPCInfo.BossStatusInfo.MaxHealth = MaxHealth;

			OnModifyNPCPerceptionInfo(CheckNPCInfo);

			//변경된 요소에 대해 마킹
			PerceptionNPCInfoContainer.MarkItemDirty(CheckNPCInfo);

			break;
		}
	}
}

void AAGP_PlayerCharacter::ForEachBossStatus_Execute(const FOnNotifyBossStatusInfoSignature::FDelegate& InDelegate) const
{
	if (!InDelegate.IsBound())
	{
		return;
	}

	for (const FAGPPerceptionNPCInfo& PerceptionNPCInfo : PerceptionNPCInfoContainer.PerceptionNPCInfos)
	{
		InDelegate.ExecuteIfBound(PerceptionNPCInfo.BossStatusInfo);
	}
}

void AAGP_PlayerCharacter::OnPossesPlayerController()
{
	AAGP_PlayerStateBase* OwnerPlayerState = GetPlayerState<AAGP_PlayerStateBase>();
	if (!OwnerPlayerState)
	{
		return;
	}

	/* 기존에 있던 Avatar Costume Actor들 삭제 */
	verifyf(CharacterPresentationComponent, TEXT("[%s] - [%s], Invalid AttachCostumeActorManageComponent"), *GetNameSafe(this), __FUNCTIONW__);
	CharacterPresentationComponent->AllDestroyAttachCostumeActors();

	//Costume에 대한 AvatarPawn 설정
	OwnerPlayerState->SetAvatarCharacter(this);

	//AbilitySystem 초기화
	UAGP_AbilitySystemComponent* PlayerStateASC = Cast<UAGP_AbilitySystemComponent>(OwnerPlayerState->GetAbilitySystemComponent());
	verifyf(PlayerStateASC, TEXT("PlayerState ASC Invalid.."));

	//PlayerState 및 AbilitySystem 관련 컴포넌트들 캐싱
	CachedCurrentPlayerState = OwnerPlayerState;
	CachedAbilitySystemComponent = PlayerStateASC;
	CachedCommonAttributeSet = OwnerPlayerState->GetAttributeSetCommon();

	//ASC의 Owner => PlayerState 설정
	InitializeAbilitySystem(OwnerPlayerState);

	/* Character Widget(HeadUP) 초기화 */
	InitailizeCharacterWidget();
}

void AAGP_PlayerCharacter::OnUnpossessPlayerController()
{
	AAGP_PlayerStateBase* OwnerPlayerState = CachedCurrentPlayerState.Get();
	if (!OwnerPlayerState)
	{
		return;
	}

	OwnerPlayerState->ResetAvatarCharacter();

	UninitializeAbilitySystem();
	UninitializeCharacterWidget();

	CachedCurrentPlayerState.Reset();
	CachedAbilitySystemComponent.Reset();
	CachedCommonAttributeSet.Reset();
}

void AAGP_PlayerCharacter::OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FAGPPerceptionNPCInfo& RemoveNPCInfo = PerceptionNPCInfoContainer.PerceptionNPCInfos[Index];
		OnRemovedNPCPerceptionInfo(RemoveNPCInfo);
	}
}

void AAGP_PlayerCharacter::OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FAGPPerceptionNPCInfo& AddedNPCInfo = PerceptionNPCInfoContainer.PerceptionNPCInfos[Index];
		OnAddedNPCPerceptionInfo(AddedNPCInfo);
	}
}

void AAGP_PlayerCharacter::OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FAGPPerceptionNPCInfo& ChangedNPCInfo = PerceptionNPCInfoContainer.PerceptionNPCInfos[Index];
		OnModifyNPCPerceptionInfo(ChangedNPCInfo);
	}
}

void AAGP_PlayerCharacter::OnAddedNPCPerceptionInfo(const FAGPPerceptionNPCInfo& AddedNPCInfo)
{
	//AGP_NET_LOG(this, LogAGPCharacter, Log, TEXT("[%s]"), *AddedNPCInfo.BossStatusInfo.ToString());

	OnAddedBossStatus.Broadcast(AddedNPCInfo.BossStatusInfo);
}

void AAGP_PlayerCharacter::OnModifyNPCPerceptionInfo(const FAGPPerceptionNPCInfo& ChangedNPCInfo)
{
	//AGP_NET_LOG(this, LogAGPCharacter, Log, TEXT("[%s]"), *ChangedNPCInfo.BossStatusInfo.ToString());

	OnModifyBossStatus.Broadcast(ChangedNPCInfo.BossStatusInfo);
}

void AAGP_PlayerCharacter::OnRemovedNPCPerceptionInfo(const FAGPPerceptionNPCInfo& RemovedNPCInfo)
{
	//AGP_NET_LOG(this, LogAGPCharacter, Log, TEXT("[%s]"), *RemovedNPCInfo.BossStatusInfo.ToString());

	OnRemoveBossStatus.Broadcast(RemovedNPCInfo.BossStatusInfo);
}
