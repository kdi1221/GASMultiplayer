// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameMode/AGP_GameModeBase.h"
#include "Common/AGPCommonEnums.h"
#include "AGPGM_Survial.generated.h"

class FTimerManager;
class AEQSPointAreaActor;
class AAGPRelayActor;
class AAGP_NPCBase;
class UGameplayEffect;
class AAGP_PlayerStateBase;

/* 현재 모드의 경과시간이 끝났을때 전환될 다음 상태 정보 */
USTRUCT(BlueprintType)
struct FAGPNextSurvialGameMode
{
	GENERATED_BODY()

public:
	/* 해당 모드가 종료되었을때 넘어갈 다음 모드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAGPSurvialGameModeState NextModeState = EAGPSurvialGameModeState::None;

	/* 해당 모드가 종료되기까지 필요한 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ModeDuration = 0.f;
};

/* 한 웨이브 내 특정 몬스터에 대한 스폰 단위 */
USTRUCT(BlueprintType)
struct FAGPWaveSpawnNPCInfo
{
	GENERATED_BODY()

public:
	/* 스폰할 몬스터 클래스 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAGP_NPCBase> SpawnNPCClass;

	/* 최소 Spawn 갯수 */
	UPROPERTY(EditDefaultsOnly)
	int32 MinSpawnNum = 1;

	/* 최대 Spawn 갯수 */
	UPROPERTY(EditDefaultsOnly)
	int32 MaxSpawnNum = 1;
};

/* 한 웨이브내에 스폰할 몬스터들 정보 */
USTRUCT(BlueprintType)
struct FAGPWaveSpawnInfo
{
	GENERATED_BODY()

public:
	/* 해당 웨이브 내 스폰할 NPC 정보 리스트 */
	UPROPERTY(EditDefaultsOnly)
	TArray<FAGPWaveSpawnNPCInfo> SpawnList;
};


/**
 * 웨이브 생존 게임모드
 */

UCLASS()
class ACTIONGAMEPROJECT_API AAGPGM_Survial : public AAGP_GameModeBase
{
	GENERATED_BODY()

private:
	/* 각 상태별 유지 시간 */
	UPROPERTY(EditDefaultsOnly)
	TMap<EAGPSurvialGameModeState, FAGPNextSurvialGameMode> MapDurationAndNextMode;

	/* 웨이브 별 스폰할 몬스터 정보 */
	UPROPERTY(EditDefaultsOnly)
	TArray<FAGPWaveSpawnInfo> WaveSpawnInfos;

private:
	/* 현재 맵에 배치된 EQSPointAreaActor들 */
	UPROPERTY()
	TSet<TObjectPtr<AEQSPointAreaActor>> SpawnNPCPointAreaActors;

	/* 현재 맵에 배치된 RelayActor들 */
	UPROPERTY()
	TSet<TObjectPtr<AAGPRelayActor>> SpawnRelayActors;

	/* 각 EQSPointAreaActor들로부터 수집된 Spawn가능한 위치들 */
	UPROPERTY()
	TArray<FVector> EnableSpawnPointLocations;

private:
	/* 현재 Survial Mode 진행 상태 */
	EAGPSurvialGameModeState CurrentModeState = EAGPSurvialGameModeState::None;

	/* 현재 Wave Round */
	int32 CurrentWaveRound = 0;

	/* 현재 살아있는 NPC 수 */
	int32 CurrentLiveNPCNum = 0;

	/* 현재 남아있는 NPC 수(Death 상태에서 Destroy 대기 상태의 NPC포함) */
	int32 CurrentRemainNPCNum = 0;

	/* 현재 상태 종료에 대한 타이머 핸들 */
	FTimerHandle CurrentStateTimeUpHandle;

	/* 플레이어들의 상태를 확인하는 타이머 핸들 */
	FTimerHandle NextTickPlayerStateCheckHandle;

	/* Wave Start를 위한 타이머 핸들 */
	FTimerHandle NextTickWaveStartHandle;

public:
	AAGPGM_Survial();

public:
	virtual void InitGameState() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

private:
	void SetSurvialModeState(EAGPSurvialGameModeState NewState);
	void SetCurrentLiveNPCNum(const int32 NewNPCNum);
	void SetRemainNPCNum(const int32 NewNPCNum);
	void OnCurrentStateTimeUp();
	void OnChangeNewModeState();
	void OnExitCurrentModeState();

private:
	/* 모든 EQSPointActor 실행 */
	void RunAllEQSPointActor();

	/* 모든 EQSPointActor에 대한 정리 작업 */
	void ClearAllEQSPointActor();

	/* 스폰 가능한 위치정보 초기화 */
	void ClearSpawnPointLocations();

	/* 다음틱 - EnableSpawnPointLocations에서 랜덤하게 위치를 선택해서 몬스터 스폰 후 모드 전환 */
	void OnNextFrameSpawnNPC();

	/* NPC에게 플레이어들의 캐릭터 존재 알림 */
	void NotifyPlayerExistenceToNPC(APawn* InPlayerPawn);

	/* 특정 플레이어의 Survival내의 상태 변경 알림 */
	void OnPlayerStateInSurvivalModeChanged(const AAGP_PlayerStateBase* ChangedPS, EAGPSurvivalModePlayerState CurrentState);

	/* 특정 플레이어의 상태 변경에 따른 다음 틱에서의 플레이어들 생존여부 검사 예약 */
	void ScheduleCheckPlayersAliveNextTick();

	/* 틱에서의 플레이어들 생존여부 검사(타이머에 의한 예약으로 호출) */
	void OnNextTickPlayerCheck();

	/* 웨이브 시작 */
	void OnNextTickWaveStart();

	/* RelayActor => 범위 내 몬스터들에게 플레이어 존재 알림 */
	void PlayerExistNotifyToNPC();

private:
	/* 특정 EQS Spawn Actor의 EQS 결과 콜백(성공) */
	UFUNCTION()
	void OnEQSPointActorSuccess(AEQSPointAreaActor* NotifyActor, const TArray<FVector>& SpawnLocations);

	/* 특정 EQS Spawn Actor의 EQS 결과 콜백(실패) */
	UFUNCTION()
	void OnEQSPointActorFail(AEQSPointAreaActor* NotifyActor);

	/* 특정 NPC Death 이벤트 처리 */
	UFUNCTION()
	void OnNPCDeathStateChange(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState);

	/* 특정 NPC가 Destroy될때 알림 */
	UFUNCTION()
	void OnDestroySpawnedNPC(AActor* DestroyedNPC);

	/* 특정 플레이어의 캐릭터가 사망했을때 알림 */
	UFUNCTION()
	void OnPlayerCharacterUpdateDeath(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState);

	/* Wave Start 트리거 액터의 상태 변경시 호출 */
	UFUNCTION()
	void OnWaveTriggerActorStateChanged(EAGPInteractableActorState CurrentState);

private:
	/* 실행된 각 EQS가 모두 실행되었는지 확인 */
	bool IsAllEQSPointActorDone() const;

public:
	FORCEINLINE EAGPSurvialGameModeState GetCurrentModeState() const { return CurrentModeState; }
	FORCEINLINE int32 GetCurrentWaveRound() const { return CurrentWaveRound; }
	FORCEINLINE int32 GetCurrentLiveNPCNum() const { return CurrentLiveNPCNum; }
	FORCEINLINE int32 GetRemainNPCNum() const { return CurrentRemainNPCNum; }
};
