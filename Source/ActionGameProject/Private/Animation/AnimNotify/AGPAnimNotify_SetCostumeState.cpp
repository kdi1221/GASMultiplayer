// KJY All Rights Reserved


#include "Animation/AnimNotify/AGPAnimNotify_SetCostumeState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/AGP_CharacterBase.h"
#include "Costume/AGPCostumeStruct.h"
#include "Characters/Components/AGPCharPresentationComponent.h"
#include "Log/AGPLogChannels.h"

FString UAGPAnimNotify_SetCostumeState::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Set Costume Actor State : [%s]"), *CostumeActorStatus.ToString());
}

void UAGPAnimNotify_SetCostumeState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
	{
		return;
	}

	AAGP_CharacterBase* OwnerCharacter = Cast<AAGP_CharacterBase>(MeshComp->GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	UAGPCharPresentationComponent* CharacterPresentationComponent = OwnerCharacter->GetCharacterPresentationComponent();
	verifyf(CharacterPresentationComponent, TEXT("[%s], CharacterPresentationComponent Invalid"), __FUNCTIONW__);

	FCostumeInstanceIdentifierHandle CostumeIdentifierHandle;

	/* 지정한 CostumeType별로 다른 Identifier 생성 */
	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			CostumeIdentifierHandle.SetCostumeInstanceIdentifier(MakeShared<FCostumeInstanceEquipmentIdentifier>(CostumeEquipmentSlotTag));
		}
		break;
	}

	if (CostumeIdentifierHandle.IsValid())
	{
		CharacterPresentationComponent->SetCostumeActorStatus(CostumeIdentifierHandle, CostumeActorName, CostumeActorStatus);
	}
	else
	{
		AGP_NET_LOG(this, LogAGPAnimation, Warning, TEXT("Invalid CostumeIdentifier, CostumeType[%s]"), *UEnum::GetDisplayValueAsText(CostumeType).ToString());
	}
}
