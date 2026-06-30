// KJY All Rights Reserved


#include "Item/Components/AGPAbstractionsAvatarComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Costume/AttachCostumeBase.h"
#include "Costume/AGPCostumeActorBase.h"
#include "Costume/AGPCostumeStruct.h"
#include "Characters/AGP_CharacterBase.h"
#include "Characters/Components/AGPCharPresentationComponent.h"

#include "Log/AGPLogChannels.h"

FAGPAttachCostumeElement::FAGPAttachCostumeElement()
{

}

void FAGPAttachCostumeElement::SetCostumeInstance(UAttachCostumeBase* InCostumeInstance)
{
	CostumeInstance = InCostumeInstance;

	CostumeType = CostumeInstance->GetCostumeType();

	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			CostumeEquipmentSlotTag = InCostumeInstance->GetSoruceEquipmentSlotTag();
		}
		break;
	}
}

bool FAGPAttachCostumeElement::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << CostumeInstance;
	Ar << CostumeType;

	/* CostumeType에 따라 리플리케이션할 변수들을 결정한다. */
	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			Ar << CostumeEquipmentSlotTag;
		}
		break;
	}

	bOutSuccess = true;

	return true;
}

bool FAGPAttachCostumeElement::Identical(const FAGPAttachCostumeElement* Other, uint32 PortFlags) const
{
	if (!Other)
	{
		return false;
	}

	/* 공통 속성 비교, 다르면 return false */
	const bool SameCostumeInstance = CostumeInstance == Other->CostumeInstance;
	const bool SameCostumeType = CostumeType == Other->CostumeType;
	if (!SameCostumeInstance || !SameCostumeType)
	{
		return false;
	}

	/* Type별 속성 비교 */
	switch (CostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			return CostumeEquipmentSlotTag == Other->CostumeEquipmentSlotTag;
		}
	}

	return true;
}

FAGPAttachCostumeElementsContainer::FAGPAttachCostumeElementsContainer()
{

}

void FAGPAttachCostumeElementsContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (UAGPAbstractionsAvatarComponent* AttachCostumeComponent = CachedOwnerComponent.Get())
	{
		AttachCostumeComponent->OnPreReplicatedRemove(RemovedIndices, FinalSize);
	}
}

void FAGPAttachCostumeElementsContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (UAGPAbstractionsAvatarComponent* AttachCostumeComponent = CachedOwnerComponent.Get())
	{
		AttachCostumeComponent->OnPostReplicatedAdd(AddedIndices, FinalSize);
	}
}

void FAGPAttachCostumeElementsContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (UAGPAbstractionsAvatarComponent* AttachCostumeComponent = CachedOwnerComponent.Get())
	{
		AttachCostumeComponent->OnPostReplicatedChange(ChangedIndices, FinalSize);
	}
}

bool FAGPAttachCostumeElementsContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	//테스트 로그
	/*if (DeltaParms.Connection && DeltaParms.Connection->OwningActor)
	{
		AActor* ConnectionOwningActor = DeltaParms.Connection->OwningActor;
		if (ROLE_Authority != ConnectionOwningActor->GetLocalRole())
		{
			AGP_NET_LOG(ConnectionOwningActor, LogAGPCostume, Log, TEXT("OwningActor[%s]"), *GetNameSafe(DeltaParms.Connection->OwningActor));
		}
	}*/

	return FFastArraySerializer::FastArrayDeltaSerialize<FAGPAttachCostumeElement, FAGPAttachCostumeElementsContainer>(AttachCostumeElements, DeltaParms, *this);
}

void FAGPAttachCostumeElementsContainer::InitializeContainer(UAGPAbstractionsAvatarComponent* InCostumeManageComponent)
{
	CachedOwnerComponent = InCostumeManageComponent;
}




UAGPAbstractionsAvatarComponent::UAGPAbstractionsAvatarComponent()
{
	bWantsInitializeComponent = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	//ReplicateSubobjects => AddReplicatedSubObject, 테스트 필요
	bReplicateUsingRegisteredSubObjectList = false;

	SetIsReplicatedByDefault(true);
}

