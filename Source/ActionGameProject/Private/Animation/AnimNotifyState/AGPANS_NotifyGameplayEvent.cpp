// KJY All Rights Reserved


#include "Animation/AnimNotifyState/AGPANS_NotifyGameplayEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


FString UAGPANS_NotifyGameplayEvent::GetNotifyName_Implementation() const
{
	return NotifyStateName;
}

void UAGPANS_NotifyGameplayEvent::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* MeshOwner = MeshComp->GetOwner();
	if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshOwner))
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshOwner, BeginEventGameplayTag, FGameplayEventData());
	}
}

void UAGPANS_NotifyGameplayEvent::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* MeshOwner = MeshComp->GetOwner();
	if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshOwner))
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshOwner, EndEventGameplayTag, FGameplayEventData());
	}
}