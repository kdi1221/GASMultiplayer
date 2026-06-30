// KJY All Rights Reserved


#include "Levels/Actors/EQSPointAreaActor.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Log/AGPLogChannels.h"

AEQSPointAreaActor::AEQSPointAreaActor()
{
	bNetLoadOnClient = false;
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetActorHiddenInGame(true);
}

bool AEQSPointAreaActor::RunEQS()
{
	checkf(ExecuteQueryTemplate, TEXT("[%s], Invalid ExecuteQueryTemplate"), __FUNCTIONW__);
	FEnvQueryRequest QueryRequest(ExecuteQueryTemplate, this);
	ExecuteEQSIndex = QueryRequest.Execute(EEnvQueryRunMode::AllMatching, this, &AEQSPointAreaActor::OnQueryFinished);
	if (INDEX_NONE == ExecuteEQSIndex)
	{
		AGP_LOG(LogAGPSpawnNPC, Verbose, TEXT("Request EQS Fail.. AEQSPointAreaActor[%s]"), *GetNameSafe(this));
		ClearEQSRequestInfo();
		return false;
	}

	return true;
}

void AEQSPointAreaActor::StopEQS()
{
	if (!IsRunningEQS())
	{
		return;
	}

	UEnvQueryManager* EnvQueryManager = UEnvQueryManager::GetCurrent(this);
	if (EnvQueryManager)
	{
		EnvQueryManager->AbortQuery(ExecuteEQSIndex);
	}

	ClearEQSRequestInfo();
}

bool AEQSPointAreaActor::IsRunningEQS() const
{
	return INDEX_NONE != ExecuteEQSIndex;
}

void AEQSPointAreaActor::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	ClearEQSRequestInfo();

	if (!Result.IsValid() || !Result->IsSuccessful())
	{
		AGP_LOG(LogAGPAI, Verbose, TEXT("EQS Fail.. AEQSPointAreaActor[%s], Result[%s]"),
			*GetNameSafe(this),
			Result.IsValid() ? *UEnum::GetDisplayValueAsText(Result->GetRawStatus()).ToString() : TEXT("Invalid"));

		NotifyFail();
		return;
	}

	/* EQS로 위치 후보군들을 얻어옴 */
	TArray<FVector> EQSLocations;
	Result->GetAllAsLocations(EQSLocations);
	if (EQSLocations.IsEmpty())
	{
		AGP_LOG(LogAGPAI, Verbose, TEXT("Query Success.. But Location Empty"));

		NotifyFail();
		return;
	}

	NotifySuccess(EQSLocations);
}

void AEQSPointAreaActor::NotifySuccess(const TArray<FVector>& ResultLocations)
{
	OnGetLocationSuccess.Broadcast(this, ResultLocations);
}

void AEQSPointAreaActor::NotifyFail()
{
	OnGetLocationFail.Broadcast(this);
}

void AEQSPointAreaActor::ClearEQSRequestInfo()
{
	if (INDEX_NONE == ExecuteEQSIndex)
	{
		return;
	}

	ExecuteEQSIndex = INDEX_NONE;
}