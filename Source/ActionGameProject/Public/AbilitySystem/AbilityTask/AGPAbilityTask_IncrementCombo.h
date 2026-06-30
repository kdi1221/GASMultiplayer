// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AGPAbilityTask_IncrementCombo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FComboIncrementSuccessSignature, int32, ComboCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FComboMaxLimitSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FClientComboCountInvalidSignature);


/*
	Client => Server로 검증할 ComboCount를 보내기 위한 구조체 
*/
USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_NextCombo : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()
	
	/** 클라이언트에서 보내온 증가된 Combo Count */
	UPROPERTY()
	int32	NextComboCount = 0;

	FGameplayAbilityTargetData_NextCombo()
	{ }

	FGameplayAbilityTargetData_NextCombo(const int32 ComboCount)
		: NextComboCount(ComboCount)
	{ }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_NextCombo::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_NextCombo> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_NextCombo>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

/*
	Combo Ability 내부에서 지정한 Combo Count 증가
	클라이언트에서 보내온 Combo Count 검증
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPAbilityTask_IncrementCombo : public UAbilityTask
{
	GENERATED_BODY()
	
private:
	int32 CurrentComboCount = 0;
	int32 NextComboCount = 0;
	int32 MaxComboCount = 0;

public:
	UPROPERTY(BlueprintAssignable)
	FComboIncrementSuccessSignature IncrementSuccess;

	UPROPERTY(BlueprintAssignable)
	FComboMaxLimitSignature ComboMaxLimit;

	UPROPERTY(BlueprintAssignable)
	FClientComboCountInvalidSignature ClientComoboCountInvalid;
	
public:
	UFUNCTION(BlueprintCallable, Category = "AGP|AbilityTasks", meta = (DisplayName = "IncrementCombo", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAGPAbilityTask_IncrementCombo* CreateIncrementCombo(UGameplayAbility* OwningAbility, int32 ComboCount, int32 MaxCount);

private:
	virtual void Activate() override;

private:
	void IncrementComboCount();

	void NotifyIncrementAfter();
	void NotifyClientComboCountInvalid();

	void OnActiveClient();
	void OnActiveServer();

	//서버에서 클라이언트가 보내온 Combo Data를 수신받았을때 호출되는 콜백함수
	void OnReplicatedComboDataCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
