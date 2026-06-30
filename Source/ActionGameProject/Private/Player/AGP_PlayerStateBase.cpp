// KJY All Rights Reserved


#include "Player/AGP_PlayerStateBase.h"

#include "Characters/AGP_CharacterBase.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "Item/Components/AGPInventoryComponent.h"
#include "Item/Components/AGPEquipmentSlotsComponent.h"
#include "Item/Components/AGPAbstractionsAvatarComponent.h"
#include "Item/Components/AGPItemSystemsFacadeComponent.h"
#include "Item/FunctionLibrary/AGPItemFunctionLibrary.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"
#include "Common/AGPCommonEnums.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

AAGP_PlayerStateBase::AAGP_PlayerStateBase()
{
	//플레이어 캐릭터의 경우 플레이어 상태 내에 AbilitySystemComponent를 생성한다. 
	//게임 진행 종류에 따라 플레이어의 캐릭터(Avatar)는 여러번 바뀔수가 있으며, 이 때 상태 정보등을 공유하기 위해 여기에 생성한다.
	//플레이어 캐릭터의 경우 네트워크를 통해 서버와 연결되므로(AutonomousProxy) Mixed를 설정한다.
	AbilitySystemComponent = CreateDefaultSubobject<UAGP_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CommonAttributeSet = CreateDefaultSubobject<UAGPAttributeSetCommon>(TEXT("AttributeSetCommon"));

	//인벤토리 관리 컴포넌트 생성
	InventoryComponent = CreateDefaultSubobject<UAGPInventoryComponent>(TEXT("InventoryComponent"));

	//장비슬롯 관리 컴포넌트 생성
	EquipmentComponent = CreateDefaultSubobject<UAGPEquipmentSlotsComponent>(TEXT("EquipmentComponent"));

	//Possess될 Pawn에대한 AbstractionsAvatarComponent 생성
	AbstractionsAvatarComponent = CreateDefaultSubobject<UAGPAbstractionsAvatarComponent>(TEXT("AbstractionsAvatarComponent"));

	//아이템 시스템 파서드 패턴 컴포넌트 생성
	ItemSystemFacadeComponent = CreateDefaultSubobject<UAGPItemSystemsFacadeComponent>(TEXT("ItemSystemFacadeComponent"));

	//해당 Actor의 초당 리플리케이션 횟수
	//PlayerState의 기본 넷 업데이트 빈도는 1로 설정되어 있기때문에 
	//PlayerState에 AbilitySystemComponet와 AttributeSet를 사용하기위해 업데이트 주기를 다음과 같이 설정
	//NetUpdateFrequency = 100.f;
	SetNetUpdateFrequency(100.f);

	//TeamGroupID 초기화
	PlayerTeamGroupID = FGenericTeamId(static_cast<uint8>(EAGPTeamGroup::PlayerGroup_1));

}

#pragma region[IAbilitySystemInterface Interface]
UAbilitySystemComponent* AAGP_PlayerStateBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
#pragma endregion

#pragma region[IAGPItemSystemExternalInterface]
UAGPItemSystemsFacadeComponent* AAGP_PlayerStateBase::GetItemSystemFacadeComponent() const
{
	return ItemSystemFacadeComponent;
}
#pragma endregion

#pragma region[IGenericTeamAgentInterface]
void AAGP_PlayerStateBase::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	PlayerTeamGroupID = TeamID;
}

FGenericTeamId AAGP_PlayerStateBase::GetGenericTeamId() const
{
	return PlayerTeamGroupID;
}
#pragma endregion

void AAGP_PlayerStateBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//AGP_NET_LOG(this, LogAGPCharacter, Log, TEXT("[%s]"), *GetNameSafe(this));

	verifyf(AbilitySystemComponent, TEXT("PlayerState AbilitySystemComponent Invalid"));
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());

	/* 아이템 시스템 파서드 패턴 컴포넌트 초기화 */
	verifyf(ItemSystemFacadeComponent, TEXT("PlayerState ItemSystemFacadeComponent Invalid"));
	ItemSystemFacadeComponent->SetInventoryComponent(InventoryComponent);
	ItemSystemFacadeComponent->SetEquipmentSlotsComponent(EquipmentComponent);
	ItemSystemFacadeComponent->SetAbstractionsAvatarComponent(AbstractionsAvatarComponent);
}

/* 플레이어 초기 장비 셋팅 */
void AAGP_PlayerStateBase::InitializePlayerItems()
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

void AAGP_PlayerStateBase::InitializePlayerAbilities()
{
	checkf(AbilitySystemComponent, TEXT("PlayerState AbilitySystemComponent Invalid"));
	AbilitySystemComponent->GrantBaseAbility();
	AbilitySystemComponent->GrantLocalBaseFeatureAbility();
}

void AAGP_PlayerStateBase::SetAvatarCharacter(AAGP_CharacterBase* InAvatarCharacter)
{
	verifyf(InAvatarCharacter, TEXT("[%s], Invalid InAvatarCharacter"), __FUNCTIONW__);

	AbstractionsAvatarComponent->SetAvatarCharacter(InAvatarCharacter);
}

void AAGP_PlayerStateBase::ResetAvatarCharacter()
{
	verifyf(AbstractionsAvatarComponent, TEXT("[%s], Invalid AbstractionsAvatarComponent"), __FUNCTIONW__);

	AbstractionsAvatarComponent->ResetAvatarCharacter();
}

void AAGP_PlayerStateBase::SetCurrentStateInSurvivalMode(const EAGPSurvivalModePlayerState InNewState)
{
	CurrentStateInSurvivalMode = InNewState;

	OnStateInSurvivalModeChanged.Broadcast(this, CurrentStateInSurvivalMode);
}
