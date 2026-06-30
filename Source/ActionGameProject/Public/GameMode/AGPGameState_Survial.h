// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameMode/AGPGameStateBase.h"
#include "Common/AGPCommonStructs.h"
#include "AGPGameState_Survial.generated.h"

/**
 * 생존모드 GameState
 */


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangeSurvialModeStateSignature, const FAGPSurvialGameModeContext&, CurrentSurvialModeState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangeSurvialModeNPCNumSignature, const int32, CurrentNPCNum);


UCLASS()
class ACTIONGAMEPROJECT_API AAGPGameState_Survial : public AAGPGameStateBase
{
	GENERATED_BODY()
	
private:
	/* 현재 GameMode 상태 */
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentSurvialModeContext)
	FAGPSurvialGameModeContext CurrentSurvialModeContext;

	/* 현재 살아있는 NPC 수 */
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentLiveNPCNum)
	int32 CurrentLiveNPCNum = 0;

	/* 현재 남아있는 NPC 수(Death 상태에서 Destroy 대기 상태의 NPC포함) */
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_CurrentRemainNPCNum)
	int32 CurrentRemainNPCNum = 0;

public:
	UPROPERTY(BlueprintAssignable)
	FChangeSurvialModeStateSignature OnChangeSurvialModeState;

	UPROPERTY(BlueprintAssignable)
	FChangeSurvialModeNPCNumSignature OnNotifyChangeCurrentLiveNPCNum;

	UPROPERTY(BlueprintAssignable)
	FChangeSurvialModeNPCNumSignature OnNotifyChangeRemainNPCNum;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void SetSurvialMode(EAGPSurvialGameModeState NewState, float NewStateDuration, int32 WaveRound);
	void SetCurrentLiveNPCNum(int32 NewNPCNum);
	void SetRemainNPCNum(int32 NewNPCNum);

private:
	UFUNCTION()
	void OnRep_CurrentSurvialModeContext(const FAGPSurvialGameModeContext& BeforeGameModeContext);

	UFUNCTION()
	void OnRep_CurrentLiveNPCNum();

	UFUNCTION()
	void OnRep_CurrentRemainNPCNum();

public:
	UFUNCTION(BlueprintPure)
	const FAGPSurvialGameModeContext& GetCurrentModeContext() const { return CurrentSurvialModeContext; }

	UFUNCTION(BlueprintPure)
	const int32 GetCurrentLiveNPCNum() const { return CurrentLiveNPCNum; }

	UFUNCTION(BlueprintPure)
	const int32 GetCurrentRemainNPCNum() const { return CurrentRemainNPCNum; }

private:
	void OnChangeCurrentSurvialModeContext();
	void OnChangeCurrentLiveNPCNum();
	void OnChangeCurrentRemainNPCNum();
};
