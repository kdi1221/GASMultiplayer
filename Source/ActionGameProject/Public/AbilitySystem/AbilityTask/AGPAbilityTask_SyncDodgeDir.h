// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AGPAbilityTask_SyncDodgeDir.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSyncDodgeDirectionSignature, const FVector&, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInvalidDodgeDirectionSignature);



/* SyncDodgeDir에서 사용할 Target Data */
USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_DodgeDirection : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	/** 클라이언트에서 보내온 Dodge 방향 */
	UPROPERTY()
	FVector_NetQuantizeNormal DodgeDirection;

	FGameplayAbilityTargetData_DodgeDirection()
	{ }

	FGameplayAbilityTargetData_DodgeDirection(const FVector& InDirection)
		:DodgeDirection(InDirection.GetSafeNormal())
	{ }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_DodgeDirection::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_DodgeDirection> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_DodgeDirection>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

/**
 * Dodge Ability 사용시 Client => Server 방향 동기화(플레이어 캐릭터 전용)
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPAbilityTask_SyncDodgeDir : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FSyncDodgeDirectionSignature DodgeDirectionSync;

	UPROPERTY(BlueprintAssignable)
	FInvalidDodgeDirectionSignature InvalidDodgeDirection;
	
public:
	UFUNCTION(BlueprintCallable, Category = "AGP|AbilityTasks", meta = (DisplayName = "SyncDodgeDirection", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAGPAbilityTask_SyncDodgeDir* CreateSyncDodgeDirection(UGameplayAbility* OwningAbility);
	
private:
	virtual void Activate() override;

private:
	void OnActiveClient();
	void OnActiveServer();

	void NotifyDodgeDirection(const FVector& InDodgeDirection);
	void NotifyInvalidDodgeDirection();

	void OnReplicatedDodgeDirectionDataCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
