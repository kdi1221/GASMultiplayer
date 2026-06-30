// KJY All Rights Reserved


#include "AI/BTTask/AGPBTTask_RotateToFaceTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

UAGPBTTask_RotateToFaceTarget::UAGPBTTask_RotateToFaceTarget()
{
	NodeName = TEXT("Rotate to Face Target");
	AnglePrecision = 10.f;
	RotationInterpSpeed = 5.f;

	/* Tick 수신 */
	bNotifyTick = true;

	/* Task 종료 수신받음 */
	bNotifyTaskFinished = true;

	/* Node Instance 생성하지 않음(Node Memory 활용) */
	bCreateNodeInstance = false;

	INIT_TASK_NODE_NOTIFY_FLAGS();

	InTargetToFaceKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAGPBTTask_RotateToFaceTarget, InTargetToFaceKey), AActor::StaticClass());
}

void UAGPBTTask_RotateToFaceTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetToFaceKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UAGPBTTask_RotateToFaceTarget::GetInstanceMemorySize() const
{
	return sizeof(FRotateToFaceTargetTaskMemory);
}

FString UAGPBTTask_RotateToFaceTarget::GetStaticDescription() const
{
	const FString KeyDescription = InTargetToFaceKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("Smoothly rotates to face %s Key until the angle precision %s is reached"), *KeyDescription, *FString::SanitizeFloat(AnglePrecision));
}

EBTNodeResult::Type UAGPBTTask_RotateToFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* OwnerBlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!OwnerBlackboardComponent)
	{
		return EBTNodeResult::Failed;
	}

	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	if (!OwnerAIController)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(OwnerBlackboardComponent->GetValueAsObject(InTargetToFaceKey.SelectedKeyName));
	APawn* OwningPawn = OwnerAIController->GetPawn();

	/* 노드 내 메모리(FRotateToFaceTargetTaskMemory)에 타겟과 Owning Pawn 저장 */
	FRotateToFaceTargetTaskMemory* TaskMemory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);
	checkf(TaskMemory, TEXT("[%s], Invalid TaskMemory"), __FUNCTIONW__);

	TaskMemory->OwningPawn = OwningPawn;
	TaskMemory->TargetActor = TargetActor;

	if (!TaskMemory->IsValid())
	{
		return EBTNodeResult::Failed;
	}

	/* 이미 도달했으면 곧바로 성공 반환 */
	if (HasReachedAnglePercision(OwningPawn, TargetActor))
	{
		TaskMemory->Reset();
		return EBTNodeResult::Succeeded;
	}

	/* Tick에서 회전하므로 진행중 반환 */
	return EBTNodeResult::InProgress;
}

void UAGPBTTask_RotateToFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FRotateToFaceTargetTaskMemory* TaskMemory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);
	checkf(TaskMemory, TEXT("[%s], Invalid TaskMemory"), __FUNCTIONW__);

	/* 도중에 Target, Owner Pawn Invalid 상황이면 실패 반환 */
	if (!TaskMemory->IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}

	APawn* OwningPawn = TaskMemory->OwningPawn.Get();
	AActor* TargetActor = TaskMemory->TargetActor.Get();

	if (HasReachedAnglePercision(OwningPawn, TargetActor))
	{
		/* 목표 회전값에 도달하면 성공 반환 */
		TaskMemory->Reset();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		/* 목표를 바라보도록 회전 */
		/* TODO : 나중에 Locomotion 애니메이션등을 하게되면 턴인플레이스등으로 조정? */
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(OwningPawn->GetActorLocation(), TargetActor->GetActorLocation());
		const FRotator TargetRot = FMath::RInterpTo(OwningPawn->GetActorRotation(), LookAtRot, DeltaSeconds, RotationInterpSpeed);

		OwningPawn->SetActorRotation(TargetRot);
	}
}

bool UAGPBTTask_RotateToFaceTarget::HasReachedAnglePercision(APawn* QueryPawn, AActor* TargetActor) const
{
	if (!QueryPawn || !TargetActor)
	{
		return false;
	}

	/* Pawn이 바라보는 방향과 Target과의 방향간의 각을 구해서 AnglePrecision와 비교 */
	const FVector OwnerForward = QueryPawn->GetActorForwardVector();
	const FVector OwnerToTargetNormalized = (TargetActor->GetActorLocation() - QueryPawn->GetActorLocation()).GetSafeNormal();

	const float DotResult = FVector::DotProduct(OwnerForward, OwnerToTargetNormalized);
	const float AngleDiff = UKismetMathLibrary::DegAcos(DotResult);

	return AngleDiff <= AnglePrecision;
}
