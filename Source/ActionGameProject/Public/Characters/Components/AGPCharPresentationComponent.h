// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Costume/AGPCostumeStruct.h"
#include "Common/AGPCommonStructs.h"
#include "AbilitySystem/AGPAbilityTypes.h"
#include "AGPCharPresentationComponent.generated.h"

struct FCostumeInstanceIdentifierBase;

class UAttachCostumeBase;
class AAGPCostumeActorBase;
class UAGPLinkedAnimLayerBase;
class UAnimMontage;
class UAGPCharacterAnimInstance;
class UAGP_AbilitySystemComponent;


/* Owner 캐릭터의 CostumeActor, Animation들을 관리 */
/* 1. CostumeActor의 생성 및 생성된 Actor들 관리 */
/* 2. Animation Montage, AnimLink 관리 */
/* 3. Animation Instance에서 참고할 State들에 대한 정보들 저장 및 관리 */
/* 4. Animation Refresh Bone 설정(Tag가 설정된 Ability가 활성화중일때 해당 기능 활성화) */

UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPCharPresentationComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	/* 추상화된 소켓과 실제 Mesh의 소켓 맵핑 */
	UPROPERTY(EditDefaultsOnly, meta = (ForceInlineRow))
	TMap<FName, FName> CostumeAttachSocketMapping;

	/* Link될 AnimLayer Instance Class */
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UAGPLinkedAnimLayerBase>> LinkAnimLayerMap;

	/* Owner Pawn이 가지고 있는 Montage들 */
	UPROPERTY(EditDefaultsOnly, meta = (ForceInlineRow))
	TMap<FName, TObjectPtr<UAnimMontage>> OwnerPawnMontageMap;

	/* HitReact시 동작 결정(몽타주 플레이) */
	UPROPERTY(EditDefaultsOnly)
	EAGPHitReactPlayMontageType HitReactMontagePlayType = EAGPHitReactPlayMontageType::NotPlayMontage;

	/* HitReact시 동작 결정(기타 기능들) */
	UPROPERTY(EditDefaultsOnly, meta = (Bitmask, BitmaskEnum = "/Script/ActionGameProject.EAGPHitReactFlags"))
	uint8 HitReactFlag;

	/* HitReact Montage Map(Random) */
	UPROPERTY(EditDefaultsOnly, meta = (ForceInlineRow, EditCondition = "HitReactMontagePlayType == EAGPHitReactPlayMontageType::RandomMontage", EditConditionHides))
	TMap<FName, TObjectPtr<UAnimMontage>> HitReactRandomMontageMap;

	UPROPERTY(EditDefaultsOnly, meta = (ForceInlineRow, EditCondition = "HitReactMontagePlayType == EAGPHitReactPlayMontageType::DirectionalMontage", EditConditionHides))
	TMap<EAGPDirection, TObjectPtr<UAnimMontage>> HitReactDirectionMontageMap = 
	{
		{EAGPDirection::Front, nullptr},
		{EAGPDirection::Back, nullptr},
		{EAGPDirection::Left, nullptr},
		{EAGPDirection::Right, nullptr}
	};

	/* Death 상태에서의 동작 결정(기타 기능들) */
	UPROPERTY(EditDefaultsOnly, meta = (Bitmask, BitmaskEnum = "/Script/ActionGameProject.EAGPDeathFlags"))
	uint8 DeathFlag;	

	/* Death Pose Map */
	UPROPERTY(EditDefaultsOnly, meta = (ForceInlineRow))
	TMap<FName, FAGPDeathPoseAnimMontages> DeathMontagePoseMap;

	/* ASC로부터 추가 및 삭제되는 것에 대한 이벤트 콜백을 받을 Animation 상태에 관련된 GameplayTag들 */
	UPROPERTY(EditDefaultsOnly)
	TSet<FGameplayTag> RegisterCallbackGameplayTags;
	
protected:
	/* RegisterCallbackGameplayTags들 중 현재 Owner에 설정된 GameplayTag들 */
	UPROPERTY(VisibleInstanceOnly)
	FGameplayTagContainer OwnedGameplayTags;

	/* 현재 캐릭터 SkeletalMeshComponent에 AnimTick RefreshBone설정 여부 */
	UPROPERTY(VisibleInstanceOnly)
	bool IsApplyAnimTickRefreshBone = false;

	/* 현재 캐릭터 AnimInstance에 설정된 AnimLink */
	UPROPERTY(VisibleInstanceOnly)
	FName CurrentAnimLinkName;

private:
	TWeakObjectPtr<APawn> CachedCostumeActorInstigator;
	TWeakObjectPtr<USkeletalMeshComponent> CachedOwnerSkeletalMeshComponent;

	/* EAGPCostumeType::FromEquipment인 CostumeActor들 */	
	TMap<FAGPCostumeEquipmentMapKey, TWeakObjectPtr<AAGPCostumeActorBase>> AttachedCostumeActorFromEquipmentMap;

	TWeakObjectPtr<UAGPCharacterAnimInstance> CachedOwnerAnimInstance;