#pragma region[UActorComponent Interface]
bool UAGPAbstractionsAvatarComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FAGPAttachCostumeElement& AttachCostumeElement : AttachCostumeContainer.AttachCostumeElements)
	{
		UAttachCostumeBase* CostumeInstance = AttachCostumeElement.CostumeInstance;
		if (IsValid(CostumeInstance))
		{
			WroteSomething |= Channel->ReplicateSubobject(CostumeInstance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UAGPAbstractionsAvatarComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (FAGPAttachCostumeElement& AttachCostumeElement : AttachCostumeContainer.AttachCostumeElements)
		{
			UAttachCostumeBase* CostumeInstance = AttachCostumeElement.CostumeInstance;
			if (IsValid(CostumeInstance))
			{
				AddReplicatedSubObject(CostumeInstance);
			}
		}
	}
}

void UAGPAbstractionsAvatarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AttachCostumeContainer);
	DOREPLIFETIME(ThisClass, CurrentAnimLinkName);
}

void UAGPAbstractionsAvatarComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && EWorldType::EditorPreview != CurrentWorld->WorldType)
	{
		AttachCostumeContainer.InitializeContainer(this);

		/* Default Anim Link 설정 */
		if (ROLE_Authority == GetOwnerRole())
		{
			ResetAnimLinkLayer();
		}
	}
}
#pragma endregion

void UAGPAbstractionsAvatarComponent::OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FAGPAttachCostumeElement& RemoveCostumeElement = AttachCostumeContainer.AttachCostumeElements[Index];

		OnRemoveCostumeElement(RemoveCostumeElement.CostumeInstance);
	}
}

void UAGPAbstractionsAvatarComponent::OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FAGPAttachCostumeElement& AddedCostumeElement = AttachCostumeContainer.AttachCostumeElements[Index];

		/* 리플리케이션된 CostumeInstance의 속성을 CostumeInstance Type별로 설정 */
		UAttachCostumeBase* CostumeInstnace = AddedCostumeElement.CostumeInstance;
		checkf(CostumeInstnace, TEXT("%s - Invalid CostumeInstance, CostumeElement[%d]"), __FUNCTIONW__, Index);

		//Costume Instance의 식별자 지정
		CostumeInstnace->SetCostumeType(AddedCostumeElement.CostumeType);

		switch (AddedCostumeElement.CostumeType)
		{
		case EAGPCostumeType::FromEquipment:
			{
				CostumeInstnace->SetCostumeEquipmentSlot(AddedCostumeElement.CostumeEquipmentSlotTag);
			}
			break;
		}

		OnAddedCostumeElement(CostumeInstnace);
	}
}

void UAGPAbstractionsAvatarComponent::OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FAGPAttachCostumeElement& ChangeCostumeElement = AttachCostumeContainer.AttachCostumeElements[Index];

	}
}

void UAGPAbstractionsAvatarComponent::OnAddedCostumeElement(UAttachCostumeBase* InAddCostumeElement)
{
	checkf(InAddCostumeElement, TEXT("%s - Invalid InAddCostumeElement"), __FUNCTIONW__);

	AddCostumeActorToAvatar(CachedCharacterPresentationComponent.Get(), InAddCostumeElement);

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(InAddCostumeElement);
	}
}

void UAGPAbstractionsAvatarComponent::OnRemoveCostumeElement(UAttachCostumeBase* InRemoveCostumeElement)
{
	checkf(InRemoveCostumeElement, TEXT("%s - Invalid InRemoveCostumeElement"), __FUNCTIONW__);

	RemoveCostumeActorFromAvatar(CachedCharacterPresentationComponent.Get(), InRemoveCostumeElement);

	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(InRemoveCostumeElement);
	}
}

void UAGPAbstractionsAvatarComponent::AddCostumeActorToAvatar(UAGPCharPresentationComponent* CharacterPresentationComponent, UAttachCostumeBase* InAddCostumeElement)
{
	if (CharacterPresentationComponent)
	{
		CharacterPresentationComponent->AddCostumeActor(InAddCostumeElement);
	}
}

