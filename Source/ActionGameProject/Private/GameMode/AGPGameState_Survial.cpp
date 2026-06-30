// KJY All Rights Reserved


#include "GameMode/AGPGameState_Survial.h"
#include "Net/UnrealNetwork.h"
#include "Log/AGPLogChannels.h"

void AAGPGameState_Survial::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAGPGameState_Survial, CurrentSurvialModeContext);
	DOREPLIFETIME(AAGPGameState_Survial, CurrentLiveNPCNum);
	DOREPLIFETIME(AAGPGameState_Survial, CurrentRemainNPCNum);
}

void AAGPGameState_Survial::SetSurvialMode(EAGPSurvialGameModeState NewState, float NewStateDuration, int32 WaveRound)
{
	checkf(HasAuthority(), TEXT("[%s] Only Call Authority"), __FUNCTIONW__);

	CurrentSurvialModeContext.ModeState = NewState;
	CurrentSurvialModeContext.ModeStartTime = GetServerWorldTimeSeconds();
	CurrentSurvialModeContext.ModeDuration = NewStateDuration;
	CurrentSurvialModeContext.CurrentWaveRound = WaveRound;

	OnChangeCurrentSurvialModeContext();
}

void AAGPGameState_Survial::SetCurrentLiveNPCNum(int32 NewNPCNum)
{
	CurrentLiveNPCNum = NewNPCNum;

	OnChangeCurrentLiveNPCNum();
}

void AAGPGameState_Survial::SetRemainNPCNum(int32 NewNPCNum)
{
	CurrentRemainNPCNum = NewNPCNum;

	OnChangeCurrentRemainNPCNum();
}

void AAGPGameState_Survial::OnRep_CurrentSurvialModeContext(const FAGPSurvialGameModeContext& BeforeGameModeContext)
{
	OnChangeCurrentSurvialModeContext();
}

void AAGPGameState_Survial::OnRep_CurrentLiveNPCNum()
{
	OnChangeCurrentLiveNPCNum();
}

void AAGPGameState_Survial::OnRep_CurrentRemainNPCNum()
{
	OnChangeCurrentRemainNPCNum();
}

void AAGPGameState_Survial::OnChangeCurrentSurvialModeContext()
{
	AGP_NET_LOG(this, LogAGPGameState, Log, TEXT("Survial Mode Change, Current[%s]"), *CurrentSurvialModeContext.ToString());

	OnChangeSurvialModeState.Broadcast(CurrentSurvialModeContext);
}

void AAGPGameState_Survial::OnChangeCurrentLiveNPCNum()
{
	AGP_NET_LOG(this, LogAGPGameState, Log, TEXT("Survial Mode NPC Live Num Change, Current[%d]"), CurrentLiveNPCNum);

	OnNotifyChangeCurrentLiveNPCNum.Broadcast(CurrentLiveNPCNum);
}

void AAGPGameState_Survial::OnChangeCurrentRemainNPCNum()
{
	AGP_NET_LOG(this, LogAGPGameState, Log, TEXT("Survial Mode NPC Remain Num Change, Current[%d]"), CurrentRemainNPCNum);

	OnNotifyChangeRemainNPCNum.Broadcast(CurrentRemainNPCNum);
}