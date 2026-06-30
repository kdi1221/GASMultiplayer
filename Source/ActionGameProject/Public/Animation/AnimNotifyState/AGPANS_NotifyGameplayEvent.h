// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AGPANS_NotifyGameplayEvent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "NotifyState Gameplay Event"))
class ACTIONGAMEPROJECT_API UAGPANS_NotifyGameplayEvent : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	/* Begin 이벤트에서 알림을 보낼 GameplayEvent Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FGameplayTag BeginEventGameplayTag;

	/* End 이벤트에서 알림을 보낼 GameplayEvent Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FGameplayTag EndEventGameplayTag;

	/* 몽타주 에디터 상에 표시할 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FString NotifyStateName;
	
public:
	virtual FString GetNotifyName_Implementation() const override;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
