// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Item/Instance/Functionality/AGPItemFunctionality.h"
#include "AGPItemFunc_BindInput.generated.h"

class UAGPInputConfig;
class AAGP_PlayerControllerBase;

/**
 * 아이템에 대한 이벤트(ex : 장착 등)에 따라 Input Binding(For Player)
 */

UCLASS(DisplayName = "BindInput")
class ACTIONGAMEPROJECT_API UAGPItemFunc_BindInput : public UAGPItemFunctionality
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UAGPInputConfig> InputConfig;

private:
	TArray<uint32> BindingInputHandles;
	
public:
	UAGPItemFunc_BindInput();

public:
	virtual void OnMountedInSlot(const UAGPItemSlot* InSlotInstance) override;
	virtual void OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance) override;
	
private:
	AAGP_PlayerControllerBase* GetLocalPlayerControllerFromSlotOwner(AActor* InSlotOwnerActor) const;

#if WITH_EDITOR
public:
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif	
};
