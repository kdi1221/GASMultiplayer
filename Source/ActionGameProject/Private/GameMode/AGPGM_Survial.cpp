// KJY All Rights Reserved


#include "GameMode/AGPGM_Survial.h"
#include "GameMode/AGPGameState_Survial.h"
#include "Levels/Actors/EQSPointAreaActor.h"
#include "Levels/Actors/AGPRelayActor.h"
#include "FieldObject/AGPInteractableObjectBase.h"
#include "EngineUtils.h"
#include "Net/OnlineEngineInterface.h"
#include "Common/AGPCommonFunctionLibrary.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/AGP_NPCBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Player/AGP_PlayerStateBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/GameSession.h"
#include "GameMapsSettings.h"
#include "Log/AGPLogChannels.h"

AAGPGM_Survial::AAGPGM_Survial()
{

}

void AAGPGM_Survial::InitGameState()
{
	Super::InitGameState();

	if (AAGPGameState_Survial* GSSurvial = GetGameState<AAGPGameState_Survial>())
	{
		/* TODO : 현재 상태에 대한 Duration 지정 필요 */
		GSSurvial->SetSurvialMode(CurrentModeState, 0.f, CurrentWaveRound);
		GSSurvial->SetCurrentLiveNPCNum(GetCurrentLiveNPCNum());
		GSSurvial->SetRemainNPCNum(GetRemainNPCNum());
	}
}

void AAGPGM_Survial::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	const bool bUniqueIdCheckOk = (!UniqueId.IsValid() || UOnlineEngineInterface::Get()->IsCompatibleUniqueNetId(UniqueId));
	if (bUniqueIdCheckOk)
	{
		ErrorMessage = GameSession->ApproveLogin(Options);
	}
	else
	{
		ErrorMessage = TEXT("incompatible_unique_net_id");
	}

	if (ErrorMessage.IsEmpty())
	{
		/* 현재 GameMode의 상태가 WaitStart가 아닌경우에는 플레이어 접속 불가 */
		if (EAGPSurvialGameModeState::WaitStart != GetCurrentModeState())
		{
			ErrorMessage = TEXT("The game has already started.");
		}
	}

	FGameModeEvents::GameModePreLoginEvent.Broadcast(this, UniqueId, ErrorMessage);
}

void AAGPGM_Survial::Logout(AController* Exiting)
{
	/* 로그아웃한 플레이어의 상태 변화 이벤트 수신 종료 */
	if (AAGP_PlayerStateBase* LogoutPlayerState = Exiting->GetPlayerState<AAGP_PlayerStateBase>())
	{
		LogoutPlayerState->OnStateInSurvivalModeChanged.RemoveAll(this);
	}

	/* 로그아웃한 플레이어 캐릭터의 Death 이벤트 수신 종료 */
	if (AAGP_CharacterBase* LogoutPlayerCharacter = Exiting->GetPawn<AAGP_CharacterBase>())
	{
		LogoutPlayerCharacter->OnUpdateDeathState.RemoveAll(this);
	}

	/* 특정 플레이어의 로그아웃 -> 다음프레임에서 남은 플레이어들의 생존여부 확인 */
	ScheduleCheckPlayersAliveNextTick();

	Super::Logout(Exiting);
}

