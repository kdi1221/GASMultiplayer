// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Characters/AGP_CharacterBase.h"
#include "Common/AGPCommonStructs.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AGP_PlayerCharacter.generated.h"

class AAGP_PlayerStateBase;
class USpringArmComponent;
class UCameraComponent;
class UAGPScanFieldObjectComponent;
class AAGP_AIController;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNotifyBossStatusInfoSignature, const FAGPBossStatusInfoCommon&);

#pragma region[FAGPPerceptionNPCInfo]

/* 플레이어 캐릭터를 감지한 NPC에 대한 정보 */
USTRUCT(BlueprintType)
struct FAGPPerceptionNPCInfo : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	friend AAGP_PlayerCharacter;
	friend FAGPPerceptionNPCInfoContainer;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FAGPBossStatusInfoCommon BossStatusInfo;

public:
	FAGPPerceptionNPCInfo() = default;

public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	bool Identical(const FAGPPerceptionNPCInfo* Other, uint32 PortFlags) const;
};

template<>
struct TStructOpsTypeTraits<FAGPPerceptionNPCInfo> : public TStructOpsTypeTraitsBase2<FAGPPerceptionNPCInfo>
{
	enum
	{
		WithNetSerializer = true,
		WithIdentical = true
	};
};

#pragma endregion

#pragma region[FAGPPerceptionNPCInfoContainer]

USTRUCT(BlueprintType)
struct FAGPPerceptionNPCInfoContainer : public FFastArraySerializer
{
	GENERATED_BODY()

private:
	friend AAGP_PlayerCharacter;

private:
	UPROPERTY(VisibleInstanceOnly)
	TArray<FAGPPerceptionNPCInfo> PerceptionNPCInfos;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<AAGP_PlayerCharacter> CachedOwnerCharacter = nullptr;

public:
	FAGPPerceptionNPCInfoContainer() = default;

public:
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

public:
	void InitializeContainer(AAGP_PlayerCharacter* InOwnerCharacter);
};

template<>
struct TStructOpsTypeTraits<FAGPPerceptionNPCInfoContainer> : public TStructOpsTypeTraitsBase2<FAGPPerceptionNPCInfoContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

#pragma endregion





/**
 * 게임 내 플레이어블 캐릭터들의 베이스 클래스
 */

UCLASS()
class ACTIONGAMEPROJECT_API AAGP_PlayerCharacter : public AAGP_CharacterBase
{
	GENERATED_BODY()
	
private:
	friend FAGPPerceptionNPCInfoContainer;
	
#pragma region [Components]
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FieldObject")
	TObjectPtr<UAGPScanFieldObjectComponent> ScanFieldObjectComponent;
#pragma endregion

private:
	/* 플레이어 캐릭터를 인식한 AI컨트롤러 및 Pawn들에 대한 정보 */
	UPROPERTY(VisibleInstanceOnly, Replicated)
	FAGPPerceptionNPCInfoContainer PerceptionNPCInfoContainer;

private:
	//ASC 및 Attribute는 PlayerState에서 관리되고 있으므로 여기서는 약참조로 캐싱한다.(소멸 과정에서 원할한 GC 처리를 위함)
	TWeakObjectPtr<UAGP_AbilitySystemComponent> CachedAbilitySystemComponent;
	TWeakObjectPtr<UAGPAttributeSetCommon> CachedCommonAttributeSet;
	TWeakObjectPtr<AAGP_PlayerStateBase> CachedCurrentPlayerState;

public:
	FOnNotifyBossStatusInfoSignature OnAddedBossStatus;
	FOnNotifyBossStatusInfoSignature OnRemoveBossStatus;
	FOnNotifyBossStatusInfoSignature OnModifyBossStatus;

#pragma region [Constructor]
public:
	AAGP_PlayerCharacter();
#pragma endregion


#pragma region[Parent Class Interface]
protected:
	/* Pawn과 연동되는 PlayerState가 변경될때 호출, 여기서 AbilitySystem 초기화 */
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

public:
	virtual void DetachFromControllerPendingDestroy() override;
	virtual void SetPlayerDefaults() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
#pragma endregion

#pragma region[AAGP_CharacterBase Interface]
protected:
	virtual void InitializeState() override;
	virtual void InitailizeCharacterWidget() override;

public:
	virtual UAGP_AbilitySystemComponent* GetAGPAbilitySystemComponent() const override;
	virtual UAGPAttributeSetCommon* GetCommonAttributeSet() const override;
#pragma endregion

#pragma region[IAGPItemSystemExternalInterface]
public:
	virtual UAGPItemSystemsFacadeComponent* GetItemSystemFacadeComponent() const override;
#pragma endregion

#pragma region[IGenericTeamAgentInterface]
public:
	virtual FGenericTeamId GetGenericTeamId() const override;
#pragma endregion

public:
	/* 다른 NPC(보스몬스터)로부터 플레이어에 대한 감지 이벤트 수신*/
	void OnPerceptionByNPC(bool bIsPerception, AAGP_AIController* AIController);

	/* 플레이어를 감지한 NPC(보스몬스터)로부터 수신되는 상태(체력) 변경 업데이트 */
	void OnPerceptionNPCChangeHealth(AAGP_AIController* AIController, float CurrentHealth, float MaxHealth);

	/* BossStatus마다 작업 실행 */
	void ForEachBossStatus_Execute(const FOnNotifyBossStatusInfoSignature::FDelegate& InDelegate) const;

private:
	void OnPossesPlayerController();
	void OnUnpossessPlayerController();

	void OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	void OnAddedNPCPerceptionInfo(const FAGPPerceptionNPCInfo& AddedNPCInfo);
	void OnModifyNPCPerceptionInfo(const FAGPPerceptionNPCInfo& ChangedNPCInfo);
	void OnRemovedNPCPerceptionInfo(const FAGPPerceptionNPCInfo& RemovedNPCInfo);

public:
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return FollowCamera; }
	FORCEINLINE UAGPScanFieldObjectComponent* GetScanFieldObjectComponent() const { return ScanFieldObjectComponent; }

public:
	UFUNCTION(BlueprintPure)
	const TArray<FAGPPerceptionNPCInfo>& GetPerceptionNPCInfo() const {return PerceptionNPCInfoContainer.PerceptionNPCInfos;}
};
