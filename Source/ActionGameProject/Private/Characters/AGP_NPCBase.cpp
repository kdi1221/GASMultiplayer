// KJY All Rights Reserved


#include "Characters/AGP_NPCBase.h"
#include "Characters/Components/AGPCharacterMovementComponent.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "Item/Components/AGPInventoryComponent.h"
#include "Item/Components/AGPEquipmentSlotsComponent.h"
#include "Item/Components/AGPAbstractionsAvatarComponent.h"
#include "Item/Components/AGPItemSystemsFacadeComponent.h"
#include "AI/AGP_AIController.h"
#include "Log/AGPLogChannels.h"


AAGP_NPCBase::AAGP_NPCBase()
{
	//NPC의 경우 캐릭터 내에 AbilitySystemComponent를 생성한다. 
	//Replicate Mode 설정 - NPC의 경우 서버에서만 Effect정보를 가지고 있으면 되므로 Minimal를 설정한다.
	AbilitySystemComponent = CreateDefaultSubobject<UAGP_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	CommonAttributeSet = CreateDefaultSubobject<UAGPAttributeSetCommon>(TEXT("CommonAttributeSet"));

	//인벤토리 관리 컴포넌트 생성
	InventoryComponent = CreateDefaultSubobject<UAGPInventoryComponent>(TEXT("InventoryComponent"));

	//장비슬롯 관리 컴포넌트 생성
	EquipmentComponent = CreateDefaultSubobject<UAGPEquipmentSlotsComponent>(TEXT("EquipmentComponent"));

	//Possess될 Pawn에대한 AbstractionsAvatarComponent 생성
	AbstractionsAvatarComponent = CreateDefaultSubobject<UAGPAbstractionsAvatarComponent>(TEXT("AbstractionsAvatarComponent"));

	//아이템 시스템 파서드 패턴 컴포넌트 생성
	ItemSystemFacadeComponent = CreateDefaultSubobject<UAGPItemSystemsFacadeComponent>(TEXT("ItemSystemFacadeComponent"));

	//초기 AI Possess 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	/* MovementComponent 설정 */
	checkf(CharacterMovementComponent, TEXT("Constructor MovementComponent Invalid.."));
	CharacterMovementComponent->bUseControllerDesiredRotation = false;
	CharacterMovementComponent->bOrientRotationToMovement = true;
	CharacterMovementComponent->RotationRate = FRotator(0.f, 180.f, 0.f);
	CharacterMovementComponent->MaxWalkSpeed = 300.f;
	CharacterMovementComponent->BrakingDecelerationWalking = 1000.f;
}

void AAGP_NPCBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld && EWorldType::EditorPreview != CurrentWorld->WorldType)
	{
		//AGP_NET_LOG(this, LogAGPEnemy, Log, TEXT("Initialize ASC For NPC"));

		/* 아이템 시스템 파서드 패턴 컴포넌트 초기화 */
		verifyf(ItemSystemFacadeComponent, TEXT("[%s] - ItemSystemFacadeComponent Invalid"), __FUNCTIONW__);
		ItemSystemFacadeComponent->SetInventoryComponent(InventoryComponent);
		ItemSystemFacadeComponent->SetEquipmentSlotsComponent(EquipmentComponent);
		ItemSystemFacadeComponent->SetAbstractionsAvatarComponent(AbstractionsAvatarComponent);

		//NPC의 경우 여기서 Costume Component 설정
		AbstractionsAvatarComponent->SetAvatarCharacter(this);

		//NPC의 경우 Owner를 자기 자신으로 설정
		InitializeAbilitySystem(this);

		/* Character Widget(HeadUP) 초기화 */
		InitailizeCharacterWidget();
	}
}

void AAGP_NPCBase::BeginPlay()
{
	Super::BeginPlay();

	if (ROLE_Authority == GetLocalRole())
	{
		/* NPC가 가지고 있을 아이템들 초기화 */
		InitializeItems();

		/* 초기 NPC가 가지고 있을 Ability등 초기화 */
		InitializeAbility();

		/* Attribute 초기화 */
		InitializeAttribute();

		/* State 초기화 */
		InitializeState();

		/* 초기 Ability 활성화 */
		InitializeActivateAbility();

		/* 초기화된 Attribute 갱신(Widget, Replicate) */
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->ForceReplication();
		}

		SetFloatingWidgetVisiblityIfAlive();
	}
}

UAGP_AbilitySystemComponent* AAGP_NPCBase::GetAGPAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAGPAttributeSetCommon* AAGP_NPCBase::GetCommonAttributeSet() const
{
	return CommonAttributeSet;
}

#pragma region[IAGPItemSystemExternalInterface]
UAGPItemSystemsFacadeComponent* AAGP_NPCBase::GetItemSystemFacadeComponent() const
{
	return ItemSystemFacadeComponent;
}
#pragma endregion

#pragma region[IGenericTeamAgentInterface]
FGenericTeamId AAGP_NPCBase::GetGenericTeamId() const
{
	/* AIController의 Team 반환 */
	if (IGenericTeamAgentInterface* ControllerTeamInterface = Cast<IGenericTeamAgentInterface>(GetController()))
	{
		return ControllerTeamInterface->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}
#pragma endregion

void AAGP_NPCBase::InitializeItems()
{
	verifyf(ItemSystemFacadeComponent, TEXT("InitializePlayerItems - ItemSystemFacadeComponent Invalid.."));
	verifyf(ROLE_Authority == GetLocalRole(), TEXT("[%s] - Only Authority Call. Role[%s]"), __FUNCTIONW__, *UEnum::GetDisplayValueAsText(GetLocalRole()).ToString());

	/* 기본 장비 아이템 초기화 */
	for (auto& EquipmentItem : InitializeEquipments)
	{
		const FGameplayTag& InitEquipmentSlot = EquipmentItem.Key;
		TSubclassOf<UAGPItemInstance> InitEquipmentItemClass = EquipmentItem.Value;

		ItemSystemFacadeComponent->InitializeItemEquipment(InitEquipmentItemClass, InitEquipmentSlot);
	}

	/* 기본적으로 인벤토리에 소유할 아이템 */
	int32 SlotIndex = 0;
	for (const FInitializeInventoryItemData& DefaultItemData : InitializeInventoryItems)
	{
		TSubclassOf<UAGPItemInstance> NewItemClass = DefaultItemData.AddItemClass;
		const int32 NewItemStackNum = DefaultItemData.ItemNum;

		ItemSystemFacadeComponent->InitializeItemToInventory(NewItemClass, SlotIndex, NewItemStackNum);

		++SlotIndex;
	}
}

void AAGP_NPCBase::InitializeAbility()
{
	checkf(AbilitySystemComponent, TEXT("[%s], AbilitySystemComponent Invalid"), __FUNCTIONW__);

	AbilitySystemComponent->GrantBaseAbility();
}