void AAGPGM_Survial::BeginPlay()
{
	Super::BeginPlay();

	/* 맵상에 위치한 EQSPointAreaActor 수집 */
	for (TActorIterator<AEQSPointAreaActor> Iter(GetWorld()); Iter; ++Iter)
	{
		AEQSPointAreaActor* PointAreaActor = *Iter;
		checkf(PointAreaActor, TEXT("[%s], Invalid PointAreaActor"), __FUNCTIONW__);

		PointAreaActor->OnGetLocationSuccess.AddUniqueDynamic(this, &AAGPGM_Survial::OnEQSPointActorSuccess);
		PointAreaActor->OnGetLocationFail.AddUniqueDynamic(this, &AAGPGM_Survial::OnEQSPointActorFail);

		SpawnNPCPointAreaActors.Add(PointAreaActor);
	}

	/* 맵에 SpawnPointArea가 하나라도 있어야 함 */
	checkf(!SpawnNPCPointAreaActors.IsEmpty(), TEXT("[%s], SpawnNPCPointAreaActor must be placed in the Current Level"), __FUNCTIONW__);

	/* 맵상에 위치한 AGPRelayActor 수집 */
	for (TActorIterator<AAGPRelayActor> Iter(GetWorld()); Iter; ++Iter)
	{
		AAGPRelayActor* CurrentRelayActor = *Iter;
		checkf(CurrentRelayActor, TEXT("[%s], Invalid CurrentRelayActor"), __FUNCTIONW__);

		FVector Location = CurrentRelayActor->GetActorLocation();

		SpawnRelayActors.Add(CurrentRelayActor);
	}

	/* 맵에 SpawnRelayActors가 하나라도 있어야 함 */
	checkf(!SpawnRelayActors.IsEmpty(), TEXT("[%s], SpawnRelayActors must be placed in the Current Level"), __FUNCTIONW__);

	/* 맵상에 위치한 InteractableObject 확인 */
	for (TActorIterator<AAGPInteractableObjectBase> Iter(GetWorld()); Iter; ++Iter)
	{
		AAGPInteractableObjectBase* CurrentInteractableObject = *Iter;
		checkf(CurrentInteractableObject, TEXT("[%s], Invalid CurrentInteractableObject"), __FUNCTIONW__);

		if (EAGPInteractableActorState::Available != CurrentInteractableObject->GetCurrentState())
		{
			continue;
		}

		CurrentInteractableObject->OnInteractableObjectStateChanged.AddUniqueDynamic(this, &AAGPGM_Survial::OnWaveTriggerActorStateChanged);

		/* 첫 액터만 사용 */
		break;
	}


	/* 초기 - Wait Start 설정 */
	SetSurvialModeState(EAGPSurvialGameModeState::WaitStart);
}

void AAGPGM_Survial::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	/* EQS Spawn Point 정보 정리 */
	ClearSpawnPointLocations();

	/* EQSPointAreaActor 정리 */
	ClearAllEQSPointActor();
}

FString AAGPGM_Survial::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	if (NewPlayerController)
	{
		/* 해당 플레이어의 PlayerState를 준비상태로 만든다. */
		if (AAGP_PlayerStateBase* NewPlayerState = NewPlayerController->GetPlayerState<AAGP_PlayerStateBase>())
		{
			NewPlayerState->SetCurrentStateInSurvivalMode(EAGPSurvivalModePlayerState::Prepare);
			NewPlayerState->OnStateInSurvivalModeChanged.AddUObject(this, &AAGPGM_Survial::OnPlayerStateInSurvivalModeChanged);
		}
	}

	return ErrorMessage;
}

void AAGPGM_Survial::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	checkf(NewPlayer, TEXT("[%s], Invalid NewPlayer"), __FUNCTIONW__);
	
 	const bool IsPlayingState = NAME_Playing == NewPlayer->GetStateName();
	AAGP_CharacterBase* NewPlayerCharacter = NewPlayer->GetPawn<AAGP_CharacterBase>();
	if (IsPlayingState && NewPlayerCharacter)
	{
		/* 플레이어의 캐릭터가 존재 - 리스타트 성공 */
		AAGP_PlayerStateBase* NewPlayerState = NewPlayer->GetPlayerState<AAGP_PlayerStateBase>();
		checkf(NewPlayerState, TEXT("[%s], Invalid NewPlayerState"), __FUNCTIONW__);

		/* 플레이어 캐릭터의 사망에 대한 이벤트 수신 */
		NewPlayerCharacter->OnUpdateDeathState.AddDynamic(this, &AAGPGM_Survial::OnPlayerCharacterUpdateDeath);

		/* 플레이어 상태 Active로 변경 */
		NewPlayerState->SetCurrentStateInSurvivalMode(EAGPSurvivalModePlayerState::Active);
	}
}

