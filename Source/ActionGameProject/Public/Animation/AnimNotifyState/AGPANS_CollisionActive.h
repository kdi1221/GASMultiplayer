// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Costume/AGPCostumeEnum.h"
#include "Collision/AGPCollisionEnum.h"
#include "Collision/AGPCollisionStruct.h"
#include "GameplayTagContainer.h"
#include "AGPANS_CollisionActive.generated.h"

class IAGPCollisionHandleInterface;

USTRUCT(BlueprintType)
struct FAGPANSCollisionActive_CostumeActor
{
	GENERATED_BODY()
		
public:
	/* 대상 Costume의 Type */
	UPROPERTY(EditAnywhere)
	EAGPCostumeType CostumeType = EAGPCostumeType::None;

	/* 대상 CostumeType이 FromEquipment일때 사용되는 대상 Costume Instnace 구분 변수(장비한 슬롯에 대한 Tag) */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "CostumeType == EAGPCostumeType::FromEquipment", EditConditionHides))
	FGameplayTag CostumeEquipmentSlotTag;

	/* 대상이 되는 Costume Actor의 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName CostumeActorName;
};

/**
 * 
 */
UCLASS(meta = (DisplayName = "Collision Active"))
class ACTIONGAMEPROJECT_API UAGPANS_CollisionActive : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/* 대상 Collision Type */
	UPROPERTY(EditAnywhere)
	EAGPCollisionType CollisionType = EAGPCollisionType::None;

	/* Collision Type이 CostumeActor일때 사용할 Collision 식별자 관련 변수 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "CostumeActor", EditCondition = "CollisionType == EAGPCollisionType::CostumeActor", EditConditionHides))
	FAGPANSCollisionActive_CostumeActor CollisionProperty_CostumeActor;

	/* 대상 Collision 이름 */
	UPROPERTY(EditAnywhere)
	FName CollisionName;
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	virtual FString GetNotifyName_Implementation() const override;

private:
	void RequestCollisionActive(AActor* InOwnerActor,  bool IsActive) const;

	FAGPCollisionIdentifierHandle CreateCollisionIdentifierHandle_CostumeActor() const;
	FAGPCollisionIdentifierHandle CreateCollisionIdentifierHandle_AttachCharacter() const;
};
