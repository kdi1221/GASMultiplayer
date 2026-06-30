// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Item/Instance/Functionality/AGPItemFunctionality.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "AbilitySystem/AGPAbilityTypes.h"
#include "AGPItemFunc_GrantAbility.generated.h"

class IAbilitySystemInterface;

/**
 * 아이템에 대한 이벤트(ex : 장착 등)에 따라 Ability 부여
 */
UCLASS(DisplayName = "GrantAbility")
class ACTIONGAMEPROJECT_API UAGPItemFunc_GrantAbility : public UAGPItemFunctionality
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAGPGrantAbilitySet> GrantAbilities;


private:
	TArray<FGameplayAbilitySpecHandle> GrantAbilitiesSpecHandles;
	
public:
	UAGPItemFunc_GrantAbility();

public:
	virtual void OnMountedInSlot(const UAGPItemSlot* InSlotInstance) override;
	virtual void OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance) override;

private:
	void GrantAbilityToOwner(IAbilitySystemInterface* OwnerAbilitySystemInterface);
	void RevokeAbilityFromOwner(IAbilitySystemInterface* OwnerAbilitySystemInterface);

#if WITH_EDITOR
public:
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif	
};