void UAGPAbstractionsAvatarComponent::RemoveCostumeActorFromAvatar(UAGPCharPresentationComponent* CharacterPresentationComponent, UAttachCostumeBase* InRemoveCostumeElement)
{
	if (CharacterPresentationComponent)
	{
		CharacterPresentationComponent->RemoveCostumeActor(InRemoveCostumeElement);
	}
}

void UAGPAbstractionsAvatarComponent::ChangeAnimationLinkLayer()
{
	if (UAGPCharPresentationComponent* CharacterPresentationComponent = CachedCharacterPresentationComponent.Get())
	{
		/* CurrentAnimLinkName - 아직 리플리케이션 받지 못해서 None인 경우에는 DefaultAnimLinkName으로 설정, 이후 리플리케이션받으면 제대로 AnimLink 설정되도록 함 */
		const FName ChangeAnimLinkName = (CurrentAnimLinkName == NAME_None) ? DefaultAnimLinkName : CurrentAnimLinkName;

		CharacterPresentationComponent->SetAnimationLink(ChangeAnimLinkName);
	}
}

void UAGPAbstractionsAvatarComponent::AddCostumeInstance(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, TSubclassOf<UAttachCostumeBase> InCostumeClass)
{
	checkf(ROLE_Authority == GetOwnerRole(), TEXT("%s - Only Call Authority"), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());
	checkf(InCostumeIdentifierHandle.IsValid(), TEXT("[%s], Invalid Costume Identifier Handle"), __FUNCTIONW__);

	UAttachCostumeBase* NewCostumeInstance = NewObject<UAttachCostumeBase>(this, InCostumeClass);
	checkf(NewCostumeInstance, TEXT("[%s] - NewCostumeInstance Invalid.."), __FUNCTIONW__);

	//Costume Instance의 식별자 지정
	const FCostumeInstanceIdentifierBase& CostumeIdentifierBase = *InCostumeIdentifierHandle.GetCostumeInstanceIdentifier().Get();
	const EAGPCostumeType NewCostumeType = CostumeIdentifierBase.GetCostumeIdentifierType();
	NewCostumeInstance->SetCostumeType(NewCostumeType);
	switch (NewCostumeType)
	{
	case EAGPCostumeType::FromEquipment:
		{
			const FCostumeInstanceEquipmentIdentifier& EquipmentIdentifier = static_cast<const FCostumeInstanceEquipmentIdentifier&>(CostumeIdentifierBase);
			NewCostumeInstance->SetCostumeEquipmentSlot(EquipmentIdentifier.GetTagEquipmentSlot());	
		}
		break;
	}

	FAGPAttachCostumeElement& AddedCostumeElement = AttachCostumeContainer.AttachCostumeElements.AddDefaulted_GetRef();
	AddedCostumeElement.SetCostumeInstance(NewCostumeInstance);

	//변경된 요소에 대해 마킹
	AttachCostumeContainer.MarkItemDirty(AddedCostumeElement);

	OnAddedCostumeElement(NewCostumeInstance);
}

