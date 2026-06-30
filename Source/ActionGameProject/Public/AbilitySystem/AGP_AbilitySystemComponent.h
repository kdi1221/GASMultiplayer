// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AbilitySystem/AGPAbilityTypes.h"
#include "AGP_AbilitySystemComponent.generated.h"

class UAGPAttributeSetBase;
struct FInputActionValue;
struct FGameplayTag;

DECLARE_DELEGATE_OneParam(FAGPForEachAbility, FGameplayAbilitySpec&);
DECLARE_DELEGATE_OneParam(FAGPForEachAttributeSet, UAGPAttributeSetBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCustomAttributeSetChanged, UAGPAttributeSetBase*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityAssignChanged, FGameplayAbilitySpec&);


#pragma region[FAGPDynamicAttributeSetElement]

/* 동적으로 추가되는 AttributeSet에 대한 FastArray 요소 */
USTRUCT(BlueprintType)
struct FAGPDynamicAttributeSetElement : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	friend UAGP_AbilitySystemComponent;
	friend FAGPDynamicAttributeSetElementContainer;

private:
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UAGPAttributeSetBase> AttributeSet = nullptr;

public:
	FAGPDynamicAttributeSetElement() = default;

public:
	void SetAttributeSetInstance(UAGPAttributeSetBase* InAddAttributeSet);

public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	bool Identical(const FAGPDynamicAttributeSetElement* Other, uint32 PortFlags) const;
};

template<>
struct TStructOpsTypeTraits<FAGPDynamicAttributeSetElement> : public TStructOpsTypeTraitsBase2<FAGPDynamicAttributeSetElement>
{
	enum
	{
		WithNetSerializer = true,
		WithIdentical = true
	};
};

#pragma endregion

#pragma region[FAGPDynamicAttributeSetElementContainer]

USTRUCT(BlueprintType)
struct FAGPDynamicAttributeSetElementContainer : public FFastArraySerializer
{
	GENERATED_BODY()

private:
	friend UAGP_AbilitySystemComponent;

private:
	UPROPERTY(VisibleInstanceOnly)
	TArray<FAGPDynamicAttributeSetElement> AttributeSetElements;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UAGP_AbilitySystemComponent> CachedOwnerComponent;

public:
	FAGPDynamicAttributeSetElementContainer() = default;

public:
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

public:
	void SetOwnerComponent(UAGP_AbilitySystemComponent* InOwnerASC);
};

template<>
struct TStructOpsTypeTraits<FAGPDynamicAttributeSetElementContainer> : public TStructOpsTypeTraitsBase2<FAGPDynamicAttributeSetElementContainer>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

#pragma endregion

/**
 *  
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGP_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
private:
	friend FAGPDynamicAttributeSetElementContainer;
	
protected:
	/* 로컬 전용 BaseFeature Ability(나중에 PlayerState쪽으로 옮겨야할려나) */
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UAGPGameplayAbility>> LocalBaseFeatureAbilityClasses;

	/* 기본적으로 부여되는 Base Ability */
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UAGPGameplayAbility>> GrantBaseAbilityClasses;

	/* 기본적으로 부여되는 Activate Ability */
	UPROPERTY(EditDefaultsOnly)
	TArray<FAGPGrantAbilitySet> GrantActivateAbilityClasses;

private:
	UPROPERTY(Replicated, Transient)
	FAGPDynamicAttributeSetElementContainer AddDynamicAttributeSetContainer;

public:
	/* Custom AttributeSet 추가될때 호출 */
	FOnCustomAttributeSetChanged OnCustomAttributeSetAdded;

	/* Custom AttributeSet 제거될때 호출 */
	FOnCustomAttributeSetChanged OnCustomAttributeSetRemoved;

	/* 특정 Ability가 부여될때 호출 */
	FOnAbilityAssignChanged OnAbilityGranted;

	/* 특정 Ability가 제거될때 호출 */
	FOnAbilityAssignChanged OnAbilityRemoved;


public:
	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	/* 입력 처리 */
public:
	void OnAbilityInput_Started(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	void OnAbilityInput_Triggered(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	void OnAbilityInput_Completed(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	void OnAbilityInput_Ongoning(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	void OnAbilityInput_Canceled(const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	/* 입력 처리 */

public:
	/* 현재 추가되어 있는 AttributeSet 별 작업 실행 */
	void ForEachAttributeSet_Execute(const FAGPForEachAttributeSet& InDelegate);

	/* Ability마다 해당 델리게이트의 작업 실행 */
	void ForEachAbility_Execute(const FAGPForEachAbility& InDelegate);
	
public:
	/* 로컬 전용 BaseFeature Ability 부여 */
	void GrantLocalBaseFeatureAbility();

	/* 기본적으로 보유할 Ability 부여 */
	void GrantBaseAbility();

	/* 동적 AttributeSet 추가 */
	/* 외부에서 UAbilitySystemComponent::AddAttributeSetSubobject, AddSpawnedAttribute를 직접 호출하지 말고 해당 함수를 이용할것 */
	void AddDynamicAttributeSet(TSubclassOf<UAGPAttributeSetBase> InAddAttributeSetClass);

	/* 동적으로 추가된 AttributeSet 제거 */
	/* 외부에서 UAbilitySystemComponent::RemoveSpawnedAttribute를 직접호출하지 말고 해당 함수를 이용할 것 */
	void RemoveDynamicAttributeSet(TSubclassOf<UAGPAttributeSetBase> InRemoveAttributeSetClass);

public:
	/* 특정 GameplayTag 추가 및 제거 */
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InGameplayTag"))
	void SetLooseGameplayTag(const FGameplayTag& InGameplayTag);

	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InGameplayTag"))
	void RemoveLooseGameplayTag(const FGameplayTag& InGameplayTag);


	/* 특정 Ability Spec에 대한 Input 이벤트 처리 */
protected:
	virtual void NotifyInputPressedToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void NotifyInputTriggeredToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void NotifyInputOngoningToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void NotifyInputReleasedToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);
	virtual void NotifyInputCanceledToSpec(FGameplayAbilitySpec& Spec, const FInputActionValue& InputActionValue, const FGameplayTag& InInputTag);

	/* FAGPDynamicAttributeSetElementContainer의 특정 AttributeSet 관련 이벤트(추가/삭제 등) 수신 */
private:
	void OnPreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void OnPostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void OnPostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	void OnAddedAttributeSet(UAGPAttributeSetBase* InAddAttributeSet);
	void OnRemoveAttributeSet(UAGPAttributeSetBase* InRemoveAttributeSet);
};
