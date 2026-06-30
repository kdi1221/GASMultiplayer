// KJY All Rights Reserved


#include "Collision/AGPCollisionHandleComponent.h"
#include "Components/ShapeComponent.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "Collision/AGPCollisionStruct.h"
#include "Costume/AGPCostumeStruct.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

UAGPCollisionHandleComponent::UAGPCollisionHandleComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(false);
}

void UAGPCollisionHandleComponent::InitializeOnPossedToOwner(UAGP_AbilitySystemComponent* InOwnerASC)
{
	checkf(InOwnerASC, TEXT("[%s] - Invalid OwnerASC"), __FUNCTIONW__);

	CachedOwnerASC = InOwnerASC;
}

void UAGPCollisionHandleComponent::OnUnpossedFromOwner()
{
	CachedOwnerASC.Reset();

	AllCollisionDisable();
}

void UAGPCollisionHandleComponent::AllCollisionDisable()
{
	/* 모든 Collision 비활성화 */
	for (TPair<FAGPCollisionCostumeEquipmentMapKey, FAGPCollisionElement> PairEquipmentCostume : CollisionCostumeDelegateMap.FromEquipmentCollisionMap)
	{
		ToggleCollisionActive(PairEquipmentCostume.Value, false);
	}
}

void UAGPCollisionHandleComponent::RegisterCollision(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle, UShapeComponent* InCollisionComponent)
{
	/* Authority일때만 Collision 처리 */
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	checkf(InCollisionComponent, TEXT("[%s], Invalid InCollisionComponent"), __FUNCTIONW__);

	const FAGPCollisionIdentifierBase* CollisionIdentifierBase = InCollisionIdentifierHandle.GetCollisionIdentifier().Get();
	checkf(CollisionIdentifierBase, TEXT("[%s], Invalid CollisionIdentifier"), __FUNCTIONW__);

	switch (CollisionIdentifierBase->GetCollisionType())
	{
	case EAGPCollisionType::CostumeActor:
		{
			RegisterCollision_CostumeActor(static_cast<const FAGPCollisionIdentifierCostumeActor&>(*CollisionIdentifierBase), InCollisionComponent);	
		}
		break;

	case EAGPCollisionType::AttachCharacter:
		{
			RegisterCollision_AttachCharacter(static_cast<const FAGPCollisionIdentifierAttachCharacter&>(*CollisionIdentifierBase), InCollisionComponent);
		}
		break;
	}
}

void UAGPCollisionHandleComponent::UnregisterCollision(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle)
{
	/* Authority일때만 Collision 처리 */
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	const FAGPCollisionIdentifierBase* CollisionIdentifierBase = InCollisionIdentifierHandle.GetCollisionIdentifier().Get();
	checkf(CollisionIdentifierBase, TEXT("[%s], Invalid CollisionIdentifier"), __FUNCTIONW__);

	switch (CollisionIdentifierBase->GetCollisionType())
	{
	case EAGPCollisionType::CostumeActor:
		{
			UnregisterCollision_CostumeActor(static_cast<const FAGPCollisionIdentifierCostumeActor&>(*CollisionIdentifierBase));
		}
		break;

	case EAGPCollisionType::AttachCharacter:
		{
			UnregisterCollision_AttachCharacter(static_cast<const FAGPCollisionIdentifierAttachCharacter&>(*CollisionIdentifierBase));
		}
		break;
	}
}

void UAGPCollisionHandleComponent::OnOverlapNotifyHitEvent(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult)
{
	/* Authority일때만 Collision 처리 */
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	if (!IsValid(InCollisionOtherActor))
	{
		return;
	}

	const FAGPCollisionIdentifierBase* CollisionIdentifierBase = InCollisionIdentifierHandle.GetCollisionIdentifier().Get();
	checkf(CollisionIdentifierBase, TEXT("[%s], Invalid CollisionIdentifier"), __FUNCTIONW__);

	switch (CollisionIdentifierBase->GetCollisionType())
	{
	case EAGPCollisionType::CostumeActor:
		{
			OnOverlapNotifyHitEvent_CostumeActor(static_cast<const FAGPCollisionIdentifierCostumeActor&>(*CollisionIdentifierBase), InHitEventTag, InInstigatorActor, InCollisionOtherActor, SweepResult);
		}
		break;

	case EAGPCollisionType::AttachCharacter:
		{
			OnOverlapNotifyHitEvent_AttachCharacter(static_cast<const FAGPCollisionIdentifierAttachCharacter&>(*CollisionIdentifierBase), InHitEventTag, InInstigatorActor, InCollisionOtherActor, SweepResult);
		}
		break;
	}
}

