// KJY All Rights Reserved


#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "AGPGameplayTags.h"
#include "AbilitySystem/Attribute/AGPAttributeSetBase.h"
#include "Log/AGPLogChannels.h"

#pragma region[FAGPDynamicAttributeSetElement]

void FAGPDynamicAttributeSetElement::SetAttributeSetInstance(UAGPAttributeSetBase* InAddAttributeSet)
{
	AttributeSet = InAddAttributeSet;
}

bool FAGPDynamicAttributeSetElement::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << AttributeSet;

	bOutSuccess = true;

	return true;
}

bool FAGPDynamicAttributeSetElement::Identical(const FAGPDynamicAttributeSetElement* Other, uint32 PortFlags) const
{
	if (!Other)
	{
		return false;
	}

	return AttributeSet == Other->AttributeSet;
}

#pragma endregion

#pragma region[FAGPDynamicAttributeSetElementContainer]

void FAGPDynamicAttributeSetElementContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerComponent.Get())
	{
		OwnerASC->OnPreReplicatedRemove(RemovedIndices, FinalSize);
	}
}

void FAGPDynamicAttributeSetElementContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerComponent.Get())
	{
		OwnerASC->OnPostReplicatedAdd(AddedIndices, FinalSize);
	}
}

void FAGPDynamicAttributeSetElementContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerComponent.Get())
	{
		OwnerASC->OnPostReplicatedChange(ChangedIndices, FinalSize);
	}
}

bool FAGPDynamicAttributeSetElementContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FAGPDynamicAttributeSetElement, FAGPDynamicAttributeSetElementContainer>(AttributeSetElements, DeltaParms, *this);
}

void FAGPDynamicAttributeSetElementContainer::SetOwnerComponent(UAGP_AbilitySystemComponent* InOwnerASC)
{
	CachedOwnerComponent = InOwnerASC;
}

#pragma endregion


void UAGP_AbilitySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	AddDynamicAttributeSetContainer.SetOwnerComponent(this);
}

void UAGP_AbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AddDynamicAttributeSetContainer);
}

void UAGP_AbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	//TODO : Dynamic Tag에 InputTag가 존재하는경우 별도로 저장해둬서 이후 TryActivateAbility에 사용한다.
	/*for (const FGameplayTag& DynamicAbilityTag : AbilitySpec.DynamicAbilityTags)
	{
		if (DynamicAbilityTag.MatchesTag(AGPGameplayTags::Input))
		{
			AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("Ability DynamicTag Input : %s"), *DynamicAbilityTag.ToString());
		}
	}*/

	OnAbilityGranted.Broadcast(AbilitySpec);
}

void UAGP_AbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);

	OnAbilityRemoved.Broadcast(AbilitySpec);
}




void UAGP_AbilitySystemComponent::OnAbilityInput_Started(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (!InInputTag.MatchesTag(AGPGameplayTags::Input))
	{
		return;
	}

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("InInputTag : %s"), *InInputTag.ToString());

	FAGPForEachAbility NotifyInputPressedToAbility;
	NotifyInputPressedToAbility.BindLambda
	(
		[&](FGameplayAbilitySpec& AbilitySpec)
		{
			//활성화 가능한 Ability중 InputTag에 맞는 Ability를 확인함
			if (AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
			{
				//Ability Input Pressed 알림
				NotifyInputPressedToSpec(AbilitySpec, InputActionValue, InInputTag);
			}
		}
	);

	ForEachAbility_Execute(NotifyInputPressedToAbility);
}

void UAGP_AbilitySystemComponent::OnAbilityInput_Triggered(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (!InInputTag.MatchesTag(AGPGameplayTags::Input))
	{
		return;
	}

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("InInputTag : %s"), *InInputTag.ToString());

	FAGPForEachAbility NotifyInputPressedToAbility;
	NotifyInputPressedToAbility.BindLambda
	(
		[&](FGameplayAbilitySpec& AbilitySpec)
		{
			//활성화 가능한 Ability중 InputTag에 맞는 Ability를 확인함
			if (AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
			{
				//Ability Input Triggered 알림
				NotifyInputTriggeredToSpec(AbilitySpec, InputActionValue, InInputTag);
			}
		}
	);

	ForEachAbility_Execute(NotifyInputPressedToAbility);
}

