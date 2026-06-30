// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AGPStateTagNotifierComponent.generated.h"

class UAGP_AbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStateTagEventDelegateSignature, const FGameplayTag&, StateTag);


UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPStateTagNotifierComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	/* 특정 State 태그가 추가되었을때 호출되는 이벤트 */
	UPROPERTY(BlueprintAssignable)
	FStateTagEventDelegateSignature OnAddedStateTag;

	/* 특정 State 태그가 제거되었을때 호출되는 이벤트 */
	UPROPERTY(BlueprintAssignable)
	FStateTagEventDelegateSignature OnRemovedStateTag;

	/* 이벤트를 수신받을 State Tag */
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "State"))
	TSet<FGameplayTag> RegisterCallbackStateTags;

private:
	/* RegisterCallbackStateTags들 중 현재 Owner에 설정된 GameplayTag들 */
	UPROPERTY(VisibleInstanceOnly)
	FGameplayTagContainer OwnedStateTags;

public:	
	UAGPStateTagNotifierComponent();

public:
	void InitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC);
	void UninitializeAbilitySystem(UAGP_AbilitySystemComponent* InOwnerASC);

private:
	void OnAddOrRemovedStateTags(const FGameplayTag Tag, int32 NewCount);
};
