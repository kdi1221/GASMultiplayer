// KJY All Rights Reserved


#include "Characters/Components/AGPStateTagNotifierComponent.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"

UAGPStateTagNotifierComponent::UAGPStateTagNotifierComponent()
{
	bWantsInitializeComponent = false;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(false);
}

void UAGPStateTagNotifierComponent::InitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC)
{
	checkf(InOwnerASC, TEXT("[%s] - Invalid OwnerASC"), __FUNCTIONW__);

	/* 기존에 있던 Tag정보들을 제거 */
	const TArray<FGameplayTag>& OldOwnedTags = OwnedStateTags.GetGameplayTagArray();
	for (const FGameplayTag& InOldTag : OldOwnedTags)
	{
		OnRemovedStateTag.Broadcast(InOldTag);
	}
	OwnedStateTags.Reset();

	/* 현재 Owner에 추가되어있는 Tag들에 대한 이벤트 알림 */
	const FGameplayTagContainer& OwnerExplicitTags = InOwnerASC->GetOwnedGameplayTags();
	for (const FGameplayTag& RegisterTag : RegisterCallbackStateTags)
	{
		/* OwnerASC에 있던 Tag들을 확인해서 추가 */
		if (OwnerExplicitTags.HasTagExact(RegisterTag))
		{
			OwnedStateTags.AddTag(RegisterTag);
			OnAddedStateTag.Broadcast(RegisterTag);
		}

		/* GameplayTag들의 적용에 대한 이벤트 콜백 설정 */
		InOwnerASC->RegisterGameplayTagEvent(RegisterTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAGPStateTagNotifierComponent::OnAddOrRemovedStateTags);
	}
}

void UAGPStateTagNotifierComponent::UninitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC)
{
	checkf(InOwnerASC, TEXT("[%s] - Invalid OwnerASC"), __FUNCTIONW__);

	for (const FGameplayTag& RegisterTag : RegisterCallbackStateTags)
	{
		InOwnerASC->RegisterGameplayTagEvent(RegisterTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}
}

void UAGPStateTagNotifierComponent::OnAddOrRemovedStateTags(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		/* 새로운 Tag 추가 */
		OwnedStateTags.AddTag(Tag);
		OnAddedStateTag.Broadcast(Tag);
	}
	else
	{
		/* 기존에 있던 Tag 삭제 */
		OwnedStateTags.RemoveTag(Tag);
		OnRemovedStateTag.Broadcast(Tag);
	}
}
