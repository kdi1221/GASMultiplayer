// KJY All Rights Reserved


#include "Common/AGPCommonFunctionLibrary.h"
#include "Common/LatentAction/AGPCountdownLatentAction.h"
#include "Subsystem/AGPGameDataTableSubsystem.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Team.h"
#include "Log/AGPLogChannels.h"

bool UAGPCommonFunctionLibrary::IntersectPlanes(const FPlane& P1, const FPlane& P2, const FPlane& P3, FVector& OutPoint)
{
	/* 각 평면의 법선벡터 */
	FVector PlaneNormal1(P1.X, P1.Y, P1.Z), PlaneNormal2(P2.X, P2.Y, P2.Z), PlaneNormal3(P3.X, P3.Y, P3.Z);

	/* 세 평면이 교차점을 가지고 있는지 판단 : 두 평면의 법선벡터와 수직인 벡터를 나머지 평면의 법선벡터과 방향 비교 */
	/* 교차점이 있다면, 나머지 법선벡터와 방향이 같거나 반대방향이어야 한다.(수직이면 => 아주 작은값, 교차점 없음) */
	const float denom = FVector::DotProduct(PlaneNormal1, FVector::CrossProduct(PlaneNormal2, PlaneNormal3));
	if (FMath::Abs(denom) < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	/* 각 평면의 법선벡터와 교차하는 선 위에 위치한 점들을 구한뒤 합하여 세 평면의 방향 삼중곱을 나눠 평균을 구한다. */
	OutPoint = ((P1.W * FVector::CrossProduct(PlaneNormal2, PlaneNormal3)) +
		(P2.W * FVector::CrossProduct(PlaneNormal3, PlaneNormal1)) +
		(P3.W * FVector::CrossProduct(PlaneNormal1, PlaneNormal2))) / denom;

	return true;
}

bool UAGPCommonFunctionLibrary::GetViewFrustumCorners(const FConvexVolume& ViewFrustum, TArray<FVector>& OutCorners)
{
	// 평면 인덱스 확인 (Near, Left, Right, Top, Bottom, Far)
	if (ViewFrustum.Planes.Num() != eViewFrustumPlanes::PlaneMax)
	{
		return false;
	}

	const FPlane& Near = ViewFrustum.Planes[eViewFrustumPlanes::Near];
	const FPlane& Left = ViewFrustum.Planes[eViewFrustumPlanes::Left];
	const FPlane& Right = ViewFrustum.Planes[eViewFrustumPlanes::Right];
	const FPlane& Top = ViewFrustum.Planes[eViewFrustumPlanes::Top];
	const FPlane& Bottom = ViewFrustum.Planes[eViewFrustumPlanes::Bottom];
	const FPlane& Far = ViewFrustum.Planes[eViewFrustumPlanes::Far];

	/* 모서리 꼭지점에 연결된 각 3개의 평면들을 확인해서 8개의 꼭지점을 가져온다. */
	OutCorners.SetNum(eViewFrustumCorners::CornerMax);
	const bool bResult =
		IntersectPlanes(Near, Left, Top, OutCorners[eViewFrustumCorners::NearTopLeft]) &&     // Near Top Left
		IntersectPlanes(Near, Right, Top, OutCorners[eViewFrustumCorners::NearTopRight]) &&    // Near Top Right
		IntersectPlanes(Near, Right, Bottom, OutCorners[eViewFrustumCorners::NearBottomRight]) && // Near Bottom Right
		IntersectPlanes(Near, Left, Bottom, OutCorners[eViewFrustumCorners::NearBottomLeft]) &&  // Near Bottom Left
		IntersectPlanes(Far, Left, Top, OutCorners[eViewFrustumCorners::FarTopLeft]) &&      // Far Top Left
		IntersectPlanes(Far, Right, Top, OutCorners[eViewFrustumCorners::FarTopRight]) &&     // Far Top Right
		IntersectPlanes(Far, Right, Bottom, OutCorners[eViewFrustumCorners::FarBottomRight]) &&  // Far Bottom Right
		IntersectPlanes(Far, Left, Bottom, OutCorners[eViewFrustumCorners::FarBottomLeft]);     // Far Bottom Left

	return bResult;
}

void UAGPCommonFunctionLibrary::DrawFrustumForDebug(UWorld* InWorld, const FConvexVolume& InFrustum, const FColor InColor, const bool bDrawPersistent, const float InDuration)
{
	TArray<FVector> FrustumCorners;
	if (!GetViewFrustumCorners(InFrustum, FrustumCorners))
	{
		return;
	}

	// Near Plane 사각형 그리기
	for (int i = 0; i < 4; ++i)
	{
		DrawDebugLine(InWorld, FrustumCorners[i], FrustumCorners[(i + 1) % 4], InColor, bDrawPersistent, InDuration, 0, 1.f);
	}

	// Far Plane 사각형 그리기
	for (int i = 4; i < 8; ++i)
	{
		DrawDebugLine(InWorld, FrustumCorners[i], FrustumCorners[4 + (i + 1) % 4], InColor, bDrawPersistent, InDuration, 0, 1.f);
	}

	// Near-Far Plane 연결선 그리기
	for (int i = 0; i < 4; ++i)
	{
		DrawDebugLine(InWorld, FrustumCorners[i], FrustumCorners[i + 4], InColor, bDrawPersistent, InDuration, 0, 1.f);
	}
}

void UAGPCommonFunctionLibrary::CountDown(const UObject* ContextObject,
										float TotalTime,
										float UpdateInterval,
										float& OutRemainingTime,
										EAGPCountdownActionInput ActionInput,
										UPARAM(DisplayName = "Output") EAGPCountdownActionOutput& ActionOutput,
										FLatentActionInfo LatentInfo)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(ContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	FAGPCountdownLatentAction* FoundAction = LatentActionManager.FindExistingAction<FAGPCountdownLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	switch(ActionInput)
	{
	case EAGPCountdownActionInput::Start:
		{
			if (!FoundAction)
			{
				LatentActionManager.AddNewAction(
					LatentInfo.CallbackTarget,
					LatentInfo.UUID,
					new FAGPCountdownLatentAction(
						TotalTime,
						UpdateInterval,
						OutRemainingTime,
						ActionOutput,
						LatentInfo)
				);
			}
		}
		break;

	case EAGPCountdownActionInput::Cancel:
		{
			if (FoundAction)
			{
				FoundAction->CancelAction();
			}
		}
		break;
	}
}

const FText& UAGPCommonFunctionLibrary::GetMonsterNameText(UObject* InContext, const int32 MonsterNameID)
{
	checkf(InContext, TEXT("[%s], Invalid InContext"), __FUNCTIONW__);

	UWorld* WorldContext = InContext->GetWorld();
	checkf(WorldContext, TEXT("[%s], Invalid WorldContext"), __FUNCTIONW__);

	UAGPGameDataTableSubsystem* GameDataTableSubsystem = UGameInstance::GetSubsystem<UAGPGameDataTableSubsystem>(WorldContext->GetGameInstance());
	checkf(GameDataTableSubsystem, TEXT("[%s], Invalid GameDataTableSubsystem"), __FUNCTIONW__);

	return GameDataTableSubsystem->GetMonsterNameText(MonsterNameID);
}

void UAGPCommonFunctionLibrary::ReportAITeamStimulusEvent(AActor* InBroadcaster, AActor* InTarget, const FVector& InLastKnowLocation, float EventRange, float PassedInfoAge, float InStrength)
{
	checkf(InBroadcaster, TEXT("[%s], Invalid InBroadcaster"), __FUNCTIONW__);

	UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(InBroadcaster);
	if (!PerceptionSystem)
	{
		return;
	}

	FAITeamStimulusEvent Event(InBroadcaster, InTarget, InLastKnowLocation, EventRange, PassedInfoAge, InStrength);
	PerceptionSystem->OnEvent(Event);
}