void AAGPGM_Survial::SetSurvialModeState(EAGPSurvialGameModeState NewState)
{
	/* 이전에 지정된 타이머 핸들 초기화 */
	FTimerManager& CurrentTimerManager = GetCurrentTimerManager();
	CurrentTimerManager.ClearTimer(CurrentStateTimeUpHandle);

	/* 현재 상태 종료 */
	OnExitCurrentModeState();

	CurrentModeState = NewState;

	/* 새로 전환할 상태가 Duration을 가지고 있다면 타이머 설정 */
	float NewStateDuration = 0.f;
	if (MapDurationAndNextMode.Contains(CurrentModeState))
	{
		NewStateDuration = MapDurationAndNextMode[CurrentModeState].ModeDuration;

		UWorld* CurrentWorld = GetWorld();
		checkf(CurrentWorld, TEXT("[%s], Invalid CurrentWorld"), __FUNCTIONW__);

		CurrentTimerManager.SetTimer(CurrentStateTimeUpHandle, this, &AAGPGM_Survial::OnCurrentStateTimeUp, NewStateDuration);
	}

	OnChangeNewModeState();

	/* GameState에 현재 상태 공유 */
	if (AAGPGameState_Survial* GSSurvial = GetGameState<AAGPGameState_Survial>())
	{
		GSSurvial->SetSurvialMode(CurrentModeState, NewStateDuration, CurrentWaveRound);
	}
}

void AAGPGM_Survial::SetCurrentLiveNPCNum(const int32 NewNPCNum)
{
	CurrentLiveNPCNum = FMath::Max(NewNPCNum, 0);

	if (AAGPGameState_Survial* GSSurvial = GetGameState<AAGPGameState_Survial>())
	{
		GSSurvial->SetCurrentLiveNPCNum(CurrentLiveNPCNum);
	}
}

void AAGPGM_Survial::SetRemainNPCNum(const int32 NewNPCNum)
{
	CurrentRemainNPCNum = FMath::Max(NewNPCNum, 0);

	if (AAGPGameState_Survial* GSSurvial = GetGameState<AAGPGameState_Survial>())
	{
		GSSurvial->SetRemainNPCNum(CurrentRemainNPCNum);
	}
}

void AAGPGM_Survial::OnCurrentStateTimeUp()
{
	const EAGPSurvialGameModeState CurrentState = GetCurrentModeState();

	/* 현재 상태의 시간이 경과되었을때, 다음 상태로 전환한다. */
	const FAGPNextSurvialGameMode* NextModeContext = MapDurationAndNextMode.Find(CurrentState);
	checkf(NextModeContext, TEXT("[%s], Invalid NextModeContext"), __FUNCTIONW__);

	SetSurvialModeState(NextModeContext->NextModeState);
}

void AAGPGM_Survial::OnChangeNewModeState()
{
	const EAGPSurvialGameModeState CurrentState = GetCurrentModeState();
	AGP_LOG(LogAGPGameMode, Log, TEXT("[%s] Begin"), *UEnum::GetDisplayValueAsText(CurrentState).ToString());

	switch (CurrentState)
	{
	case EAGPSurvialGameModeState::WaitStart:
		{

		}
		break;

	case EAGPSurvialGameModeState::WaitForNextWave:
		{
			/* 현재 Wave Round 증가 */
			++CurrentWaveRound;
		}
		break;

	case EAGPSurvialGameModeState::PreCurrentWave:
		{
			/* EQS실행전 스폰가능한 위치정보 초기화 */
			ClearSpawnPointLocations();

			/* 몬스터들을 소환할 위치를 구하기위한 EQS 실행 */
			RunAllEQSPointActor();
		}
		break;

	case EAGPSurvialGameModeState::InProgressCurrentWave:
		{

		}
		break;

	case EAGPSurvialGameModeState::CompleteCurrentWave:
		{

		}
		break;

	case EAGPSurvialGameModeState::PreWaveComplete:
		{
			
		}
		break;

	case EAGPSurvialGameModeState::AllWavesClear:
		{

		}
		break;

	case EAGPSurvialGameModeState::PlayersDefeat:
		{
			
		}
		break;

	case EAGPSurvialGameModeState::GameEndPlayerKick:
		{
			/* 게임 종료 전 남은 플레이어를 모두 쫓아냄 */
			KickAllPlayers();
		}
		break;

	case EAGPSurvialGameModeState::Shutdown:
		{
			/* 서버 프로세스 종료 */
			ShutdownGame();
		}
		break;

	default:
		{
			
		}
		break;
	}	
}

