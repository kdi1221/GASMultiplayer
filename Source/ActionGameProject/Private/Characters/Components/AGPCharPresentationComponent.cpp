// KJY All Rights Reserved


#include "Characters/Components/AGPCharPresentationComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstances/AGPLinkedAnimLayerBase.h"
#include "Animation/AnimInstances/AGPCharacterAnimInstance.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "Costume/AttachCostumeBase.h"
#include "Costume/AGPCostumeActorBase.h"
#include "AGPGameplayTags.h"
#include "Costume/AGPCostumeStruct.h"
#include "Common/AGPCommonFunctionLibrary.h"
#include "Log/AGPLogChannels.h"

UAGPCharPresentationComponent::UAGPCharPresentationComponent()
{
	bWantsInitializeComponent = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(false);
}

void UAGPCharPresentationComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && EWorldType::EditorPreview != CurrentWorld->WorldType)
	{
		// Owner가 Destroy될때 호출될 이벤트 설정(Attach된 Costume Actor들 제거) 
		AActor* ComponentOwner = GetOwner();
		verifyf(ComponentOwner, TEXT("[%s] - ComponentOwner Invalid"), __FUNCTIONW__);

		ComponentOwner->OnDestroyed.AddDynamic(this, &UAGPCharPresentationComponent::OnDestroyOwnerCallback);
	}
}

void UAGPCharPresentationComponent::SetCostumeActorInstigator(APawn* InInstigatorPawn)
{
	CachedCostumeActorInstigator = InInstigatorPawn;
}

void UAGPCharPresentationComponent::SetOwnerSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComponent)
{
	checkf(InSkeletalMeshComponent, TEXT("[%s], Invalid InSkeletalMeshComponent"), __FUNCTIONW__);
	CachedOwnerSkeletalMeshComponent = InSkeletalMeshComponent;

	CachedOwnerAnimInstance = Cast<UAGPCharacterAnimInstance>(InSkeletalMeshComponent->GetAnimInstance());

	if (ENetRole::ROLE_Authority == GetOwnerRole())
	{
		OnChangeOwnerSkeletalMeshAnimRefreshBone();
	}
}

void UAGPCharPresentationComponent::AddCostumeActor(UAttachCostumeBase* InAddCostumeElement)
{
	checkf(InAddCostumeElement, TEXT("[%s], InAddCostumeElement Invalid"), __FUNCTIONW__);

	AActor* ComponentOwner = GetOwner();
	checkf(ComponentOwner, TEXT("[%s] - ComponentOwner Invalid"), __FUNCTIONW__);

	APawn* CostumeActorInstigator = CachedCostumeActorInstigator.Get();
	checkf(CostumeActorInstigator, TEXT("[%s] - [%s] Character CostumeActorInstigator Invalid"), __FUNCTIONW__, *GetNameSafe(ComponentOwner));

	InAddCostumeElement->SpawnCostumeActors(
		ComponentOwner,
		CostumeActorInstigator,
		FCostumeActorSpawnDelegate::CreateLambda(
			[&](AAGPCostumeActorBase* InSpawnedCostumeActor)
			{
				checkf(InSpawnedCostumeActor, TEXT("Invalid InSpawnedCostumeActor"));

				InSpawnedCostumeActor->AttachToOwner(this);

				/* CostumeType별로 다르게 저장 */
				switch (InAddCostumeElement->GetCostumeType())
				{
				case EAGPCostumeType::FromEquipment:
					{
						const FGameplayTag& EquipmentSlotTag = InSpawnedCostumeActor->GetSoruceEquipmentSlotTag();
						checkf(EquipmentSlotTag.IsValid(), TEXT("[%s] - Invalid EquipmentSlotTag"), __FUNCTIONW__);
						const FName& CostumeActorName = InSpawnedCostumeActor->GetCostumeName();

						AttachedCostumeActorFromEquipmentMap.Add(FAGPCostumeEquipmentMapKey(EquipmentSlotTag, CostumeActorName), InSpawnedCostumeActor);
					}
					break;		
				}
			}
		)
	);
}

