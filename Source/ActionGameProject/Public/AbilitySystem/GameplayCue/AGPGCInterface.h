// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "AGPGCInterface.generated.h"

UINTERFACE(MinimalAPI)
class UAGPGCInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * GameplayCue와 관련한 인터페이스 모음
 */
class ACTIONGAMEPROJECT_API IAGPGCInterface
{
	GENERATED_BODY()

public:
	/* Hit Event에 대한 GameplayCue 태그 반환 */
	virtual const FGameplayTag& GetGCHitReactTag() const = 0;

	/* Hit Event 알림 수신 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnHitEvent();
};