void AAGPGM_Survial::OnExitCurrentModeState()
{
	const EAGPSurvialGameModeState CurrentState = GetCurrentModeState();
	AGP_LOG(LogAGPGameMode, Log, TEXT("[%s] Finish"), *UEnum::GetDisplayValueAsText(CurrentState).ToString());

	switch (CurrentState)
	{
	case EAGPSurvialGameModeState::None:
		{
			
		}
		break;

	case EAGPSurvialGameModeState::WaitStart:
		{

		}
		break;

	case EAGPSurvialGameModeState::WaitForNextWave:
		{

		}
		break;

	case EAGPSurvialGameModeState::PreCurrentWave:
		{

		}
		break;

	case EAGPSurvialGameModeState::InProgressCurrentWave:
		{

		}
		break;

	case EAGPSurvialGameModeState::CompleteCurrentWave:
		{

		}
		break;

	case EAGPSurvialGameModeState::PreWaveComplete:
		{

		}
		break;

	case EAGPSurvialGameModeState::AllWavesClear:
		{
			
		}
		break;

	case EAGPSurvialGameModeState::PlayersDefeat:
		{

		}
		break;

	case EAGPSurvialGameModeState::GameEndPlayerKick:
		{
			
		}
		break;

	case EAGPSurvialGameModeState::Shutdown:
		{
			
		}
		break;

	default:
		{

		}
		break;
	}
}

void AAGPGM_Survial::RunAllEQSPointActor()
{
	bool IsRunEQSSuccess = false;
	for (AEQSPointAreaActor* CurrentEQSPointAreaActor : SpawnNPCPointAreaActors)
	{
		if (!CurrentEQSPointAreaActor)
		{
			continue;
		}

		if (CurrentEQSPointAreaActor->RunEQS())
		{
			IsRunEQSSuccess = true;
		}
		else
		{
			AGP_LOG(LogAGPGameMode, Warning, TEXT("[%s] Run EQS Fail.."), *CurrentEQSPointAreaActor->GetActorNameOrLabel());
		}
	}

	if (!IsRunEQSSuccess)
	{
		//TODO : 오류 처리
		AGP_LOG(LogAGPGameMode, Error, TEXT("All EQS Fail.."));
	}
}

void AAGPGM_Survial::ClearAllEQSPointActor()
{
	for (AEQSPointAreaActor* CurrentEQSPointAreaActor : SpawnNPCPointAreaActors)
	{
		if (!CurrentEQSPointAreaActor)
		{
			continue;
		}

		/* EQS 중지 */
		CurrentEQSPointAreaActor->StopEQS();

		/* 델리게이트 제거 */
		CurrentEQSPointAreaActor->OnGetLocationSuccess.RemoveAll(this);
		CurrentEQSPointAreaActor->OnGetLocationFail.RemoveAll(this);
	}

	SpawnNPCPointAreaActors.Empty();
}

void AAGPGM_Survial::ClearSpawnPointLocations()
{
	EnableSpawnPointLocations.Empty();
}

