// KJY All Rights Reserved


#include "Animation/AnimNotify/AGPAN_NotifyGameplayEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

FString UAGPAN_NotifyGameplayEvent::GetNotifyName_Implementation() const
{
	return NotifyStateName;
}

void UAGPAN_NotifyGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* MeshOwner = MeshComp->GetOwner();
	if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshOwner))
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshOwner, EventGameplayTag, FGameplayEventData());
	}
}


