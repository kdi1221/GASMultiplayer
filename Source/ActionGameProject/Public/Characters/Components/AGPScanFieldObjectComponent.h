// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/AGPCommonEnums.h"
#include "AGPScanFieldObjectComponent.generated.h"

class AAGPFieldObjectBase;
class AAGPFieldStone;
class AAGPInteractableObjectBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionModeChangeSignature, EAGPInteractionMode, InteractionMode);

/* 필드 내 오브젝트들 스캔*/

UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPScanFieldObjectComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FInteractionModeChangeSignature OnChangeCurrentInteractionMode;
	
private:
	/* 현재 오브젝트의 Interaction Mode */
	UPROPERTY(VisibleAnywhere, Transient, ReplicatedUsing = OnRep_CurrentInteractionMode)
	EAGPInteractionMode CurrentInteractionMode = EAGPInteractionMode::None;
	
private:
	/* 현재 Overlap으로 Scan된 Field 내 Stone Object들 */
	TSet<TWeakObjectPtr<AAGPFieldStone>> ScanFieldStones;

	/* 현재 Overlap으로 Scane된 Interactable Object */
	TWeakObjectPtr<AAGPInteractableObjectBase> ScanInteractableObject;

public:	
	UAGPScanFieldObjectComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable)
	void OnBeginOverlapFieldObject(AAGPFieldObjectBase* InFieldObject);

	UFUNCTION(BlueprintCallable)
	void OnEndOverlapFieldObject(AAGPFieldObjectBase* InFieldObject);

	UFUNCTION(BlueprintCallable)
	void ConsumeCurrentScanStones();

	UFUNCTION(BlueprintCallable)
	void ActivateCurrentInteractableObject();

private:
	void CollectFieldStoneObject(AAGPFieldStone* InFieldStone);
	void DiscardFieldStoneObject(AAGPFieldStone* InFieldStone);

	void OnBeginOverlapInteractableObject(AAGPInteractableObjectBase* InInteractableObject);
	void OnEndOverlapInteractableObject(AAGPInteractableObjectBase* InInteractableObject);

	/* 현재 Interaction모드 상태 최종 결정 */
	void UpdateCurrentInteractionMode();

	//void NotifyEnableConsumeStoneChange() const;
	void NotifyInteractionModeChange() const;

private:
	UFUNCTION()
	void OnRep_CurrentInteractionMode() const;

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE EAGPInteractionMode GetCurrentInteractionMode() const { return CurrentInteractionMode; }
};