void AAGPGM_Survial::OnNextFrameSpawnNPC()
{
	if (EnableSpawnPointLocations.IsEmpty())
	{
		//TODO : 오류 처리
		AGP_LOG(LogAGPGameMode, Error, TEXT("EnableSpawnPointLocations is empty.."));
		return;
	}

	/* 위치 후보군들을 뒤섞음(셔플) */
	UAGPCommonFunctionLibrary::ShuffleArray(EnableSpawnPointLocations);

	/* 몬스터 스폰 정보 확인해서 스폰 */
	const int32 WaveSpawnIndex = CurrentWaveRound - 1;
	checkf(WaveSpawnInfos.IsValidIndex(WaveSpawnIndex), TEXT("[%s], Invalid Wave Spawn Index, Index: [%d], Wave: [%d]"), __FUNCTIONW__, WaveSpawnIndex, CurrentWaveRound);

	int32 CountSpawnNPCNum = 0;
	int32 UseSpawnPointLocationIndex = 0;
	const FAGPWaveSpawnInfo& SpawnInfo = WaveSpawnInfos[WaveSpawnIndex];
	for (const FAGPWaveSpawnNPCInfo& SpawnNPCInfo : SpawnInfo.SpawnList)
	{
		checkf(SpawnNPCInfo.SpawnNPCClass, TEXT("[%s], Invalid SpawnNPCInfo NPCClass, WaveRound[%d]"), __FUNCTIONW__, CurrentWaveRound);

		const int32 RandomSpawnNum = FMath::RandRange(SpawnNPCInfo.MinSpawnNum, SpawnNPCInfo.MaxSpawnNum);
		const int32 RemainSpawnPointNum = FMath::Max(0, EnableSpawnPointLocations.Num() - UseSpawnPointLocationIndex);
		const int32 NumToSpawn = FMath::Min(RandomSpawnNum, RemainSpawnPointNum);
		if (NumToSpawn <= 0)
		{
			/* 더이상 스폰할 위치가 유효하지 않으면 스폰 중단*/
			break;
		}

		for (int32 SpawnNPCIndex = 0; SpawnNPCIndex < NumToSpawn; ++SpawnNPCIndex)
		{
			const FVector SpawnLocaiton = EnableSpawnPointLocations[UseSpawnPointLocationIndex++];
			const FRotator SpawnRotator(0.f, UKismetMathLibrary::RandomFloatInRange(-180.f, 180.f), 0.f);
			const FTransform SpawnTransform(SpawnRotator, SpawnLocaiton);

			AAGP_NPCBase* SpawnedNPC = SpawnNPC(SpawnNPCInfo.SpawnNPCClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (!SpawnedNPC)
			{
				AGP_LOG(LogAGPGameMode, Warning, TEXT("Spawn NPC Fail.., WaveRound[%d], Location[%s]"), CurrentWaveRound, *SpawnLocaiton.ToString());
				continue;
			}

			/* 스폰된 NPC의 Death, Destroy 이벤트 바인딩 */
			SpawnedNPC->OnUpdateDeathState.AddUniqueDynamic(this, &AAGPGM_Survial::OnNPCDeathStateChange);
			SpawnedNPC->OnDestroyed.AddUniqueDynamic(this, &AAGPGM_Survial::OnDestroySpawnedNPC);

			++CountSpawnNPCNum;
		}
	}

	/* 스폰된 몬스터들에게 플레이어들의 존재 알림 */
	PlayerExistNotifyToNPC();

	/* 스폰된 몬스터 수 저장 */
	SetCurrentLiveNPCNum(CountSpawnNPCNum);
	SetRemainNPCNum(CountSpawnNPCNum);

	/* 위치 초기화 */
	ClearSpawnPointLocations();

	/* 다음 단계로 진행(웨이브 시작) */
	SetSurvialModeState(EAGPSurvialGameModeState::InProgressCurrentWave);
}

void AAGPGM_Survial::NotifyPlayerExistenceToNPC(APawn* InPlayerPawn)
{
	if (!InPlayerPawn || UAGPAbilitySystemFunctionLibrary::IsTargetDeath(InPlayerPawn))
	{
		return;
	}

	/* 맵상에 배치된 각 Relay Actor들을 통해 플레이어 캐릭터의 존재를 스폰된 NPC들에게 알림 */
	for (AAGPRelayActor* RelayActor : SpawnRelayActors)
	{
		if (!RelayActor)
		{
			continue;
		}

		RelayActor->NotifyPlayerExistenceToNPC(InPlayerPawn);
	}
}

void AAGPGM_Survial::OnPlayerStateInSurvivalModeChanged(const AAGP_PlayerStateBase* ChangedPS, EAGPSurvivalModePlayerState CurrentState)
{
	checkf(ChangedPS, TEXT("[%s], Invalid ChangedPS"), __FUNCTIONW__);

	AGP_LOG(LogAGPGameMode, Log, TEXT("PlayerState[%s], CurrentState[%s]"), *ChangedPS->GetPlayerName(), *UEnum::GetDisplayValueAsText(CurrentState).ToString());

	ScheduleCheckPlayersAliveNextTick();
}

void AAGPGM_Survial::ScheduleCheckPlayersAliveNextTick()
{
	/* InProgressCurrentWave일때만 처리 */
	if (EAGPSurvialGameModeState::InProgressCurrentWave != GetCurrentModeState())
	{
		return;
	}

	/* 이전에 지정된 타이머 핸들 초기화 */
	FTimerManager& CurrentTimerManager = GetCurrentTimerManager();
	CurrentTimerManager.ClearTimer(NextTickPlayerStateCheckHandle);

	/* 다음 틱에서 플레이어들의 상태를 확인하는 함수 실행 예약 */
	NextTickPlayerStateCheckHandle = CurrentTimerManager.SetTimerForNextTick(this, &AAGPGM_Survial::OnNextTickPlayerCheck);
}

void AAGPGM_Survial::OnNextTickPlayerCheck()
{
	/* 타이머 핸들 초기화 */
	FTimerManager& CurrentTimerManager = GetCurrentTimerManager();
	CurrentTimerManager.ClearTimer(NextTickPlayerStateCheckHandle);

	/* InProgressCurrentWave일때만 처리 */
	if (EAGPSurvialGameModeState::InProgressCurrentWave != GetCurrentModeState())
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator();
		Iterator;
		++Iterator)
	{
		/* 현재 접속한 플레이어들의 상태 확인 */
		APlayerController* CurrentPC = Iterator->Get();
		if (!CurrentPC)
		{
			continue;
		}

		AAGP_PlayerStateBase* CurrentPS = CurrentPC->GetPlayerState<AAGP_PlayerStateBase>();
		if (!CurrentPS)
		{
			continue;
		}

		/* 플레이어중 하나라도 살아있는경우 게임 계속 진행 */
		if (CurrentPS->IsAliveInSurvivalMode())
		{
			return;
		}
	}

	/* 모든 플레이어 사망, 게임 종료 */
	SetSurvialModeState(EAGPSurvialGameModeState::PlayersDefeat);
}

