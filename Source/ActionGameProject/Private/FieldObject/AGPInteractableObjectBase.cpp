// KJY All Rights Reserved


#include "FieldObject/AGPInteractableObjectBase.h"
#include "Net/UnrealNetwork.h"

AAGPInteractableObjectBase::AAGPInteractableObjectBase()
{
}

void AAGPInteractableObjectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAGPInteractableObjectBase, CurrentState);
}

void AAGPInteractableObjectBase::ActivateObject()
{
	if (EAGPInteractableActorState::Available != GetCurrentState())
	{
		return;
	}

	SetNewInteractableState(EAGPInteractableActorState::Activated);
}

void AAGPInteractableObjectBase::SetNewInteractableState(EAGPInteractableActorState NewState)
{
	const EAGPInteractableActorState BeforeState = CurrentState;
	CurrentState = NewState;

	if (CurrentState != BeforeState)
	{
		OnInteractableObjectStateChanged.Broadcast(CurrentState);
		OnInteractableStateChanged();
	}
}

void AAGPInteractableObjectBase::OnRep_CurrentState()
{
	OnInteractableObjectStateChanged.Broadcast(CurrentState);
	OnInteractableStateChanged();
}
