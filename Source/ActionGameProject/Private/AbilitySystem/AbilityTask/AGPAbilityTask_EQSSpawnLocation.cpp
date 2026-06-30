// KJY All Rights Reserved


#include "AbilitySystem/AbilityTask/AGPAbilityTask_EQSSpawnLocation.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Common/AGPCommonFunctionLibrary.h"
#include "Log/AGPLogChannels.h"

UAGPAbilityTask_EQSSpawnLocation* UAGPAbilityTask_EQSSpawnLocation::CreateEQSSpawnLocation(UGameplayAbility* OwningAbility, UEnvQuery* InQueryTemplate, int32 InRequestNum)
{
	UAGPAbilityTask_EQSSpawnLocation* NewEQSSpawnLocation = NewAbilityTask<UAGPAbilityTask_EQSSpawnLocation>(OwningAbility);
	checkf(NewEQSSpawnLocation, TEXT("[%s], NewEQSSpawnLocation Invalid"), __FUNCTIONW__);

	NewEQSSpawnLocation->ExecuteQueryTemplate = InQueryTemplate;
	checkf(NewEQSSpawnLocation->ExecuteQueryTemplate, TEXT("[%s], QueryTemplate Invalid"), __FUNCTIONW__);

	NewEQSSpawnLocation->RequestLocationNum = InRequestNum;
	checkf(0 < NewEQSSpawnLocation->RequestLocationNum, TEXT("[%s], RequestLocationNum[%d] Invalid"), __FUNCTIONW__, NewEQSSpawnLocation->RequestLocationNum);

	return NewEQSSpawnLocation;
}

void UAGPAbilityTask_EQSSpawnLocation::Activate()
{
	AActor* QuerierAvatarActor = GetAvatarActor(); 
	if (!QuerierAvatarActor)
	{
		AGP_LOG(LogAGPAI, Verbose, TEXT("AvatarActor Invalid"));
		NotifyFail();
		EndTask();
		return;
	}

	FEnvQueryRequest QueryRequest(ExecuteQueryTemplate, QuerierAvatarActor);
	ExecuteEQSIndex = QueryRequest.Execute(EEnvQueryRunMode::AllMatching, this, &UAGPAbilityTask_EQSSpawnLocation::OnQueryFinished);
	if (INDEX_NONE == ExecuteEQSIndex)
	{
		AGP_LOG(LogAGPAI, Verbose, TEXT("Request Query Fail.. Task[%s], AvatarActor[%s]"), *GetNameSafe(this), *GetNameSafe(QuerierAvatarActor));
		NotifyFail();
		EndTask();
		return;
	}
}

void UAGPAbilityTask_EQSSpawnLocation::OnDestroy(bool bInOwnerFinished)
{
	/* 아직 EQS 진행중이면 중단 요청하고 정리한다. */
	if (INDEX_NONE != ExecuteEQSIndex)
	{
		UEnvQueryManager* EnvQueryManager = UEnvQueryManager::GetCurrent(this);
		if (EnvQueryManager)
		{
			EnvQueryManager->AbortQuery(ExecuteEQSIndex);
		}

		ClearEQSRequestInfo();
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAGPAbilityTask_EQSSpawnLocation::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	ClearEQSRequestInfo();

	if (!Result.IsValid() || !Result->IsSuccessful())
	{
		AGP_LOG(LogAGPAI, Verbose, TEXT("Query Fail.. Task[%s], Result[%s]"), 
			*GetNameSafe(this),
			Result.IsValid() ? *UEnum::GetDisplayValueAsText(Result->GetRawStatus()).ToString() : TEXT("Invalid"));

		NotifyFail();
		EndTask();
		return;
	}

	/* EQS로 위치 후보군들을 얻어옴 */
	TArray<FVector> EQSLocations;
	Result->GetAllAsLocations(EQSLocations);
	if (EQSLocations.IsEmpty())
	{
		AGP_LOG(LogAGPAI, Verbose, TEXT("Query Success.. But Location Empty"));

		NotifyFail();
		EndTask();
		return;
	}

	/* 위치 후보군들을 뒤섞음(셔플) */
	UAGPCommonFunctionLibrary::ShuffleArray(EQSLocations);

	/* EQS 결과 배열의 크기가 요청갯수보다 작으면 배열 크기만큼만 반환 */
	const int32 SpawnLocationNum = FMath::Min(EQSLocations.Num(), RequestLocationNum);

	/* 선정된 위치들을 지정된 개수만큼 반환 */
	TArray<FVector> ResultLocations(EQSLocations.GetData(), SpawnLocationNum);
	NotifySuccess(ResultLocations);

	EndTask();
}

void UAGPAbilityTask_EQSSpawnLocation::NotifySuccess(const TArray<FVector>& ResultLocations)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		GetLocationSuccess.Broadcast(ResultLocations);
	}
}

void UAGPAbilityTask_EQSSpawnLocation::NotifyFail()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		GetLocationFail.Broadcast();
	}
}

void UAGPAbilityTask_EQSSpawnLocation::ClearEQSRequestInfo()
{
	if (INDEX_NONE == ExecuteEQSIndex)
	{
		return;
	}

	ExecuteEQSIndex = INDEX_NONE;
}
