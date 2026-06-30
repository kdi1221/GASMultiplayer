// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AGPBTTask_RotateToFaceTarget.generated.h"

/* RotateToFaceTarget 노드의 메모리에서 사용할 구조체 */
struct FRotateToFaceTargetTaskMemory
{
	TWeakObjectPtr<APawn> OwningPawn;
	TWeakObjectPtr<AActor> TargetActor;

	bool IsValid() const
	{
		return OwningPawn.IsValid() && TargetActor.IsValid();
	}

	void Reset()
	{
		OwningPawn.Reset();
		TargetActor.Reset();
	}

};


/**
 * AI가 대상을 바로보도록 회전
 * AnglePrecision이하가 될때까지 RotationInterpSpeed로 회전후 성공 반환
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTTask_RotateToFaceTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
private:
	/* 대상과의 허용 각도 */
	UPROPERTY(EditAnywhere, Category = "Face Target")
	float AnglePrecision;

	/* 회전 속도 */
	UPROPERTY(EditAnywhere, Category = "Face Target")
	float RotationInterpSpeed;

	/* 바라볼 대상 블랙보드 키 */
	UPROPERTY(EditAnywhere, Category = "Face Target")
	FBlackboardKeySelector InTargetToFaceKey;

public:
	UAGPBTTask_RotateToFaceTarget();

#pragma region[UBTNode Interface]
private:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
#pragma endregion

#pragma region[UBTTaskNode Interface]
private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
#pragma endregion

private:
	bool HasReachedAnglePercision(APawn* QueryPawn, AActor* TargetActor) const;
};