void UAGP_AbilitySystemComponent::OnAbilityInput_Completed(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (!InInputTag.MatchesTag(AGPGameplayTags::Input))
	{
		return;
	}

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("InInputTag : %s"), *InInputTag.ToString());

	FAGPForEachAbility NotifyInputReleaseToAbility;
	NotifyInputReleaseToAbility.BindLambda
	(
		[&](FGameplayAbilitySpec& AbilitySpec)
		{
			//활성화 가능한 Ability중 InputTag에 맞는 Ability를 확인함
			if (AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
			{
				//Ability Input Released 알림
				NotifyInputReleasedToSpec(AbilitySpec, InputActionValue, InInputTag);
			}
		}
	);

	ForEachAbility_Execute(NotifyInputReleaseToAbility);
}

void UAGP_AbilitySystemComponent::OnAbilityInput_Ongoning(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (!InInputTag.MatchesTag(AGPGameplayTags::Input))
	{
		return;
	}

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("InInputTag : %s"), *InInputTag.ToString());

	FAGPForEachAbility NotifyInputReleaseToAbility;
	NotifyInputReleaseToAbility.BindLambda
	(
		[&](FGameplayAbilitySpec& AbilitySpec)
		{
			//활성화 가능한 Ability중 InputTag에 맞는 Ability를 확인함
			if (AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
			{
				//Ability Input Ongoing 알림
				NotifyInputOngoningToSpec(AbilitySpec, InputActionValue, InInputTag);
			}
		}
	);

	ForEachAbility_Execute(NotifyInputReleaseToAbility);
}

void UAGP_AbilitySystemComponent::OnAbilityInput_Canceled(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (!InInputTag.MatchesTag(AGPGameplayTags::Input))
	{
		return;
	}

	//AGP_NET_LOG(this, LogAGPGamePlayAbility, Log, TEXT("InInputTag : %s"), *InInputTag.ToString());

	FAGPForEachAbility NotifyInputReleaseToAbility;
	NotifyInputReleaseToAbility.BindLambda
	(
		[&](FGameplayAbilitySpec& AbilitySpec)
		{
			//활성화 가능한 Ability중 InputTag에 맞는 Ability를 확인함
			if (AbilitySpec.DynamicAbilityTags.HasTagExact(InInputTag))
			{
				//Ability Input Canceled 알림
				NotifyInputCanceledToSpec(AbilitySpec, InputActionValue, InInputTag);
			}
		}
	);

	ForEachAbility_Execute(NotifyInputReleaseToAbility);
}

