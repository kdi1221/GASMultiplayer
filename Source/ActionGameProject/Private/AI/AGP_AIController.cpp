// KJY All Rights Reserved


#include "AI/AGP_AIController.h"

#include "Navigation/CrowdFollowingComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "Perception/AIPerceptionComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "Characters/AGP_PlayerCharacter.h"
#include "Characters/AGP_NPCBase.h"
#include "Perception/AISenseConfig.h"
#include "Log/AGPLogChannels.h"

ECrowdSimulationState ConvertFromAGPCrowdEnableFlag(const EAGPCrowdEnableFlag InEnableFlag)
{
	switch (InEnableFlag)
	{
	case EAGPCrowdEnableFlag::Enabled:
		return ECrowdSimulationState::Enabled;

	case EAGPCrowdEnableFlag::ObstacleOnly:
		return ECrowdSimulationState::ObstacleOnly;

	case EAGPCrowdEnableFlag::Disabled:
		return ECrowdSimulationState::Disabled;
	}

	return ECrowdSimulationState::Enabled;
}

ECrowdAvoidanceQuality::Type ConvertFromAGPCrowdAvoidQuality(const EAGPCrowdAvoidQuality InAvoidQuality)
{
	switch (InAvoidQuality)
	{
	case EAGPCrowdAvoidQuality::Low:
		return ECrowdAvoidanceQuality::Type::Low;

	case EAGPCrowdAvoidQuality::Medium:
		return ECrowdAvoidanceQuality::Type::Medium;

	case EAGPCrowdAvoidQuality::Good:
		return ECrowdAvoidanceQuality::Type::Good;

	case EAGPCrowdAvoidQuality::High:
		return ECrowdAvoidanceQuality::Type::High;
	}

	return ECrowdAvoidanceQuality::Type::Low;
}


AAGP_AIController::AAGP_AIController(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>("PathFollowingComponent"))
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerceptionComponent");
	AIBehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("AIBehaviorTreeComponent");
	AIBlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>("AIBlackboardComponent");

	SetGenericTeamId(static_cast<uint8>(InitializeTeamGroup));

	/* OnPossess 호출시점에 BehaviorTree가 시작되도록 설정 */
	bStartAILogicOnPossess = true;
}

void AAGP_AIController::BeginPlay()
{
	/* 감지 이벤트 콜백 설정 */
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AAGP_AIController::OnPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddUniqueDynamic(this, &AAGP_AIController::OnPerceptionForgotten);

	Super::BeginPlay();

	/* Crowd(군중 이동 제어) 기능 설정 */
	if (UCrowdFollowingComponent* CrowdComp = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
	{
		CrowdComp->SetCrowdSimulationState(ConvertFromAGPCrowdEnableFlag(CrwodEnableFlag));

		CrowdComp->SetCrowdAvoidanceQuality(ConvertFromAGPCrowdAvoidQuality(CrowdAvoidQuality), false);
		CrowdComp->SetAvoidanceGroup(AvoidGroup, false);
		CrowdComp->SetAvoidanceGroup(GroupToAvoid, false);
		CrowdComp->SetCrowdCollisionQueryRange(CrowdCollisionQueryRange, false);
		CrowdComp->UpdateCrowdAgentParams();
	}
}

void AAGP_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OnControlChracterEnable();
}

void AAGP_AIController::OnUnPossess()
{
	OnControlChracterDisable();

	Super::OnUnPossess();
}

void AAGP_AIController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	const FGenericTeamId BeforeTeamID = GetGenericTeamId();

	Super::SetGenericTeamId(NewTeamID);

	/* TeamID가 변경되면 AI 감지 시스템에도 이를 알려줘야 한다. */
	if (BeforeTeamID != GetGenericTeamId() && AIPerceptionComponent)
	{
		AIPerceptionComponent->RequestStimuliListenerUpdate();

		/*UAIPerceptionSystem* AIPerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld());
		if (AIPerceptionSystem)
		{
			AIPerceptionSystem->UpdateListener(*AIPerceptionComponent);
		}*/
	}
}

FGenericTeamId AAGP_AIController::GetGenericTeamId() const
{
	return FGenericTeamId(static_cast<uint8>(InitializeTeamGroup));
}

void AAGP_AIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	RegisterPerceptionPlayer(Cast<AAGP_PlayerCharacter>(Actor));
}

void AAGP_AIController::OnPerceptionForgotten(AActor* Actor)
{
	UnregisterPerceptionPlayer(Cast<AAGP_PlayerCharacter>(Actor));
}

