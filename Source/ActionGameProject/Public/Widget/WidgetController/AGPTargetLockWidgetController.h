// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widget/WidgetController/AGPWidgetController.h"
#include "AGPTargetLockWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChangedSignature, AActor*, TargetActor);

/**
 * 타겟잠금 위젯에 대한 컨트롤러
 */

UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPTargetLockWidgetController : public UAGPWidgetController
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnTargetChangedSignature OnTargetChanged;
	
private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentTargetActor;

public:
	void SetTargetActor(AActor* InTargetActor);

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE AActor* GetTargetActor() const {return CurrentTargetActor;}
};
