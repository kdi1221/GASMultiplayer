// KJY All Rights Reserved


#include "Animation/AnimNotifyState/AGPANS_CollisionActive.h"
#include "Components/SkeletalMeshComponent.h"
#include "Collision/AGPCollisionHandleInterface.h"
#include "Collision/AGPCollisionHandleComponent.h"
#include "Costume/AGPCostumeFunctionLibrary.h"
#include "Collision/AGPCollisionHandleFuncLibrary.h"

void UAGPANS_CollisionActive::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	RequestCollisionActive(MeshComp->GetOwner(), true);

}

void UAGPANS_CollisionActive::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	RequestCollisionActive(MeshComp->GetOwner(), false);
}

FString UAGPANS_CollisionActive::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("CollisionActive : [%s]"), *CollisionName.ToString());
}

void UAGPANS_CollisionActive::RequestCollisionActive(AActor* InOwnerActor, bool IsActive) const
{
	if (!InOwnerActor || !InOwnerActor->Implements<UAGPCollisionHandleInterface>())
	{
		return;
	}

	UAGPCollisionHandleComponent* OwnerCollisionHandleComponent = IAGPCollisionHandleInterface::Execute_GetCollisionHandleComponent(InOwnerActor);
	if (!OwnerCollisionHandleComponent)
	{
		return;
	}

	FAGPCollisionIdentifierHandle CollisionIdentifierHandle;

	switch (CollisionType)
	{
	case EAGPCollisionType::CostumeActor:
		{
			CollisionIdentifierHandle = CreateCollisionIdentifierHandle_CostumeActor();
		}
		break;

	case EAGPCollisionType::AttachCharacter:
		{
			CollisionIdentifierHandle = CreateCollisionIdentifierHandle_AttachCharacter();
		}
		break;
	}

	OwnerCollisionHandleComponent->CollisionActiveToggle(CollisionIdentifierHandle, IsActive);
}

FAGPCollisionIdentifierHandle UAGPANS_CollisionActive::CreateCollisionIdentifierHandle_CostumeActor() const
{
	FCostumeInstanceIdentifierHandle CostumeIdentifierHandle;

	switch (CollisionProperty_CostumeActor.CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			CostumeIdentifierHandle = UAGPCostumeFunctionLibrary::CreateCostumeIdentifierFromEquipment(CollisionProperty_CostumeActor.CostumeEquipmentSlotTag);
		}
		break;
	}

	return UAGPCollisionHandleFuncLibrary::CreateCollisionIdentifierCostumeActor(CollisionName, CostumeIdentifierHandle, CollisionProperty_CostumeActor.CostumeActorName);
}

FAGPCollisionIdentifierHandle UAGPANS_CollisionActive::CreateCollisionIdentifierHandle_AttachCharacter() const
{
	return UAGPCollisionHandleFuncLibrary::CreateCollisionIdentifierAttachCharacter(CollisionName);
}
