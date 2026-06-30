// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagContainer.h"
#include "Costume/AGPCostumeEnum.h"
#include "AGPAbstractionsAvatarComponent.generated.h"

struct FCostumeInstanceIdentifierHandle;

class UAttachCostumeBase;
class AAGPCostumeActorBase;
class UAGPCharPresentationComponent;
class AAGP_CharacterBase;

USTRUCT(BlueprintType)
struct FAGPAttachCostumeElement : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	friend UAGPAbstractionsAvatarComponent;
	friend FAGPAttachCostumeElementsContainer;

private:
	UPROPERTY(VisibleInstanceOnly, Instanced)
	TObjectPtr<UAttachCostumeBase> CostumeInstance = nullptr;

	UPROPERTY(VisibleInstanceOnly)
	EAGPCostumeType CostumeType = EAGPCostumeType::None;

	UPROPERTY(VisibleInstanceOnly)
	FGameplayTag CostumeEquipmentSlotTag;

public:
	FAGPAttachCostumeElement();

public:
	void SetCostumeInstance(UAttachCostumeBase* InCostumeInstance);

public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	bool Identical(const FAGPAttachCostumeElement* Other, uint32 PortFlags) const;
};

template<>
struct TStructOpsTypeTraits<FAGPAttachCostumeElement> : public TStructOpsTypeTraitsBase2<FAGPAttachCostumeElement>
{
	enum
	{
		WithNetSerializer = true,
		WithIdentical = true
	};
};

USTRUCT(BlueprintType)
struct FAGPAttachCostumeElementsContainer : public FFastArraySerializer
{
	GENERATED_BODY()

private:
	friend UAGPAbstractionsAvatarComponent;

private:
	UPROPERTY(VisibleInstanceOnly)
	TArray<FAGPAttachCostumeElement> AttachCostumeElements;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UAGPAbstractionsAvatarComponent> CachedOwnerComponent;

public:
	FAGPAttachCostumeElementsContainer();

public:
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

public:
	void InitializeContainer(UAGPAbstractionsAvatarComponent* InCostumeManageComponent);
};

template<>
struct TStructOpsTypeTraits<FAGPAttachCostumeElementsContainer> : public TStructOpsTypeTraitsBase2<FAGPAttachCostumeElementsContainer>
{
	enum 
	{ 
		WithNetDeltaSerializer = true 
	};
};




/* AvatarPawn에 대한 추상적인 접근을 위한 컴포넌트 */
/* PlayerState, PlayerController와 같이 소유한 Pawn이 Possess, Unpossess동작에 따라 소유한 Pawn이 없을수도 있다. */
/* 그에 상관없이 아이템 및 Costume의 기능 및 정보를 저장할 수 있게해서, 이후에 새롭게 Possess되는 Avatar Pawn에 설정될 수 있도록 한다.*/
UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPAbstractionsAvatarComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	friend FAGPAttachCostumeElementsContainer;

private:
	UPROPERTY(VisibleInstanceOnly, Replicated, meta = (ShowOnlyInnerProperties))
	FAGPAttachCostumeElementsContainer AttachCostumeContainer;

	/* 현재 지정된 Animation Link 이름 */
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRepCurrentAnimLinkName, Category = "Animation")
	FName CurrentAnimLinkName = NAME_None;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	FName DefaultAnimLinkName;

private:
	TWeakObjectPtr<UAGPCharPresentationComponent> CachedCharacterPresentationComponent;

public:	
	UAGPAbstractionsAvatarComponent();

#pragma region[UActorComponent Interface]
public:
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;
#pragma endregion

private:
	void OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

private:
	void OnAddedCostumeElement(UAttachCostumeBase* InAddCostumeElement);
	void OnRemoveCostumeElement(UAttachCostumeBase* InRemoveCostumeElement);

	void AddCostumeActorToAvatar(UAGPCharPresentationComponent* CharacterPresentationComponent, UAttachCostumeBase* InAddCostumeElement);
	void RemoveCostumeActorFromAvatar(UAGPCharPresentationComponent* CharacterPresentationComponent, UAttachCostumeBase* InRemoveCostumeElement);

	void ChangeAnimationLinkLayer();



public:
	void AddCostumeInstance(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, TSubclassOf<UAttachCostumeBase> InCostumeClass);
	void RemoveCostumeInstance(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle);

	void SetAnimationLinkLayer(const FName& InNewLinkLayerName);
	void ResetAnimLinkLayer();

	void SetAvatarCharacter(AAGP_CharacterBase* InNewAvatarCharacter);
	void ResetAvatarCharacter();

private:
	UFUNCTION()
	void OnRepCurrentAnimLinkName();
};
