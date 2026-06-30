// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Common/AGPCommonEnums.h"

/* 제한시간(TotalTime)내 주기(IntervalTime)마다 실행되는 블루프린트 Action Node */
class FAGPCountdownLatentAction : public FPendingLatentAction
{
private:
	/* 실행 도중 취소 요청 입력 여부 */
	bool bNeedToCancel = false;

	/* 해당 Action의 최대 실행 시간*/
	float TotalTime = 0.f;

	/* 해당 Action의 실행 주기 */
	float IntervalTime = 0.f;

	/* 해당 Action의 남은 시간(출력) */
	float& OutRemainingTime;

	/* 해당 Action의 실행 출력 */
	EAGPCountdownActionOutput& OutputAction;

	/* 해당 LatentAction의 실행함수 */
	FName ExecutionFunction;

	/* 해당 LatentAction의 출력 링크 */
	int32 OutputLink = 0;

	/* 해당 LatentAction을 실행한 대상 */
	FWeakObjectPtr CallbackTarget;

	/* 마지막 주기 실행 이후 경과 시간 */
	float ElapsedInterval = 0.f;

	/* Action 시작 이후 경과된 시간 */
	float ElapsedTimeSinceStart = 0.f;

public:
	FAGPCountdownLatentAction(float InTotalTime,
							float InIntervalTime,
							float& InOutRemainingTime,
							EAGPCountdownActionOutput& InOutputAction,
							const FLatentActionInfo& LatentInfo);

public:
	virtual void UpdateOperation(FLatentResponse& Response) override;

public:
	void CancelAction();
};
