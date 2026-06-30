// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Item/Instance/Functionality/AGPItemFunctionality.h"
#include "AGPItemFunc_SetAnimLinkLayer.generated.h"

class IAGPItemSystemExternalInterface;

/**
 * 아이템 이벤트(장비 등)에 따라 대상이 되는 캐릭터의 Animation Link Layer 설정
 */

UCLASS(DisplayName = "SetAnimationLinkLayer")
class ACTIONGAMEPROJECT_API UAGPItemFunc_SetAnimLinkLayer : public UAGPItemFunctionality
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly)
	FName AnimLinkLayerName;

public:
	UAGPItemFunc_SetAnimLinkLayer();

public:
	virtual void OnMountedInSlot(const UAGPItemSlot* InSlotInstance) override;
	virtual void OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance) override;

private:
	void SetOwnerAnimLinkLayer(IAGPItemSystemExternalInterface* InItemSystemInterface);
	void ResetOwnerAnimLinkLayer(IAGPItemSystemExternalInterface* InItemSystemInterface);

#if WITH_EDITOR
public:
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif	
};
