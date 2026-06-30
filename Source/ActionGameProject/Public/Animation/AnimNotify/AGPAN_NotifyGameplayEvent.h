// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AGPAN_NotifyGameplayEvent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Notify Gameplay Event"))
class ACTIONGAMEPROJECT_API UAGPAN_NotifyGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	/* 알림을 보낼 GameplayEvent Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FGameplayTag EventGameplayTag;

	/* 몽타주 에디터 상에 표시할 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FString NotifyStateName;

public:
	virtual FString GetNotifyName_Implementation() const override;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);
};