void UAGPCollisionHandleComponent::CollisionActiveToggle(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle, bool IsActive)
{
	/* Authority일때만 Collision 처리 */
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	const FAGPCollisionIdentifierBase* CollisionIdentifierBase = InCollisionIdentifierHandle.GetCollisionIdentifier().Get();
	checkf(CollisionIdentifierBase, TEXT("[%s], Invalid CollisionIdentifier"), __FUNCTIONW__);

	switch (CollisionIdentifierBase->GetCollisionType())
	{
	case EAGPCollisionType::CostumeActor:
		{
			CollisionActiveToggle_CostumeActor(static_cast<const FAGPCollisionIdentifierCostumeActor&>(*CollisionIdentifierBase), IsActive);
		}
		break;

	case EAGPCollisionType::AttachCharacter:
		{
			CollisionActiveToggle_AttachCharacter(static_cast<const FAGPCollisionIdentifierAttachCharacter&>(*CollisionIdentifierBase), IsActive);
		}
		break;
	}
}

void UAGPCollisionHandleComponent::RegisterCollision_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor, UShapeComponent* InCollisionComponent)
{
	checkf(InCollisionComponent, TEXT("[%s], Invalid InCollisionComponent"), __FUNCTIONW__);

	const FCostumeInstanceIdentifierBase* CostumeIdentifierBase = InCollisionIdentifierCostumeActor.GetCostumeInstanceIdentifier().Get();
	checkf(CostumeIdentifierBase, TEXT("[%s], Invalid CostumeIdentifier"), __FUNCTIONW__);

	const FName& CollisionName = InCollisionIdentifierCostumeActor.GetCollisionName();
	const FName& CostumeActorName = InCollisionIdentifierCostumeActor.GetCostumeActorName();
	const EAGPCostumeType CostumeType = CostumeIdentifierBase->GetCostumeIdentifierType();
	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			const FCostumeInstanceEquipmentIdentifier& CostumeIdentifierEquipment = static_cast<const FCostumeInstanceEquipmentIdentifier&>(*CostumeIdentifierBase);
			const FGameplayTag& CostumeEquipmentTag = CostumeIdentifierEquipment.GetTagEquipmentSlot();

			FAGPCollisionElement& AddedCollisionElement = CollisionCostumeDelegateMap.FromEquipmentCollisionMap.Add(FAGPCollisionCostumeEquipmentMapKey(CostumeEquipmentTag, CostumeActorName, CollisionName));
			AddedCollisionElement.CachedCollisionComponent = InCollisionComponent;

			//AGP_NET_LOG(this, LogAGPCollision, Log, TEXT("Collision Name[%s]"), *CollisionName.ToString());
		}
		break;
	}
}

void UAGPCollisionHandleComponent::RegisterCollision_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter, UShapeComponent* InCollisionComponent)
{
	checkf(InCollisionComponent, TEXT("[%s], Invalid InCollisionComponent"), __FUNCTIONW__);

	const FName& CollisionName = InCollisionIdentifierAttachCharacter.GetCollisionName();

	FAGPCollisionElement& AddedCollisionElement = CollisionAttachCharacterMap.Add(CollisionName);
	AddedCollisionElement.CachedCollisionComponent = InCollisionComponent;
}