void AAGP_AIController::OnPerceptionCharacterDestroyed(AActor* DestroyedActor)
{
	UnregisterPerceptionPlayer(Cast<AAGP_PlayerCharacter>(DestroyedActor));
}

void AAGP_AIController::OnPerceptionCharacterDeath(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState)
{
	if (UpdateState != EAGPCharacterDeath::Destroy)
	{
		return;
	}

	UnregisterPerceptionPlayer(Cast<AAGP_PlayerCharacter>(UpdatedCharacter));
}

void AAGP_AIController::OnControlPawnDeath(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState)
{
	if (UpdateState != EAGPCharacterDeath::Destroy)
	{
		return;
	}

	OnControlChracterDisable();
}

void AAGP_AIController::OnChangeCurrentHealth(const FOnAttributeChangeData& Data)
{
	CurrentHealthValue = Data.NewValue;

	//AGP_NET_LOG(this, LogAGPAI, Log, TEXT("CurrentHealth : [%f]"), CurrentHealthValue);

	OnAIPawnHealthUpdated.Broadcast(this, CurrentHealthValue, MaxHealthValue);
}

void AAGP_AIController::OnChangeMaxHealth(const FOnAttributeChangeData& Data)
{
	MaxHealthValue = Data.NewValue;

	//AGP_NET_LOG(this, LogAGPAI, Log, TEXT("MaxHealthValue : [%f]"), MaxHealthValue);

	OnAIPawnHealthUpdated.Broadcast(this, CurrentHealthValue, MaxHealthValue);
}

void AAGP_AIController::RegisterPerceptionPlayer(AAGP_PlayerCharacter* PlayerCharacter)
{
	if (!bNotifyStateToPlayer)
	{
		return;
	}

	if (!PlayerCharacter)
	{
		return;
	}

	/* Death이후 Destroy상태인 플레이어캐릭터는 처리 안함 */
	if (EAGPCharacterDeath::Destroy == PlayerCharacter->GetDeathState())
	{
		return;
	}

	/* 이미 바인딩한 상태면 추가하지 않음 */
	if (PerceptionPlayerCharacter.Contains(PlayerCharacter))
	{
		return;
	}

	PlayerCharacter->OnDestroyed.AddDynamic(this, &AAGP_AIController::OnPerceptionCharacterDestroyed);
	PlayerCharacter->OnUpdateDeathState.AddDynamic(this, &AAGP_AIController::OnPerceptionCharacterDeath);
	PlayerCharacter->OnPerceptionByNPC(true, this);
	PerceptionPlayerCharacter.Add(PlayerCharacter);
}

void AAGP_AIController::UnregisterPerceptionPlayer(AAGP_PlayerCharacter* PlayerCharacter)
{
	if (!bNotifyStateToPlayer)
	{
		return;
	}

	if (!PlayerCharacter)
	{
		return;
	}

	/* 바인딩되어 있지 않으면 진행하지 않음 */
	if (!PerceptionPlayerCharacter.Contains(PlayerCharacter))
	{
		return;
	}

	PlayerCharacter->OnDestroyed.RemoveAll(this);
	PlayerCharacter->OnUpdateDeathState.RemoveAll(this);
	PlayerCharacter->OnPerceptionByNPC(false, this);
	PerceptionPlayerCharacter.Remove(PlayerCharacter);
}

void AAGP_AIController::SetAllSenseEnable(bool bEnable)
{
	checkf(AIPerceptionComponent, TEXT("[%s], Invalid AIPerceptionComponent"), __FUNCTIONW__);

	for (UAIPerceptionComponent::TAISenseConfigConstIterator SenseConfigIterator = AIPerceptionComponent->GetSensesConfigIterator();
		SenseConfigIterator;
		++SenseConfigIterator)
	{
		UAISenseConfig* SenseConfig = *SenseConfigIterator;
		if (!SenseConfig)
		{
			continue;
		}

		AIPerceptionComponent->SetSenseEnabled(SenseConfig->GetSenseImplementation(), bEnable);
	}
}

void AAGP_AIController::ResetAllSenseInfo()
{
	checkf(AIPerceptionComponent, TEXT("[%s], Invalid AIPerceptionComponent"), __FUNCTIONW__);

	AIPerceptionComponent->ForgetAll();

	/* 감지했던 플레이어 캐릭터들에게 감지 취소를 알린다. */
	TArray<TWeakObjectPtr<AAGP_PlayerCharacter>> RemovePerceptionCharacters = PerceptionPlayerCharacter.Array();
	for (TWeakObjectPtr<AAGP_PlayerCharacter> PerceptionCharacter : RemovePerceptionCharacters)
	{
		UnregisterPerceptionPlayer(PerceptionCharacter.Get());
	}

	PerceptionPlayerCharacter.Reset();
}

