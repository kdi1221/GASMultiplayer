// KJY All Rights Reserved


#include "AbilitySystem/AbilityTask/AGPAbilityTask_SyncDodgeDir.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "AbilitySystem/Abilities/AGPGameplayAbility.h"

bool FGameplayAbilityTargetData_DodgeDirection::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << DodgeDirection;

	bOutSuccess = true;
	return true;
}

UAGPAbilityTask_SyncDodgeDir* UAGPAbilityTask_SyncDodgeDir::CreateSyncDodgeDirection(UGameplayAbility* OwningAbility)
{
	UAGPAbilityTask_SyncDodgeDir* NewSyncDodgeDirTask = NewAbilityTask<UAGPAbilityTask_SyncDodgeDir>(OwningAbility);
	verifyf(NewSyncDodgeDirTask, TEXT("[%s], NewSyncDodgeDirTask Invalid"), __FUNCTIONW__);

	return NewSyncDodgeDirTask;
}

void UAGPAbilityTask_SyncDodgeDir::Activate()
{
	if (IsLocallyControlled())
	{
		OnActiveClient();
	}
	else
	{
		OnActiveServer();
	}
}

void UAGPAbilityTask_SyncDodgeDir::OnActiveClient()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	const bool IsClientPredict = IsPredictingClient();
	FScopedPredictionWindow ScopedPrediction(ASC, IsClientPredict);

	//플레이어의 LastMovementInput을 Dodge 회전방향으로 결정
	const FVector LastMovementInput = UAGPAbilitySystemFunctionLibrary::GetLastMovementInputFromAvatar(Cast<UAGPGameplayAbility>(Ability));
	if (LastMovementInput.IsNearlyZero())
	{
		NotifyInvalidDodgeDirection();
	}
	else
	{
		FVector DodgeDirection = FVector::ZeroVector;

		if (IsClientPredict)
		{
			/* 예측 클라이언트의 경우 Server에 자신의 Dodge 방향을 알린다. */
			FGameplayAbilityTargetData_DodgeDirection* DodgeDirectionData = new FGameplayAbilityTargetData_DodgeDirection(LastMovementInput);
			verifyf(DodgeDirectionData, TEXT("[%s], Invalid DodgeDirectionData"), __FUNCTIONW__);

			FGameplayAbilityTargetDataHandle DataHandle;
			DataHandle.Add(DodgeDirectionData);

			ASC->ServerSetReplicatedTargetData(
				GetAbilitySpecHandle(),
				GetActivationPredictionKey(),
				DataHandle,
				FGameplayTag(),
				ASC->ScopedPredictionKey);

			/* FVector_NetQuantizeNormal에서 다시 FVector로 대입(정밀도 손실 고려) */
			DodgeDirection = DodgeDirectionData->DodgeDirection;
		}
		else
		{
			DodgeDirection = LastMovementInput.GetSafeNormal();
		}

		NotifyDodgeDirection(DodgeDirection);
	}
	
	EndTask();
}

void UAGPAbilityTask_SyncDodgeDir::OnActiveServer()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
	const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

	ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UAGPAbilityTask_SyncDodgeDir::OnReplicatedDodgeDirectionDataCallback);

	const bool bCalledDelegate = ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

	if (!bCalledDelegate)
	{
		SetWaitingOnRemotePlayerData();
	}
}

void UAGPAbilityTask_SyncDodgeDir::NotifyDodgeDirection(const FVector& InDodgeDirection)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		DodgeDirectionSync.Broadcast(InDodgeDirection);
	}
}

void UAGPAbilityTask_SyncDodgeDir::NotifyInvalidDodgeDirection()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		InvalidDodgeDirection.Broadcast();
	}
}

void UAGPAbilityTask_SyncDodgeDir::OnReplicatedDodgeDirectionDataCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	const FGameplayAbilityTargetData_DodgeDirection* ClientDodgeDirectionData = static_cast<const FGameplayAbilityTargetData_DodgeDirection*>(DataHandle.Get(0));
	verifyf(ClientDodgeDirectionData, TEXT("[%s], Invalid NextComboData"), __FUNCTIONW__);

	const FVector ClientDodgeDirection = ClientDodgeDirectionData->DodgeDirection;
	if (ClientDodgeDirection.IsNearlyZero())
	{
		/* 클라이언트에서 보내온 방향이 정상적이지 않으면 실패 처리 */
		NotifyInvalidDodgeDirection();
	}
	else
	{
		/* 방향이 정상적이면 다음 과정 진행 */
		NotifyDodgeDirection(ClientDodgeDirection);
	}

	EndTask();
}


