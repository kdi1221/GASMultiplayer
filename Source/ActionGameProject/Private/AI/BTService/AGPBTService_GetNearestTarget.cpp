// KJY All Rights Reserved


#include "AI/BTService/AGPBTService_GetNearestTarget.h"
#include "AI/AGP_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "Log/AGPLogChannels.h"

UAGPBTService_GetNearestTarget::UAGPBTService_GetNearestTarget()
{
	NodeName = TEXT("Get Nearest Target");

	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	Interval = 0.3f;
	RandomDeviation = 0.f;

	/* TargetActor Blackboard Key 설정 - Actor만 지정가능 */
	InSetTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InSetTargetActorKey), AActor::StaticClass());
}

void UAGPBTService_GetNearestTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	/* TargetActor Blackboard Key 설정 */
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InSetTargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UAGPBTService_GetNearestTarget::GetStaticDescription() const
{
	/* 현재 노드의 상태에 대한 String 반환 */
	const FString KeyDescription = InSetTargetActorKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("Get Nearest Target %s Key %s"), *KeyDescription, *GetStaticServiceDescription());
}

void UAGPBTService_GetNearestTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAGP_AIController* OwnerAIController = Cast<AAGP_AIController>(OwnerComp.GetAIOwner());
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);

	APawn* ControlPawn = OwnerAIController->GetPawn();
	if (!ControlPawn)
	{
		return;
	}

	const FVector ControlPawnLocaiton = ControlPawn->GetActorLocation();

	AActor* BestNearestActor = nullptr;
	float BestNearestDist = TNumericLimits<float>::Max();
	OwnerAIController->ForEachHostileActors
	(
		[&](AActor* SensedActor) 
		{
			/* 사망한 캐릭터는 확인하지 않음 */
			if (UAGPAbilitySystemFunctionLibrary::IsTargetDeath(SensedActor))
			{
				return;
			}

			/* 각 Actor들과의 거리를 확인해서 가장 가까운 대상을 찾음*/
			const FVector ActorLocation = SensedActor->GetActorLocation();
			const float ToTargetDist = FVector::DistSquared(ControlPawnLocaiton, ActorLocation);

			if (ToTargetDist < BestNearestDist)
			{
				BestNearestActor = SensedActor;
				BestNearestDist = ToTargetDist;
			}
		}
	);

	if (BestNearestActor)
	{
		UBlackboardComponent* OwnerBlackboardComponent = OwnerComp.GetBlackboardComponent();
		checkf(OwnerBlackboardComponent, TEXT("[%s], Invalid OwnerBlackboardComponent"), __FUNCTIONW__);

		OwnerBlackboardComponent->SetValueAsObject(InSetTargetActorKey.SelectedKeyName, BestNearestActor);
	}
}