public:	
	UAGPCharPresentationComponent();

#pragma region[UActorComponent Interface]
public:
	virtual void InitializeComponent() override;
#pragma endregion

public:
	void SetCostumeActorInstigator(APawn* InInstigatorPawn);
	void SetOwnerSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComponent);

	void AddCostumeActor(UAttachCostumeBase* InAddCostumeElement);
	void RemoveCostumeActor(const UAttachCostumeBase* InRemoveCostumeElement);
	void AllDestroyAttachCostumeActors();

	void SetAnimationLink(const FName& InNewAnimLinkName);
	void InitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC);
	void UninitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC);

	/* 외부로부터(EX : AnimNotify) 특정 CostumeActor의 상태 변경 */
	void SetCostumeActorStatus(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, const FName& InCostumeActorName, const FName& InActorStatus);

	/* Owner Chacter의 Hit Event 수신, CostumeActor들의 Hit FX 표시 */
	void OnOwnerHitEvent();

	/* Owner Character의 DeathDestroy 이벤트 수신, CostumeActor들에 대한 Dissolve FX 표시 */
	void OnOwnerDeathDestroy(const bool IsShowDissolveFX);

public:
	/* 현재 상태에 대한 Tag가 있는지 체크 */
	bool HasOwnedGameplayTag(const FGameplayTag& InTagToCheck) const;

	/* Key에 대응되는 몽타주 반환 */
	UAnimMontage* GetMontageWithKey(const FName& InMontageKey) const;

	/* Key에 대응되는 Random HitReact 몽타주 반환 */
	UAnimMontage* GetRandomHitReactMontageWithKey(const FName& InMontageKey) const;

	/* 현재 HitReact 몽타주들 중 랜덤하게 선택된 Key값 반환 */
	FName GetRandomHitReactMontageKey() const;

	/* Direction에 대응되는 Direction HitReact 몽타주 반환 */
	UAnimMontage* GetDirectionHitReactMontage(const EAGPDirection InDirection) const;

	/* 현재 Death Pose들 중 랜덤하게 선택된 값 반환 */
	FName GetRandomDeathPoseName() const;

	/* AvatarCharacter의 Death 몽타주 반환 */
	UAnimMontage* GetDeathMontageFromDeathPose(const FName& InDeathPoseName) const;

	/* AvatarCharacter의 Rebirth 몽타주 반환 */
	UAnimMontage* GetRebirthMontageFromDeathPose(const FName& InDeathPoseName) const;

	/* 추상화된 소켓이름으로부터 실제 메시에 추가되어있는 소켓 이름 반환 */
	const FName& GetSpecificSocketName(const FName& InAbstractSocketName) const;

	/* Owner의 SkeletalMeshComponent 반환(Cached) */
	USkeletalMeshComponent* GetOwenrSkeletalMeshComponent() const;

	/* 설정된 HitReact Flag와 비교해서 설정여부 반환 */
	bool CheckHitReactFlag(int32 InBitMask);

	/* 설정된 DeathFlag와 비교해서 설정여부 반환 */
	bool CheckDeathFlag(int32 InBitMask);

	/* 현재 붙어있는 CostumeActor 반환 */
	AAGPCostumeActorBase* GetAttachedCostumeActor(const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, const FName& InCostumeActorName);

private:
	/* CurrentAnimLinkName가 변경될때 호출되는 내부 함수 */
	void OnChangeAnimationLink();

	/* Owner의 ASC로부터 Character.State의 Tag들의 추가 및 삭제에 대한 콜백 호출 */
	void OnAddOrRemovedGameplayTags(const FGameplayTag Tag, int32 NewCount);

	/* Owner의 ASC로부터 Animation.SetProperty.AnimRefreshBone의 추가 및 삭제에 대한 콜백 호출 */
	void OnAnimRefreshBoneTagChanged(const FGameplayTag Tag, int32 NewCount);

	/* GameplayTag의 추가, 삭제를 각 CostumeActor에 알림 */
	void NotifyGameplayTagEventToAttachCostumeActors(const FGameplayTag& Tag, const bool IsAdded);

	/* Animation.SetProperty.AnimRefreshBone의 추가/삭제에 따른 SkeletalMeshComponent의 속성 조정 */
	void OnChangeOwnerSkeletalMeshAnimRefreshBone();

private:
	UFUNCTION()
	void OnDestroyOwnerCallback(AActor* InDestryoedOwner);

public:
	FORCEINLINE EAGPHitReactPlayMontageType GetHitReactMontageType() const { return HitReactMontagePlayType; }
};