void UAGPCollisionHandleComponent::UnregisterCollision_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor)
{
	const FCostumeInstanceIdentifierBase* CostumeIdentifierBase = InCollisionIdentifierCostumeActor.GetCostumeInstanceIdentifier().Get();
	checkf(CostumeIdentifierBase, TEXT("[%s], Invalid CostumeIdentifier"), __FUNCTIONW__);

	const FName& CollisionName = InCollisionIdentifierCostumeActor.GetCollisionName();
	const FName& CostumeActorName = InCollisionIdentifierCostumeActor.GetCostumeActorName();
	const EAGPCostumeType CostumeType = CostumeIdentifierBase->GetCostumeIdentifierType();

	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			const FCostumeInstanceEquipmentIdentifier& CostumeIdentifierEquipment = static_cast<const FCostumeInstanceEquipmentIdentifier&>(*CostumeIdentifierBase);
			const FGameplayTag& CostumeEquipmentTag = CostumeIdentifierEquipment.GetTagEquipmentSlot();

			//Collision 등록해제
			FAGPCollisionCostumeEquipmentMapKey RemoveCollisionDelegateKey(CostumeEquipmentTag, CostumeActorName, CollisionName);

			if (CollisionCostumeDelegateMap.FromEquipmentCollisionMap.Contains(RemoveCollisionDelegateKey))
			{
				CollisionCostumeDelegateMap.FromEquipmentCollisionMap.Remove(RemoveCollisionDelegateKey);
			}

			//AGP_NET_LOG(this, LogAGPCollision, Log, TEXT("Collision Name[%s]"), *CollisionName.ToString());
		}
		break;
	}
}

void UAGPCollisionHandleComponent::UnregisterCollision_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter)
{
	const FName& CollisionName = InCollisionIdentifierAttachCharacter.GetCollisionName();

	if (CollisionAttachCharacterMap.Contains(CollisionName))
	{
		CollisionAttachCharacterMap.Remove(CollisionName);
	}
}

void UAGPCollisionHandleComponent::OnOverlapNotifyHitEvent_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult)
{
	const FCostumeInstanceIdentifierBase* CostumeIdentifierBase = InCollisionIdentifierCostumeActor.GetCostumeInstanceIdentifier().Get();
	checkf(CostumeIdentifierBase, TEXT("[%s], Invalid CostumeIdentifier"), __FUNCTIONW__);

	const FName& CollisionName = InCollisionIdentifierCostumeActor.GetCollisionName();
	const FName& CostumeActorName = InCollisionIdentifierCostumeActor.GetCostumeActorName();
	const EAGPCostumeType CostumeType = CostumeIdentifierBase->GetCostumeIdentifierType();

	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			const FCostumeInstanceEquipmentIdentifier& CostumeIdentifierEquipment = static_cast<const FCostumeInstanceEquipmentIdentifier&>(*CostumeIdentifierBase);
			const FGameplayTag& CostumeEquipmentTag = CostumeIdentifierEquipment.GetTagEquipmentSlot();

			//Collision 찾아서 알림
			FAGPCollisionCostumeEquipmentMapKey FindCollisionDelegateKey(CostumeEquipmentTag, CostumeActorName, CollisionName);

			if (FAGPCollisionElement* FindCollisionElement = CollisionCostumeDelegateMap.FromEquipmentCollisionMap.Find(FindCollisionDelegateKey))
			{
				NotifyHitEvent(*FindCollisionElement, InHitEventTag, InInstigatorActor, InCollisionOtherActor, SweepResult);
			}
			else
			{
				AGP_NET_LOG(this, LogAGPCollision, Warning, TEXT("Invalid Collision Element - Collision Name[%s]"), *CollisionName.ToString());
			}
		}
		break;
	}
}

void UAGPCollisionHandleComponent::OnOverlapNotifyHitEvent_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult)
{
	const FName& CollisionName = InCollisionIdentifierAttachCharacter.GetCollisionName();

	if (FAGPCollisionElement* FindCollisionElement = CollisionAttachCharacterMap.Find(CollisionName))
	{
		NotifyHitEvent(*FindCollisionElement, InHitEventTag, InInstigatorActor, InCollisionOtherActor, SweepResult);
	}
	else
	{
		AGP_NET_LOG(this, LogAGPCollision, Warning, TEXT("Invalid Collision Element - Collision Name[%s]"), *CollisionName.ToString());
	}
}