void UAGP_AbilitySystemComponent::ForEachAttributeSet_Execute(const FAGPForEachAttributeSet& InDelegate)
{
	for (const FAGPDynamicAttributeSetElement& AttributeSetElement : AddDynamicAttributeSetContainer.AttributeSetElements)
	{
		if (!InDelegate.ExecuteIfBound(AttributeSetElement.AttributeSet))
		{
			AGP_NET_LOG(this, LogAbilitySystemComponent, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

void UAGP_AbilitySystemComponent::ForEachAbility_Execute(const FAGPForEachAbility& InDelegate)
{
	//Ability의 정보조회를 반복할때 Ability의 상태가 변경될 수 있으므로 반복하기 전 잠금
	FScopedAbilityListLock ActiveScopeLock(*this);

	TArray<FGameplayAbilitySpec>& ActivatableAbilityList = GetActivatableAbilities();
	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilityList)
	{
		if (!InDelegate.ExecuteIfBound(AbilitySpec))
		{
			AGP_NET_LOG(this, LogAbilitySystemComponent, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

void UAGP_AbilitySystemComponent::GrantLocalBaseFeatureAbility()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	/* LocalBaseFeatureAbility 부여 */
	for (TSubclassOf<UAGPGameplayAbility> GrantBaseFeatureAbilityClass : LocalBaseFeatureAbilityClasses)
	{
		if (GrantBaseFeatureAbilityClass)
		{
			GiveAbility(FGameplayAbilitySpec(GrantBaseFeatureAbilityClass));
		}
	}
}

void UAGP_AbilitySystemComponent::GrantBaseAbility()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	/* Base Ability 부여 */
	for (TSubclassOf<UAGPGameplayAbility> GrantBaseAbilityClass : GrantBaseAbilityClasses)
	{
		if (GrantBaseAbilityClass)
		{
			GiveAbility(FGameplayAbilitySpec(GrantBaseAbilityClass));
		}
	}

	/* Activate Ability 부여 */
	for (const FAGPGrantAbilitySet& AbilitySet : GrantActivateAbilityClasses)
	{
		if (!AbilitySet.AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityClass, AbilitySet.GrantAbilityLevel);
		AbilitySpec.GetDynamicSpecSourceTags().AppendTags(AbilitySet.GrantAbilityToDynamicTag);
		
		GiveAbility(AbilitySpec);
	}
}

void UAGP_AbilitySystemComponent::AddDynamicAttributeSet(TSubclassOf<UAGPAttributeSetBase> InAddAttributeSetClass)
{
	checkf(IsOwnerActorAuthoritative(), TEXT("[%s] - Only Call Authority"), __FUNCTIONW__);
	checkf(InAddAttributeSetClass, TEXT("[%s], Invalid InAddAttributeSetClass"), __FUNCTIONW__);
	
	/* 추가할 AttributeSet 생성 */
	UAGPAttributeSetBase* NewAttributeSet = NewObject<UAGPAttributeSetBase>(GetOwner(), InAddAttributeSetClass);
	checkf(NewAttributeSet, TEXT("[%s], Invalid NewAttributeSet"), __FUNCTIONW__);

	/* UAbilitySystemComponent::SpawnedAttributes에 추가 */
	AddAttributeSetSubobject(NewAttributeSet);

	FAGPDynamicAttributeSetElement& AddedAttributeSetElement = AddDynamicAttributeSetContainer.AttributeSetElements.AddDefaulted_GetRef();
	AddedAttributeSetElement.SetAttributeSetInstance(NewAttributeSet);

	//변경된 요소에 대해 마킹
	AddDynamicAttributeSetContainer.MarkItemDirty(AddedAttributeSetElement);

	OnAddedAttributeSet(NewAttributeSet);
}

void UAGP_AbilitySystemComponent::RemoveDynamicAttributeSet(TSubclassOf<UAGPAttributeSetBase> InRemoveAttributeSetClass)
{
	checkf(IsOwnerActorAuthoritative(), TEXT("[%s] - Only Call Authority"), __FUNCTIONW__);
	checkf(InRemoveAttributeSetClass, TEXT("[%s], Invalid InRemoveAttributeSetClass"), __FUNCTIONW__);

	const int32 AttributeSetElementNum = AddDynamicAttributeSetContainer.AttributeSetElements.Num();
	for (int32 ElementIndex = 0; ElementIndex < AttributeSetElementNum; ++ElementIndex)
	{
		UAGPAttributeSetBase* AttributeSetInstance = AddDynamicAttributeSetContainer.AttributeSetElements[ElementIndex].AttributeSet;
		if (AttributeSetInstance && AttributeSetInstance->GetClass() == InRemoveAttributeSetClass)
		{
			OnRemoveAttributeSet(AttributeSetInstance);

			RemoveSpawnedAttribute(AttributeSetInstance);

			AddDynamicAttributeSetContainer.AttributeSetElements.RemoveAt(ElementIndex);

			//특정 요소가 지워졌으므로 Array 마킹
			AddDynamicAttributeSetContainer.MarkArrayDirty();

			break;
		}
	}
}

void UAGP_AbilitySystemComponent::SetLooseGameplayTag(const FGameplayTag& InGameplayTag)
{
	SetLooseGameplayTagCount(InGameplayTag, 1, UAbilitySystemGlobals::Get().ShouldReplicateActivationOwnedTags() ? EGameplayTagReplicationState::CountToOwner : EGameplayTagReplicationState::None);
	/*SetLooseGameplayTagCount(InGameplayTag, 1);

	if (UAbilitySystemGlobals::Get().ShouldReplicateActivationOwnedTags())
	{
		SetReplicatedLooseGameplayTagCount(InGameplayTag, 1);
	}*/
}

void UAGP_AbilitySystemComponent::RemoveLooseGameplayTag(const FGameplayTag& InGameplayTag)
{
	SetLooseGameplayTagCount(InGameplayTag, 0, UAbilitySystemGlobals::Get().ShouldReplicateActivationOwnedTags() ? EGameplayTagReplicationState::CountToOwner : EGameplayTagReplicationState::None);
	/*SetLooseGameplayTagCount(InGameplayTag, 0);

	if (UAbilitySystemGlobals::Get().ShouldReplicateActivationOwnedTags())
	{
		SetReplicatedLooseGameplayTagCount(InGameplayTag, 0);
	}*/
}

void UAGP_AbilitySystemComponent::NotifyInputPressedToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	Spec.InputPressed = true;

	if (Spec.IsActive())
	{
		//Ability가 활성화된 경우 Ability에 Input 알림 보내기
		/*if (Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Spec.Ability))
			{
				SpecAbility->OnNotifyInputPressed(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
			}
		}
		else
		{*/
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Spec.Ability))
				{
					//SpecAbility->OnNotifyInputPressed(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
					SpecAbility->OnNotifyInputPressed(Spec.Handle, AbilityActorInfo.Get(), SpecAbility->GetCurrentActivationInfo(), InputActionValue, InInputTag);
				}
			}
		//}
	}
}

void UAGP_AbilitySystemComponent::NotifyInputTriggeredToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (Spec.IsActive())
	{
		//Ability가 활성화된 경우 Ability에 Input 알림 보내기
		/*if (Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Spec.Ability))
			{
				SpecAbility->OnNotifyInputTriggered(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
			}
		}
		else
		{*/
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Instance))
				{
					//SpecAbility->OnNotifyInputTriggered(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
					SpecAbility->OnNotifyInputTriggered(Spec.Handle, AbilityActorInfo.Get(), SpecAbility->GetCurrentActivationInfo(), InputActionValue, InInputTag);
				}
			}
		//}
	}
	else
	{
		//Triggered - Ability가 활성화되지 않은 경우 Input의 Activate 이벤트를 확인해서 활성화
		if (InInputTag.MatchesTag(AGPGameplayTags::Input_Ability_Active))
		{
			TryActivateAbility(Spec.Handle);
		}
	}
}