void UAGPCharPresentationComponent::RemoveCostumeActor(const UAttachCostumeBase* InRemoveCostumeElement)
{
	verifyf(InRemoveCostumeElement, TEXT("[%s], InRemoveCostumeElement Invalid"), __FUNCTIONW__);

	switch (InRemoveCostumeElement->GetCostumeType())
	{
	case EAGPCostumeType::FromEquipment:
		{
			const FGameplayTag& EquipmentSlotTag = InRemoveCostumeElement->GetSoruceEquipmentSlotTag();

			TArray<FAGPCostumeEquipmentMapKey> ReserveRemoveKey;
			for (TPair<FAGPCostumeEquipmentMapKey, TWeakObjectPtr<AAGPCostumeActorBase>>& PairEquipmentCostumeActor : AttachedCostumeActorFromEquipmentMap)
			{
				if (!PairEquipmentCostumeActor.Key.TagEquipmentSlot.MatchesTagExact(EquipmentSlotTag))
				{
					continue;
				}

				ReserveRemoveKey.Add(PairEquipmentCostumeActor.Key);
				if (AAGPCostumeActorBase* AttachedCostumeActor = PairEquipmentCostumeActor.Value.Get())
				{
					//InRemoveCostumeElement를 Owner로 하는 CostumeActor들을 찾아서 Destroy한 후 이후 Map에서 제거
					AttachedCostumeActor->Destroy();
				}
				else
				{
					AGP_NET_LOG(this, LogAGPCostume, Warning, TEXT("Invalid AttachedCostumeActor, CostumeEquipment[%s], CostumeName[%s], Owner[%s]"), *PairEquipmentCostumeActor.Key.TagEquipmentSlot.ToString(), *PairEquipmentCostumeActor.Key.CostumeActorName.ToString(), *GetNameSafe(GetOwner()));
				}
			}

			for (const FAGPCostumeEquipmentMapKey& RemoveKey : ReserveRemoveKey)
			{
				AttachedCostumeActorFromEquipmentMap.Remove(RemoveKey);
			}
		}
		break;
	}
}

void UAGPCharPresentationComponent::AllDestroyAttachCostumeActors()
{
	/*Equipment CostumeActor들 정리 */
	for (TPair<FAGPCostumeEquipmentMapKey, TWeakObjectPtr<AAGPCostumeActorBase>>& PairEquipmentCostumeActor : AttachedCostumeActorFromEquipmentMap)
	{
		if (AAGPCostumeActorBase* AttachedCostumeActor = PairEquipmentCostumeActor.Value.Get())
		{
			AttachedCostumeActor->Destroy();
		}
	}

	AttachedCostumeActorFromEquipmentMap.Empty();
}

void UAGPCharPresentationComponent::SetAnimationLink(const FName& InNewAnimLinkName)
{
	CurrentAnimLinkName = InNewAnimLinkName;

	OnChangeAnimationLink();
}

void UAGPCharPresentationComponent::InitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC)
{
	checkf(InOwnerASC, TEXT("[%s] - Invalid OwnerASC"), __FUNCTIONW__);

	/* 기존에 있던 Tag정보들을 제거 */
	const TArray<FGameplayTag>& OldOwnedTags = OwnedGameplayTags.GetGameplayTagArray();
	for (const FGameplayTag& InOldTag : OldOwnedTags)
	{
		NotifyGameplayTagEventToAttachCostumeActors(InOldTag, false);
	}
	OwnedGameplayTags.Reset();

	const FGameplayTagContainer& OwnerExplicitTags = InOwnerASC->GetOwnedGameplayTags();
	for (const FGameplayTag& RegisterTag : RegisterCallbackGameplayTags)
	{
		/* OwnerASC에 있던 Tag들을 확인해서 추가 */
		if (OwnerExplicitTags.HasTagExact(RegisterTag))
		{
			OwnedGameplayTags.AddTag(RegisterTag);
			NotifyGameplayTagEventToAttachCostumeActors(RegisterTag, true);
		}

		/* GameplayTag들의 적용에 대한 이벤트 콜백 설정 */
		InOwnerASC->RegisterGameplayTagEvent(RegisterTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAGPCharPresentationComponent::OnAddOrRemovedGameplayTags);
	}

	/* AnimRefreshBone Tag 관련 이벤트 등록 및 Tag여부 확인 (Authority Only) */
	if (ENetRole::ROLE_Authority == GetOwnerRole())
	{
		IsApplyAnimTickRefreshBone = OwnerExplicitTags.HasTagExact(AGPGameplayTags::Animation_SetProperty_AnimRefreshBone);
		OnChangeOwnerSkeletalMeshAnimRefreshBone();
		InOwnerASC->RegisterGameplayTagEvent(AGPGameplayTags::Animation_SetProperty_AnimRefreshBone, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UAGPCharPresentationComponent::OnAnimRefreshBoneTagChanged);
	}
}