void UAGPCollisionHandleComponent::CollisionActiveToggle_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor, bool IsActive)
{
	const FCostumeInstanceIdentifierBase* CostumeIdentifierBase = InCollisionIdentifierCostumeActor.GetCostumeInstanceIdentifier().Get();
	checkf(CostumeIdentifierBase, TEXT("[%s], Invalid CostumeIdentifier"), __FUNCTIONW__);

	const FName& CollisionName = InCollisionIdentifierCostumeActor.GetCollisionName();
	const FName& CostumeActorName = InCollisionIdentifierCostumeActor.GetCostumeActorName();
	const EAGPCostumeType CostumeType = CostumeIdentifierBase->GetCostumeIdentifierType();

	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			const FCostumeInstanceEquipmentIdentifier& CostumeIdentifierEquipment = static_cast<const FCostumeInstanceEquipmentIdentifier&>(*CostumeIdentifierBase);
			const FGameplayTag& CostumeEquipmentTag = CostumeIdentifierEquipment.GetTagEquipmentSlot();

			//Collision Active Toggle
			FAGPCollisionCostumeEquipmentMapKey ActiveCollisionDelegateKey(CostumeEquipmentTag, CostumeActorName, CollisionName);

			if (CollisionCostumeDelegateMap.FromEquipmentCollisionMap.Contains(ActiveCollisionDelegateKey))
			{
				ToggleCollisionActive(CollisionCostumeDelegateMap.FromEquipmentCollisionMap[ActiveCollisionDelegateKey], IsActive);
			}

			//AGP_NET_LOG(this, LogAGPCollision, Log, TEXT("Collision Name[%s], Active[%s]"), *CollisionName.ToString(), IsActive ? TEXT("TRUE") : TEXT("FALSE"));
		}
		break;
	}
}

void UAGPCollisionHandleComponent::CollisionActiveToggle_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter, bool IsActive)
{
	const FName& CollisionName = InCollisionIdentifierAttachCharacter.GetCollisionName();

	if (CollisionAttachCharacterMap.Contains(CollisionName))
	{
		ToggleCollisionActive(CollisionAttachCharacterMap[CollisionName], IsActive);
	}
}

void UAGPCollisionHandleComponent::ToggleCollisionActive(FAGPCollisionElement& InCollisionElement, bool IsActive)
{
	InCollisionElement.DetectedOverlapActors.Empty();

	if (UShapeComponent* CollisionComponent = InCollisionElement.CachedCollisionComponent.Get())
	{
		CollisionComponent->SetCollisionEnabled(IsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void UAGPCollisionHandleComponent::NotifyHitEvent(FAGPCollisionElement& InCollisionElement, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult)
{
	checkf(InCollisionOtherActor, TEXT("[%s], Invalid InCollisionOtherActor"), __FUNCTIONW__);
	
	/* Owner인 경우 Pass */
	if (GetOwner() == InCollisionOtherActor)
	{
		return;
	}
	 
	/* 이미 이전에 Collision 체크된 Actor면 Pass */
	if (InCollisionElement.DetectedOverlapActors.Contains(InCollisionOtherActor))
	{
		return;
	}

	InCollisionElement.DetectedOverlapActors.AddUnique(InCollisionOtherActor);

	if(UAGP_AbilitySystemComponent* OwnerASC = CachedOwnerASC.Get())
	{
		FScopedPredictionWindow NewScopedWindow(OwnerASC, true);

		FGameplayEventData HitEventPayloadData;
		HitEventPayloadData.EventTag = InHitEventTag;
		HitEventPayloadData.Instigator = InInstigatorActor;
		HitEventPayloadData.Target = InCollisionOtherActor;

		OwnerASC->HandleGameplayEvent(InHitEventTag, &HitEventPayloadData);
	}
	else
	{
		AGP_NET_LOG(this, LogAGPCollision, Warning, TEXT("Invalid CachedOwnerASC"));
	}
}