void AAGPGM_Survial::OnNextTickWaveStart()
{
	FTimerManager& CurrentTimerManager = GetCurrentTimerManager();
	CurrentTimerManager.ClearTimer(NextTickWaveStartHandle);

	SetSurvialModeState(EAGPSurvialGameModeState::WaitForNextWave);
}

void AAGPGM_Survial::PlayerExistNotifyToNPC()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator();
		Iterator;
		++Iterator)
	{
		/* 현재 접속한 플레이어들의 컨트롤러들 확인 */
		APlayerController* CurrentPC = Iterator->Get();
		if (!CurrentPC || nullptr == CurrentPC->PlayerState || MustSpectate(CurrentPC))
		{
			continue;
		}

		/* 플레이어의 폰을 몬스터들에게 알림 */
		NotifyPlayerExistenceToNPC(CurrentPC->GetPawn());
	}
}

void AAGPGM_Survial::OnEQSPointActorSuccess(AEQSPointAreaActor* NotifyActor, const TArray<FVector>& SpawnLocations)
{
	/* PreCurrentWave일때만 처리 */
	if (EAGPSurvialGameModeState::PreCurrentWave != GetCurrentModeState())
	{
		return;
	}

	/* EQS 결과 위치를 모두 합침 */
	EnableSpawnPointLocations.Append(SpawnLocations);

	/* 모든 EQS가 실행되면(성공 or 실패 상관없이) 다음 프레임에서 몬스터 스폰 후 진행단계로 넘어감*/
	if (IsAllEQSPointActorDone())
	{
		GetCurrentTimerManager().SetTimerForNextTick(this, &AAGPGM_Survial::OnNextFrameSpawnNPC);
	}
}

void AAGPGM_Survial::OnEQSPointActorFail(AEQSPointAreaActor* NotifyActor)
{
	/* PreCurrentWave일때만 처리 */
	if (EAGPSurvialGameModeState::PreCurrentWave != GetCurrentModeState())
	{
		return;
	}

	/* 모든 EQS가 실행되면(성공 or 실패 상관없이) 다음 프레임에서 몬스터 스폰 후 진행단계로 넘어감*/
	if (IsAllEQSPointActorDone())
	{
		GetCurrentTimerManager().SetTimerForNextTick(this, &AAGPGM_Survial::OnNextFrameSpawnNPC);
	}
}

