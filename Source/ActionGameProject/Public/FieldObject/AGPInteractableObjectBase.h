// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "FieldObject/AGPFieldObjectBase.h"
#include "Common/AGPCommonEnums.h"
#include "AGPInteractableObjectBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableObjectStateChangeSignature, EAGPInteractableActorState, CurrentState);

/**
 * 
 */
UCLASS()
class ACTIONGAMEPROJECT_API AAGPInteractableObjectBase : public AAGPFieldObjectBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentState)
	EAGPInteractableActorState CurrentState = EAGPInteractableActorState::Available;

public:
	UPROPERTY(BlueprintAssignable)
	FOnInteractableObjectStateChangeSignature OnInteractableObjectStateChanged;

public:
	AAGPInteractableObjectBase();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void ActivateObject();

private:
	void SetNewInteractableState(EAGPInteractableActorState NewState);

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnInteractableStateChanged();

private:
	UFUNCTION()
	void OnRep_CurrentState();

public:
	UFUNCTION(BlueprintPure)
	EAGPInteractableActorState GetCurrentState() const { return CurrentState; }
};
