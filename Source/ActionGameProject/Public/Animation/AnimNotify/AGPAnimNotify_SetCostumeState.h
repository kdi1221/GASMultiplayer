// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "Costume/AGPCostumeEnum.h"
#include "AGPAnimNotify_SetCostumeState.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Set Costume Actor State"))
class ACTIONGAMEPROJECT_API UAGPAnimNotify_SetCostumeState : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	/* 대상 Costume의 Type */
	UPROPERTY(EditAnywhere)
	EAGPCostumeType CostumeType = EAGPCostumeType::None;

	/* 대상 CostumeType이 FromEquipment일때 사용되는 대상 Costume Instnace 구분 변수(장비한 슬롯에 대한 Tag) */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "CostumeType == EAGPCostumeType::FromEquipment", EditConditionHides))
	FGameplayTag CostumeEquipmentSlotTag;

	/* 대상이되는 Costume Actor의 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName CostumeActorName;

	/* 변경할 Costume Actor의 상태 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName CostumeActorStatus;
	
public:
	virtual FString GetNotifyName_Implementation() const override;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