void AAGPGM_Survial::OnNPCDeathStateChange(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState)
{
	if (!UpdatedCharacter || EAGPCharacterDeath::Destroy != UpdateState)
	{
		return;
	}

	UpdatedCharacter->OnUpdateDeathState.RemoveAll(this);

	SetCurrentLiveNPCNum(GetCurrentLiveNPCNum() - 1);

	/* 살아있는 NPC가 더이상 존재하지 않음(Destroy상태의 NPC는 존재) */
	if (0 == GetCurrentLiveNPCNum())
	{
		const int32 NextWaveRound = GetCurrentWaveRound() + 1;
		if (WaveSpawnInfos.IsValidIndex(NextWaveRound - 1))
		{
			/* 다음 웨이브 정보가 있으면 현재 웨이브 완료 */
			SetSurvialModeState(EAGPSurvialGameModeState::CompleteCurrentWave);
		}
		else
		{
			/* 그렇지 않으면 Complete로 진행 */
			SetSurvialModeState(EAGPSurvialGameModeState::PreWaveComplete);
		}
	}
}

void AAGPGM_Survial::OnDestroySpawnedNPC(AActor* DestroyedNPC)
{
	SetRemainNPCNum(GetRemainNPCNum() - 1);

	/* 모든 NPC가 Destroy까지 된 상황이면 */
	if (0 == GetRemainNPCNum())
	{
		switch (GetCurrentModeState())
		{
		case EAGPSurvialGameModeState::CompleteCurrentWave:
			{
				/* 현재 웨이브 완료 상태면 다음 웨이브 진행 */
				SetSurvialModeState(EAGPSurvialGameModeState::WaitForNextWave);
			}
			break;

		case EAGPSurvialGameModeState::PreWaveComplete:
			{
				/* 모든 웨이브를 완료한 상태면 승리화면 상태로 진입 */
				SetSurvialModeState(EAGPSurvialGameModeState::AllWavesClear);
			}
			break;
		}
	}
}

void AAGPGM_Survial::OnPlayerCharacterUpdateDeath(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState)
{
	checkf(UpdatedCharacter, TEXT("[%s], Invalid UpdatedCharacter, State[%s]"), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(UpdateState).ToString());

	/* 캐릭터 Death이후 Destroy 상태일때만 처리 */
	if (EAGPCharacterDeath::Destroy != UpdateState)
	{
		return;
	}

	AAGP_PlayerStateBase* DeathPlayerState = UpdatedCharacter->GetPlayerState<AAGP_PlayerStateBase>();
	checkf(DeathPlayerState, TEXT("[%s], Invalid DeathPlayerState"), __FUNCTIONW__);

	DeathPlayerState->SetCurrentStateInSurvivalMode(EAGPSurvivalModePlayerState::Inactive);

	//델리게이트 뒷정리
	UpdatedCharacter->OnUpdateDeathState.RemoveAll(this);

	//Spectator 처리 
	if (APlayerController* DeathPlayerController = UpdatedCharacter->GetController<APlayerController>())
	{
		ASpectatorPawn* SpectatorPawn = GetWorld()->SpawnActor<ASpectatorPawn>(
				SpectatorClass,
				UpdatedCharacter->GetPawnViewLocation(),
				UpdatedCharacter->GetControlRotation());

		if (SpectatorPawn)
		{
			DeathPlayerController->UnPossess();
			DeathPlayerController->Possess(SpectatorPawn);	
		}
	}
}

void AAGPGM_Survial::OnWaveTriggerActorStateChanged(EAGPInteractableActorState CurrentState)
{
	/* 트리거 액터가 활성화된 순간에만 설정 */
	if (EAGPInteractableActorState::Activated != CurrentState)
	{
		return;
	}

	/* WaitStart일때만 처리 */
	if (EAGPSurvialGameModeState::WaitStart != GetCurrentModeState())
	{
		return;
	}

	/* 타이머가 유효하지 않을때만 처리 */
	FTimerManager& CurrentTimerManager = GetCurrentTimerManager();
	if (CurrentTimerManager.IsTimerActive(NextTickWaveStartHandle))
	{
		return;
	}

	/* 다음 틱에서 플레이어들의 상태를 확인하는 함수 실행 예약 */
	NextTickWaveStartHandle = CurrentTimerManager.SetTimerForNextTick(this, &AAGPGM_Survial::OnNextTickWaveStart);
}

bool AAGPGM_Survial::IsAllEQSPointActorDone() const
{
	for (AEQSPointAreaActor* CurrentEQSPointAreaActor : SpawnNPCPointAreaActors)
	{
		if (!CurrentEQSPointAreaActor)
		{
			continue;
		}

		if (CurrentEQSPointAreaActor->IsRunningEQS())
		{
			return false;
		}
	}

	return true;
}