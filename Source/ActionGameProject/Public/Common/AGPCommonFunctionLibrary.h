// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AGPCommonEnums.h"
#include "AGPCommonFunctionLibrary.generated.h"

enum eViewFrustumPlanes
{
	Near = 0,
	Left,
	Right,
	Top,
	Bottom,
	Far,
	PlaneMax
};

enum eViewFrustumCorners
{
	NearTopLeft = 0,
	NearTopRight,
	NearBottomRight,
	NearBottomLeft,
	FarTopLeft,
	FarTopRight,
	FarBottomRight,
	FarBottomLeft,
	CornerMax
};

/**
 *  프로젝트 공용 함수 모음
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPCommonFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/* TMap의 Key중 랜덤하게 하나를 선택해서 반환 */
	template<typename KeyType, typename ValueType>
	static KeyType GetRandomSelectMapKey(const TMap<KeyType, ValueType>& InMap);

	/* TSet의 값들 중 하나를 랜덤하게 선택해서 반환 */
	template<typename ElementType>
	static ElementType GetRandomSelectSet(const TSet<ElementType>& InSet);

	/* 3개의 평면이 교차하는 특정 교차점 위치 반환 */
	static bool IntersectPlanes(const FPlane& P1, const FPlane& P2, const FPlane& P3, FVector& OutPoint);

	/* 뷰 프러스텀 모서리 구하기(ViewFrustum은 eViewFrustumPlanes순서 지켜야함, OutCorners는 eViewFrustumCorners순서로 반환) */
	static bool GetViewFrustumCorners(const FConvexVolume& ViewFrustum, TArray<FVector>& OutCorners);

	/* 디버깅용 프러스텀 그리기 */
	static void DrawFrustumForDebug(UWorld* InWorld, const FConvexVolume& InFrustum, const FColor InColor = FColor::Red, const bool bDrawPersistent = false, const float InDuration = 2.f);

	/* TArray Shuffle */
	template<typename T>
	static void ShuffleArray(TArray<T>& InArray);

public:
	/* Countdown LatentAction */
	UFUNCTION(BlueprintCallable, Category = "AGP|Common|FunctionLibrary", meta = (Latent, WorldContext = "ContextObject", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "ActionInput|ActionOutput", TotalTime = "1.0", UpdateInterval = "0.1"))
	static void CountDown(const UObject* ContextObject,
							float TotalTime,
							float UpdateInterval,
							float& OutRemainingTime,
							EAGPCountdownActionInput ActionInput,
							UPARAM(DisplayName = "Output") EAGPCountdownActionOutput& ActionOutput,
							FLatentActionInfo LatentInfo);

public:
	/* 몬스터 이름 반환 */
	UFUNCTION(BlueprintPure, Category = "AGP|Monster", Meta = (DefaultToSelf = "InContext"))
	static const FText& GetMonsterNameText(UObject* InContext, const int32 MonsterNameID);

public:
	/* Report AI Team Stimulus Event*/
	UFUNCTION(BlueprintCallable, Category = "AGP|AI|FunctionLibrary")
	static void ReportAITeamStimulusEvent(AActor* InBroadcaster, AActor* InTarget, const FVector& InLastKnowLocation, float EventRange, float PassedInfoAge = 0.f, float InStrength = 1.f);
};


template<typename KeyType, typename ValueType>
static KeyType UAGPCommonFunctionLibrary::GetRandomSelectMapKey(const TMap<KeyType, ValueType>& InMap)
{
	/* 빈 키 값, 예외상황에서 반환 */
	static const KeyType EmptyKey;

	if (InMap.IsEmpty())
	{
		return EmptyKey;
	}

	TArray<KeyType> Keys;
	InMap.GetKeys(Keys);

	const int32 RandomIndex = FMath::RandRange(0, Keys.Num() - 1);
	return Keys.IsValidIndex(RandomIndex) ? Keys[RandomIndex] : EmptyKey;
}

/* TSet의 값들 중 하나를 랜덤하게 선택해서 반환 */
template<typename ElementType>
static ElementType UAGPCommonFunctionLibrary::GetRandomSelectSet(const TSet<ElementType>& InSet)
{
	/* 빈 키 값, 예외상황에서 반환 */
	static const ElementType EmptyValue;

	if (InSet.IsEmpty())
	{
		return EmptyValue;
	}

	TArray<ElementType> ElementArray(InSet.Array());

	const int32 RandomIndex = FMath::RandRange(0, ElementArray.Num() - 1);
	return ElementArray.IsValidIndex(RandomIndex) ? ElementArray[RandomIndex] : EmptyValue;
}

/* TArray Shuffle */
template<typename T>
static void UAGPCommonFunctionLibrary::ShuffleArray(TArray<T>& InArray)
{
	const int32 ArrayNum = InArray.Num();
	if (ArrayNum <= 1)
	{
		return;
	}

	const int32 LastIndex = ArrayNum - 1;

	/* 랜덤 시드 결정(CPU 타이머) */
	const uint32 UseSeed = static_cast<uint32>(FPlatformTime::Cycles64());
	FRandomStream Stream(UseSeed);

	for (int32 i = 0; i < ArrayNum; ++i)
	{
		const int32 RandomIndex = Stream.RandRange(i, LastIndex);
		if (i != RandomIndex)
		{
			InArray.Swap(i, RandomIndex);
		}
	}
}