// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EQSPointAreaActor.generated.h"

struct FEnvQueryResult;
class UEnvQuery;

/* 월드상에 배치되어 EQS를 통해 자신 주변의 위치 정보 반환 */
UCLASS(Abstract, Blueprintable)
class ACTIONGAMEPROJECT_API AEQSPointAreaActor : public AActor
{
	GENERATED_BODY()

private:
	/* 실행할 Query Template*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnvQuery> ExecuteQueryTemplate;

private:
	/* 실행중인 EQS 인덱스  */
	int32 ExecuteEQSIndex = INDEX_NONE;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEQSGetPointsSuccessSignature, AEQSPointAreaActor*, NotifyActor, const TArray<FVector>&, SpawnLocations);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEQSGetPointsFailSignature, AEQSPointAreaActor*, NotifyActor);

	/* EQS 실행 결과(성공) */
	UPROPERTY(BlueprintAssignable)
	FOnEQSGetPointsSuccessSignature OnGetLocationSuccess;

	/* EQS 실행 결과(실패) */
	UPROPERTY(BlueprintAssignable)
	FOnEQSGetPointsFailSignature OnGetLocationFail;

public:	
	AEQSPointAreaActor();

public:
	/* EQS 실행 */
	bool RunEQS();

	/* EQS 실행중이면 중지 */
	void StopEQS();

public:
	/* 현재 EQS 실행 여부 반환 */
	bool IsRunningEQS() const;

private:
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);

	void NotifySuccess(const TArray<FVector>& ResultLocations);
	void NotifyFail();

	void ClearEQSRequestInfo();
};
