// KJY All Rights Reserved


#include "AI/BTService/AGPBTService_TargetLockMove.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Characters/AGP_CharacterBase.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "Log/AGPLogChannels.h"

UAGPBTService_TargetLockMove::UAGPBTService_TargetLockMove()
{
	NodeName = TEXT("Target Lock Move");

	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	bCreateNodeInstance = true;
	bTickIntervals = true;
	Interval = 0.f;
	RandomDeviation = 0.f;

	/* TargetActor Blackboard Key 설정 - Actor만 지정가능 */
	InTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetActorKey), AActor::StaticClass());
}

void UAGPBTService_TargetLockMove::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	/* TargetActor Blackboard Key 설정 */
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UAGPBTService_TargetLockMove::GetStaticDescription() const
{
	/* 현재 노드의 상태에 대한 String 반환 */
	const FString KeyDescription = InTargetActorKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("Target Lock to %s Key %s"), *KeyDescription, *GetStaticServiceDescription());
}

void UAGPBTService_TargetLockMove::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* OwnerAIController = OwnerComp.GetAIOwner();
	checkf(OwnerAIController, TEXT("[%s], Invalid OwnerAIController"), __FUNCTIONW__);
	OwningCharacter = Cast<AAGP_CharacterBase>(OwnerAIController->GetPawn());

	//AGP_NET_LOG(this, LogAGPAI, Log, TEXT("OwningCharacter[%s]"), *GetNameSafe(OwningCharacter));

	UpdateTargetReferenceLocation(OwnerComp);
}

void UAGPBTService_TargetLockMove::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//AGP_NET_LOG(this, LogAGPAI, Log, TEXT("OwningCharacter[%s]"), *GetNameSafe(OwningCharacter));

	UpdateTargetReferenceLocation(OwnerComp);
}

void UAGPBTService_TargetLockMove::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//AGP_NET_LOG(this, LogAGPAI, Log, TEXT("OwningCharacter[%s]"), *GetNameSafe(OwningCharacter));

	/* AI Character 타겟 이동 종료 */
	StopOwningCharacterTargetLockMove();
}

void UAGPBTService_TargetLockMove::UpdateTargetReferenceLocation(UBehaviorTreeComponent& OwnerComp)
{
	//Owner가 유효하지 않거나 사망했을 경우 Stop TargetLock Move
	if (!OwningCharacter || UAGPAbilitySystemFunctionLibrary::IsTargetDeath(OwningCharacter))
	{
		StopOwningCharacterTargetLockMove();
		return;
	}

	/* 블랙보드 -> 타겟 정보 갱신 */
	UBlackboardComponent* OwnerBlackboardComponent = OwnerComp.GetBlackboardComponent();
	checkf(OwnerBlackboardComponent, TEXT("[%s], Invalid OwnerBlackboardComponent"), __FUNCTIONW__);

	AActor* TargetActor = Cast<AActor>(OwnerBlackboardComponent->GetValueAsObject(InTargetActorKey.SelectedKeyName));

	/* 타겟이 유효하지 않거나 사망했을 경우 Stop TargetLock Move */
	if (!TargetActor || UAGPAbilitySystemFunctionLibrary::IsTargetDeath(TargetActor))
	{
		StopOwningCharacterTargetLockMove();
		return;
	}

	/* AI Character 타겟 이동 설정 */
	OwningCharacter->StartTargetLockMove();
	OwningCharacter->UpdateTargetLockPostion(TargetActor->GetActorLocation());
}

void UAGPBTService_TargetLockMove::StopOwningCharacterTargetLockMove()
{
	if (OwningCharacter)
	{
		OwningCharacter->StopTargetLockMove();
	}
}