void UAGP_AbilitySystemComponent::NotifyInputOngoningToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (Spec.IsActive())
	{
		//Ability가 활성화된 경우 Ability에 Input 알림 보내기
		/*if (Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Spec.Ability))
			{
				SpecAbility->OnNotifyInputOngoning(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
			}
		}
		else
		{*/
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Instance))
				{
					//SpecAbility->OnNotifyInputOngoning(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
					SpecAbility->OnNotifyInputOngoning(Spec.Handle, AbilityActorInfo.Get(), SpecAbility->GetCurrentActivationInfo(), InputActionValue, InInputTag);
				}
			}
		//}
	}
}

void UAGP_AbilitySystemComponent::NotifyInputReleasedToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	Spec.InputPressed = false;

	if (Spec.IsActive())
	{
		//Ability가 활성화된 경우 Ability에 Input 알림 보내기
		/*if (Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Spec.Ability))
			{
				SpecAbility->OnNotifyInputReleased(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
			}
		}
		else
		{*/
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Instance))
				{
					//SpecAbility->OnNotifyInputReleased(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
					SpecAbility->OnNotifyInputReleased(Spec.Handle, AbilityActorInfo.Get(), SpecAbility->GetCurrentActivationInfo(), InputActionValue, InInputTag);
				}
			}
		//}
	}
}

void UAGP_AbilitySystemComponent::NotifyInputCanceledToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag)
{
	if (Spec.IsActive())
	{
		//Ability가 활성화된 경우 Ability에 Input 알림 보내기
		/*if (Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Spec.Ability))
			{
				SpecAbility->OnNotifyInputCanceled(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
			}
		}
		else
		{*/
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				if (UAGPGameplayAbility* SpecAbility = Cast<UAGPGameplayAbility>(Instance))
				{
					//SpecAbility->OnNotifyInputCanceled(Spec.Handle, AbilityActorInfo.Get(), Spec.ActivationInfo, InputActionValue, InInputTag);
					SpecAbility->OnNotifyInputCanceled(Spec.Handle, AbilityActorInfo.Get(), SpecAbility->GetCurrentActivationInfo(), InputActionValue, InInputTag);
				}
			}
		//}
	}
}

void UAGP_AbilitySystemComponent::OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FAGPDynamicAttributeSetElement& RemoveElement = AddDynamicAttributeSetContainer.AttributeSetElements[Index];

		OnRemoveAttributeSet(RemoveElement.AttributeSet);
	}
}

void UAGP_AbilitySystemComponent::OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FAGPDynamicAttributeSetElement& AddedElement = AddDynamicAttributeSetContainer.AttributeSetElements[Index];

		/* 리플리케이션된 CostumeInstance의 속성을 CostumeInstance Type별로 설정 */
		OnAddedAttributeSet(AddedElement.AttributeSet);
	}
}

void UAGP_AbilitySystemComponent::OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

void UAGP_AbilitySystemComponent::OnAddedAttributeSet(UAGPAttributeSetBase* InAddAttributeSet)
{
	OnCustomAttributeSetAdded.Broadcast(InAddAttributeSet);
}

void UAGP_AbilitySystemComponent::OnRemoveAttributeSet(UAGPAttributeSetBase* InRemoveAttributeSet)
{
	OnCustomAttributeSetRemoved.Broadcast(InRemoveAttributeSet);
}
