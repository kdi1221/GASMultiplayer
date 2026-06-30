// KJY All Rights Reserved


#include "Common/LatentAction/AGPCountdownLatentAction.h"

FAGPCountdownLatentAction::FAGPCountdownLatentAction(float InTotalTime, 
													float InIntervalTime, 
													float& InOutRemainingTime, 
													EAGPCountdownActionOutput& InOutputAction, 
													const FLatentActionInfo& LatentInfo)
	: bNeedToCancel(false)
	, TotalTime(InTotalTime)
	, IntervalTime(InIntervalTime)
	, OutRemainingTime(InOutRemainingTime)
	, OutputAction(InOutputAction)
	, ExecutionFunction(LatentInfo.ExecutionFunction)
	, OutputLink(LatentInfo.Linkage)
	, CallbackTarget(LatentInfo.CallbackTarget)
	, ElapsedInterval(0.f)
	, ElapsedTimeSinceStart(0.f)
{

}

void FAGPCountdownLatentAction::UpdateOperation(FLatentResponse& Response)
{
	if (bNeedToCancel)
	{
		OutputAction = EAGPCountdownActionOutput::Cancelled;
		Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		return;
	}

	const float ElapsedTime = Response.ElapsedTime();
	ElapsedInterval += ElapsedTime;
	ElapsedTimeSinceStart += ElapsedTime;
	OutRemainingTime = FMath::Max(0.f, TotalTime - ElapsedTimeSinceStart);

	if (ElapsedTimeSinceStart >= TotalTime)
	{
		OutputAction = EAGPCountdownActionOutput::Completed;
		Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
	}
	else if (ElapsedInterval >= IntervalTime)
	{
		ElapsedInterval = 0.f;

		OutputAction = EAGPCountdownActionOutput::Updated;
		Response.TriggerLink(ExecutionFunction, OutputLink, CallbackTarget);
	}
}

void FAGPCountdownLatentAction::CancelAction()
{
	bNeedToCancel = true;
}
