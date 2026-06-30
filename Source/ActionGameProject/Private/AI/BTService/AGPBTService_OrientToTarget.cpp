// KJY All Rights Reserved


#include "AI/BTService/AGPBTService_OrientToTarget.h"
#include "Kismet/KismetMathLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UAGPBTService_OrientToTarget::UAGPBTService_OrientToTarget()
{
	NodeName = TEXT("Orient Rotation To Target");

	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	RotationInterpSpeed = 5.f;
	Interval = 0.f;
	RandomDeviation = 0.f;

	/* TargetActor Blackboard Key 설정 - Actor만 지정가능 */
	InTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetActorKey), AActor::StaticClass());
}

void UAGPBTService_OrientToTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	/* TargetActor Blackboard Key 설정 */
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UAGPBTService_OrientToTarget::GetStaticDescription() const
{
	/* 현재 노드의 상태에 대한 String 반환 */
	const FString KeyDescription = InTargetActorKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("Orient rotation to %s Key %s"), *KeyDescription, *GetStaticServiceDescription());
}

void UAGPBTService_OrientToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* OwnerBlackboardComponent = OwnerComp.GetBlackboardComponent();
	checkf(OwnerBlackboardComponent, TEXT("[%s], Invalid OwnerBlackboardComponent"), __FUNCTIONW__);

	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);

	AActor* TargetActor = Cast<AActor>(OwnerBlackboardComponent->GetValueAsObject(InTargetActorKey.SelectedKeyName));
	APawn* OwningPawn = OwnerAIController->GetPawn();

	/* Target을 향해 AI 컨트롤중인 Pawn이 회전하도록 계산 */
	if (OwningPawn && TargetActor)
	{
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(OwningPawn->GetActorLocation(), TargetActor->GetActorLocation());
		const FRotator TargetRot = FMath::RInterpTo(OwningPawn->GetActorRotation(), LookAtRot, DeltaSeconds, RotationInterpSpeed);

		OwningPawn->SetActorRotation(TargetRot);
	}
}
