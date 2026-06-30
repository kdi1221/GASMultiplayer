// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AGPAbilityTask_EQSSpawnLocation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEQSSpawnLocationSuccessSignature, const TArray<FVector>&, SpawnLocations);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEQSSpawnLocationFailSignature);

struct FEnvQueryResult;
class UEnvQuery;

/**
 * 지정된 EQS를 통해 Spawn 위치들 반환
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPAbilityTask_EQSSpawnLocation : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FEQSSpawnLocationSuccessSignature GetLocationSuccess;

	UPROPERTY(BlueprintAssignable)
	FEQSSpawnLocationFailSignature GetLocationFail;

private:
	/* 실행할 Query Template*/
	UPROPERTY()
	UEnvQuery* ExecuteQueryTemplate;

	/* 가져올 위치 갯수 */
	int32 RequestLocationNum = 0;

	/* 실행중인 EQS 인덱스  */
	int32 ExecuteEQSIndex = INDEX_NONE;
	
public:
	UFUNCTION(BlueprintCallable, Category = "AGP|AbilityTasks", meta = (DisplayName = "GetEQSSpawnLocation", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAGPAbilityTask_EQSSpawnLocation* CreateEQSSpawnLocation(UGameplayAbility* OwningAbility, UEnvQuery* InQueryTemplate, int32 InRequestNum);

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished);
	
private:
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);

	void NotifySuccess(const TArray<FVector>& ResultLocations);
	void NotifyFail();

	void ClearEQSRequestInfo();
};