void UAGPCharPresentationComponent::UninitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC)
{
	checkf(InOwnerASC, TEXT("[%s] - Invalid OwnerASC"), __FUNCTIONW__);

	for (const FGameplayTag& RegisterTag : RegisterCallbackGameplayTags)
	{
		InOwnerASC->RegisterGameplayTagEvent(RegisterTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}

	InOwnerASC->RegisterGameplayTagEvent(AGPGameplayTags::Animation_SetProperty_AnimRefreshBone, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
}

void UAGPCharPresentationComponent::SetCostumeActorStatus(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, const FName& InCostumeActorName, const FName& InActorStatus)
{
	checkf(InCostumeIdentifierHandle.IsValid(), TEXT("[%s], Invalid ConstumeInstance Identifier, CostumeActorName[%s]"), __FUNCTIONW__, *InCostumeActorName.ToString());

	AAGPCostumeActorBase* AttachedCostumeActor = GetAttachedCostumeActor(InCostumeIdentifierHandle, InCostumeActorName);
	if(AttachedCostumeActor)
	{
		AttachedCostumeActor->SetCurrentStatus(InActorStatus);
	}
	else
	{
		//TODO : FCostumeInstanceIdentifierBase FString으로 반환하는 함수 추가필요
		//AGP_NET_LOG(this, LogAGPCostume, Warning, TEXT("Invalid Costume Actor, CostumeType[%s], ActorName[%s]"), *UEnum::GetDisplayValueAsText(FindCostumeType).ToString(), *InCostumeActorName.ToString());
		AGP_NET_LOG(this, LogAGPCostume, Warning, TEXT("Invalid Costume Actor, ActorName[%s]"), *InCostumeActorName.ToString());
	}
}

void UAGPCharPresentationComponent::OnOwnerHitEvent()
{
	for (TPair<FAGPCostumeEquipmentMapKey, TWeakObjectPtr<AAGPCostumeActorBase>>& PairEquipmentCostumeActor : AttachedCostumeActorFromEquipmentMap)
	{
		if (AAGPCostumeActorBase* AttachedCostumeActor = PairEquipmentCostumeActor.Value.Get())
		{
			AttachedCostumeActor->OnOwnerHitEvent();
		}
	}
}

void UAGPCharPresentationComponent::OnOwnerDeathDestroy(const bool IsShowDissolveFX)
{
	/*Equipment CostumeActor들에게 Owner Death 알림 */
	for (TPair<FAGPCostumeEquipmentMapKey, TWeakObjectPtr<AAGPCostumeActorBase>>& PairEquipmentCostumeActor : AttachedCostumeActorFromEquipmentMap)
	{
		if (AAGPCostumeActorBase* AttachedCostumeActor = PairEquipmentCostumeActor.Value.Get())
		{
			AttachedCostumeActor->OnOwnerDeathDestroy(IsShowDissolveFX);
		}
	}
}

bool UAGPCharPresentationComponent::HasOwnedGameplayTag(const FGameplayTag& InTagToCheck) const
{
	return OwnedGameplayTags.HasTagExact(InTagToCheck);
}

UAnimMontage* UAGPCharPresentationComponent::GetMontageWithKey(const FName& InMontageKey) const
{
	const TObjectPtr<UAnimMontage>* FindMontage = OwnerPawnMontageMap.Find(InMontageKey);
	return FindMontage ? FindMontage->Get() : nullptr;
}

UAnimMontage* UAGPCharPresentationComponent::GetRandomHitReactMontageWithKey(const FName& InMontageKey) const
{
	const TObjectPtr<UAnimMontage>* FindMontage = HitReactRandomMontageMap.Find(InMontageKey);
	return FindMontage ? FindMontage->Get() : nullptr;
}

FName UAGPCharPresentationComponent::GetRandomHitReactMontageKey() const
{
	return UAGPCommonFunctionLibrary::GetRandomSelectMapKey(HitReactRandomMontageMap);
}

UAnimMontage* UAGPCharPresentationComponent::GetDirectionHitReactMontage(const EAGPDirection InDirection) const
{
	const TObjectPtr<UAnimMontage>* FindMontage = HitReactDirectionMontageMap.Find(InDirection);
	return FindMontage ? FindMontage->Get() : nullptr;
}

FName UAGPCharPresentationComponent::GetRandomDeathPoseName() const
{
	return UAGPCommonFunctionLibrary::GetRandomSelectMapKey(DeathMontagePoseMap);
}

UAnimMontage* UAGPCharPresentationComponent::GetDeathMontageFromDeathPose(const FName& InDeathPoseName) const
{
	const FAGPDeathPoseAnimMontages* FindDeathPose = DeathMontagePoseMap.Find(InDeathPoseName);
	return FindDeathPose ? FindDeathPose->DeathAnimMontage : nullptr;
}

UAnimMontage* UAGPCharPresentationComponent::GetRebirthMontageFromDeathPose(const FName& InDeathPoseName) const
{
	const FAGPDeathPoseAnimMontages* FindDeathPose = DeathMontagePoseMap.Find(InDeathPoseName);
	return FindDeathPose ? FindDeathPose->RebirthAnimMontage : nullptr;
}

const FName& UAGPCharPresentationComponent::GetSpecificSocketName(const FName& InAbstractSocketName) const
{
	static const FName EmptySocketName;

	const FName* FindSocketName = CostumeAttachSocketMapping.Find(InAbstractSocketName);

	return FindSocketName ? *FindSocketName : EmptySocketName;
}

USkeletalMeshComponent* UAGPCharPresentationComponent::GetOwenrSkeletalMeshComponent() const
{
	return CachedOwnerSkeletalMeshComponent.Get();
}

bool UAGPCharPresentationComponent::CheckHitReactFlag(int32 InBitMask)
{
	return static_cast<int32>(HitReactFlag) & InBitMask;
}

bool UAGPCharPresentationComponent::CheckDeathFlag(int32 InBitMask)
{
	return static_cast<int32>(DeathFlag) & InBitMask;
}

AAGPCostumeActorBase* UAGPCharPresentationComponent::GetAttachedCostumeActor(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, const FName& InCostumeActorName)
{
	checkf(InCostumeIdentifierHandle.IsValid(), TEXT("[%s], Invalid ConstumeInstance Identifier, CostumeActorName[%s]"), __FUNCTIONW__, *InCostumeActorName.ToString());

	const FCostumeInstanceIdentifierBase& CostumeIdentifierBase = *InCostumeIdentifierHandle.GetCostumeInstanceIdentifier().Get();
	const EAGPCostumeType FindCostumeType = CostumeIdentifierBase.GetCostumeIdentifierType();

	TWeakObjectPtr<AAGPCostumeActorBase>* FindAttachedCostumeActor = nullptr;
	switch (FindCostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			/* 장비로부터 추가된 CostumeActor의 경우 AttachedCostumeActorFromEquipmentMap에서 찾는다. */
			const FCostumeInstanceEquipmentIdentifier& EquipmentIdentifier = static_cast<const FCostumeInstanceEquipmentIdentifier&>(CostumeIdentifierBase);
			const FGameplayTag& CostumeEquipmentSlot = EquipmentIdentifier.GetTagEquipmentSlot();

			FindAttachedCostumeActor = AttachedCostumeActorFromEquipmentMap.Find(FAGPCostumeEquipmentMapKey(CostumeEquipmentSlot, InCostumeActorName));
		}
		break;
	}

	return FindAttachedCostumeActor->Get();
}

void UAGPCharPresentationComponent::OnChangeAnimationLink()
{
	if (const TSubclassOf<UAGPLinkedAnimLayerBase>* CurrentAnimLinkInstance = LinkAnimLayerMap.Find(CurrentAnimLinkName))
	{
		if (UAGPCharacterAnimInstance* OwnerAnimInstance = CachedOwnerAnimInstance.Get())
		{
			OwnerAnimInstance->SetAnimationLinkLayer(*CurrentAnimLinkInstance);
		}
		else
		{
			AGP_NET_LOG(this, LogAGPAnimation, Warning, TEXT("Invalid OwnerAnimInstance, Owner[%s]"), *GetNameSafe(this));
		}

	}
	else
	{
		AGP_NET_LOG(this, LogAGPAnimation, Warning, TEXT("Invalid CurrentAnimLink[%s], Owner[%s]"), *CurrentAnimLinkName.ToString(), *GetNameSafe(this));
	}
}

void UAGPCharPresentationComponent::OnAddOrRemovedGameplayTags(const FGameplayTag Tag, int32 NewCount)
{
	bool IsAdded = false;
	if (NewCount > 0)
	{
		/* 새로운 Tag 추가 */
		OwnedGameplayTags.AddTag(Tag);
		IsAdded = true;

		//AGP_NET_LOG(this, LogAGPAnimation, Log, TEXT("%s Tag Add, Count[%d]"), *Tag.ToString(), NewCount);
	}
	else
	{
		/* 기존에 있던 Tag 삭제 */
		OwnedGameplayTags.RemoveTag(Tag);
		IsAdded = false;

		//AGP_NET_LOG(this, LogAGPAnimation, Log, TEXT("%s Tag Removed, Count[%d]"), *Tag.ToString(), NewCount);
	}

	/* Attach된 CostumeActor들에게 GameplayTag의 추가, 삭제를 알린다. */
	NotifyGameplayTagEventToAttachCostumeActors(Tag, IsAdded);
}

void UAGPCharPresentationComponent::OnAnimRefreshBoneTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool OldApplyAnimTickRefreshBone = IsApplyAnimTickRefreshBone;
	IsApplyAnimTickRefreshBone = NewCount > 0;

	if (OldApplyAnimTickRefreshBone != IsApplyAnimTickRefreshBone)
	{
		OnChangeOwnerSkeletalMeshAnimRefreshBone();
	}
}