void UAGPAbstractionsAvatarComponent::RemoveCostumeInstance(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle)
{
	checkf(ROLE_Authority == GetOwnerRole(), TEXT("%s - Only Call Authority"), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());
	checkf(InCostumeIdentifierHandle.IsValid(), TEXT("[%s], Invalid Costume Identifier Handle"), __FUNCTIONW__);

	/* 지울 CostumeType에 따라 조건검사 람다식 구분 */
	static TMap<EAGPCostumeType, TFunction<bool(UAttachCostumeBase* CostumeInstance, const FCostumeInstanceIdentifierBase& InCostumeIdentifier)>> CostumeIdentifierLambdaMap =
	{
		{EAGPCostumeType::FromEquipment, [](UAttachCostumeBase* CostumeInstance, const FCostumeInstanceIdentifierBase& InCostumeIdentifier)
			->bool
			{
				checkf(EAGPCostumeType::FromEquipment == InCostumeIdentifier.GetCostumeIdentifierType(), TEXT("[%s], Invalid CostumeIdentifier"), __FUNCTIONW__);
				const FCostumeInstanceEquipmentIdentifier& EquipmentIdentifier = static_cast<const FCostumeInstanceEquipmentIdentifier&>(InCostumeIdentifier);

				const bool CheckResult =
					CostumeInstance
					&& EAGPCostumeType::FromEquipment == CostumeInstance->GetCostumeType()
					&& EquipmentIdentifier.GetTagEquipmentSlot() == CostumeInstance->GetSoruceEquipmentSlotTag();

				return CheckResult;
			}
		},
	};

	const FCostumeInstanceIdentifierBase& CostumeIdentifierBase = *InCostumeIdentifierHandle.GetCostumeInstanceIdentifier().Get();
	const EAGPCostumeType CheckRemoveCostumeType = CostumeIdentifierBase.GetCostumeIdentifierType();
	TFunction<bool(UAttachCostumeBase* CostumeInstance, const FCostumeInstanceIdentifierBase& InCostumeIdentifier)>& CheckCostumeIdentifierLambda = CostumeIdentifierLambdaMap.FindChecked(CheckRemoveCostumeType);

	/* 배열 내 해당 SourceObject에 연결된 Costume Instance를 찾아서 제거 */
	const int32 CostumeElementNum = AttachCostumeContainer.AttachCostumeElements.Num();
	for (int32 ElementIndex = 0; ElementIndex < CostumeElementNum; ++ElementIndex)
	{
		UAttachCostumeBase* CostumeInstance = AttachCostumeContainer.AttachCostumeElements[ElementIndex].CostumeInstance;
		if(CheckCostumeIdentifierLambda(CostumeInstance, CostumeIdentifierBase))
		{
			OnRemoveCostumeElement(CostumeInstance);

			AttachCostumeContainer.AttachCostumeElements.RemoveAt(ElementIndex);

			//특정 요소가 지워졌으므로 Array 마킹
			AttachCostumeContainer.MarkArrayDirty();

			break;
		}
	}
}

void UAGPAbstractionsAvatarComponent::SetAnimationLinkLayer(const FName& InNewLinkLayerName)
{
	verifyf(ROLE_Authority == GetOwnerRole(), TEXT("%s - Only Call Authority"), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	CurrentAnimLinkName = InNewLinkLayerName;

	ChangeAnimationLinkLayer();
}

void UAGPAbstractionsAvatarComponent::ResetAnimLinkLayer()
{
	verifyf(ROLE_Authority == GetOwnerRole(), TEXT("%s - Only Call Authority"), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetOwnerRole()).ToString());

	SetAnimationLinkLayer(DefaultAnimLinkName);
}

void UAGPAbstractionsAvatarComponent::SetAvatarCharacter(AAGP_CharacterBase* InNewAvatarCharacter)
{
	verifyf(InNewAvatarCharacter, TEXT("[%s] - InNewAvatarCharacter Invalid"), __FUNCTIONW__);

	UAGPCharPresentationComponent* NewAvatarPresentationComponent = InNewAvatarCharacter->GetCharacterPresentationComponent();
	verifyf(NewAvatarPresentationComponent, TEXT("[%s] - NewAvatarPresentationComponent Invalid, Character[%s]"), __FUNCTIONW__, *GetNameSafe(InNewAvatarCharacter));

	CachedCharacterPresentationComponent = NewAvatarPresentationComponent;

	/* 현재 추가되어있는 AttachCostumeInstance들에 대한 Actor들을 생성한다. */
	for (FAGPAttachCostumeElement& AttachCostumeElement : AttachCostumeContainer.AttachCostumeElements)
	{
		AddCostumeActorToAvatar(NewAvatarPresentationComponent, AttachCostumeElement.CostumeInstance);
	}
	
	/* 현재 설정된 AnimLink정보를 알려준다. */
	ChangeAnimationLinkLayer();
}

void UAGPAbstractionsAvatarComponent::ResetAvatarCharacter()
{
	CachedCharacterPresentationComponent.Reset();
}

void UAGPAbstractionsAvatarComponent::OnRepCurrentAnimLinkName()
{
	ChangeAnimationLinkLayer();
}