void AAGP_AIController::OnControlChracterEnable()
{
	AAGP_CharacterBase* ControlCharacter = Cast<AAGP_CharacterBase>(GetPawn());
	if (ControlCharacter)
	{
		ControlCharacter->OnUpdateDeathState.AddDynamic(this, &AAGP_AIController::OnControlPawnDeath);

		/* 감지 활성화(Possess시점에서 이미 해당 캐릭터가 Destroy상태가 아니라면) */
		SetAllSenseEnable(EAGPCharacterDeath::Destroy != ControlCharacter->GetDeathState());
	}

	if (bNotifyStateToPlayer)
	{
		UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlCharacter);
		if (PawnASC)
		{
			/* Pawn의 현재/최대 체력 값 저장 및 델리게이트 바인딩 */
			const UAGPAttributeSetCommon* CommonAttribute = Cast<UAGPAttributeSetCommon>(PawnASC->GetAttributeSet(UAGPAttributeSetCommon::StaticClass()));
			checkf(CommonAttribute, TEXT("[%s] - Invalid CommonAttribute"), __FUNCTIONW__);

			/* 현재 체력, 최대 체력 변동에 대한 델리게이트에 바인딩 */
			PawnASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetCurrentHealthAttribute()).AddUObject(this, &AAGP_AIController::OnChangeCurrentHealth);
			PawnASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetMaxHealthAttribute()).AddUObject(this, &AAGP_AIController::OnChangeMaxHealth);

			/* 현재, 최대 체력치 값 저장 */
			CurrentHealthValue = CommonAttribute->GetCurrentHealth();
			MaxHealthValue = CommonAttribute->GetMaxHealth();

			/* 업데이트 된 현재 상태들 알림(초기화) */
			OnAIPawnHealthUpdated.Broadcast(this, CurrentHealthValue, MaxHealthValue);
		}
	}
}

void AAGP_AIController::OnControlChracterDisable()
{
	AAGP_CharacterBase* ControlCharacter = Cast<AAGP_CharacterBase>(GetPawn());
	if (ControlCharacter)
	{
		ControlCharacter->OnUpdateDeathState.RemoveAll(this);

		/* 모든 감지정보 초기화 */
		ResetAllSenseInfo();

		/* 모든 감지 비활성화 */
		SetAllSenseEnable(false);
	}

	if (bNotifyStateToPlayer)
	{
		UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlCharacter);
		if (PawnASC)
		{
			const UAGPAttributeSetCommon* CommonAttribute = Cast<UAGPAttributeSetCommon>(PawnASC->GetAttributeSet(UAGPAttributeSetCommon::StaticClass()));
			if (CommonAttribute)
			{
				/* 현재 체력, 최대 체력 변동에 대한 델리게이트 언바인딩 */
				PawnASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetCurrentHealthAttribute()).RemoveAll(this);
				PawnASC->GetGameplayAttributeValueChangeDelegate(CommonAttribute->GetMaxHealthAttribute()).RemoveAll(this);

				CurrentHealthValue = 0.f;
				MaxHealthValue = 0.f;
			}
		}
	}
}

bool AAGP_AIController::IsPerceptionActor(AActor* InCheckActor) const
{
	checkf(AIPerceptionComponent, TEXT("[%s], Invalid AIPerceptionComponent"), __FUNCTIONW__);

	FActorPerceptionBlueprintInfo ActorPerceptionInfo;
	return AIPerceptionComponent->GetActorsPerception(InCheckActor, ActorPerceptionInfo);
}

int32 AAGP_AIController::GetMonsterNameID() const
{
	AAGP_NPCBase* ControlNPC = GetPawn<AAGP_NPCBase>();
	return ControlNPC ? ControlNPC->GetMonsterNameID() : 0;
}

void AAGP_AIController::ForEachHostileActors(const TFunctionRef<void(AActor*)>& FunctionPerActor) const
{
	checkf(AIPerceptionComponent, TEXT("[%s], Invalid AIPerceptionComponent"), __FUNCTIONW__);

	TArray<AActor*> PerceivedActors;
	AIPerceptionComponent->GetHostileActors(PerceivedActors);
	for (AActor* PerceivedActor : PerceivedActors)
	{
		if (!PerceivedActor)
		{
			AGP_LOG(LogAGPAI, Warning, TEXT("Invalid PerceivedActor, AIController[%s]"), *GetNameSafe(this));
			continue;
		}

		FunctionPerActor(PerceivedActor);
	}
}