void UAGPCharPresentationComponent::NotifyGameplayTagEventToAttachCostumeActors(const FGameplayTag& Tag, const bool IsAdded)
{
	/*Equipment CostumeActor들에게 Tag 이벤트 알림 */
	for (TPair<FAGPCostumeEquipmentMapKey, TWeakObjectPtr<AAGPCostumeActorBase>>& PairEquipmentCostumeActor : AttachedCostumeActorFromEquipmentMap)
	{
		if (AAGPCostumeActorBase* AttachedCostumeActor = PairEquipmentCostumeActor.Value.Get())
		{
			AttachedCostumeActor->OnNotifyOwnerGameplayTagEvents(Tag, IsAdded);
		}
	}
}

void UAGPCharPresentationComponent::OnChangeOwnerSkeletalMeshAnimRefreshBone()
{
	USkeletalMeshComponent* OwnerSkeletalMeshComponent = GetOwenrSkeletalMeshComponent();
	if (!OwnerSkeletalMeshComponent)
	{
		return;
	}

	OwnerSkeletalMeshComponent->VisibilityBasedAnimTickOption = IsApplyAnimTickRefreshBone ? 
																EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones : 
																EVisibilityBasedAnimTickOption::AlwaysTickPose;
}

void UAGPCharPresentationComponent::OnDestroyOwnerCallback(AActor* InDestryoedOwner)
{
	//AGP_NET_LOG(this, LogAGPCostume, Log, TEXT("Owner[%s]"), *GetNameSafe(InDestryoedOwner));

	AllDestroyAttachCostumeActors();
}