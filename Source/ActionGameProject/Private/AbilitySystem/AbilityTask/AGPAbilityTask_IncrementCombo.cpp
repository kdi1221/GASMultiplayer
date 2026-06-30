// KJY All Rights Reserved


#include "AbilitySystem/AbilityTask/AGPAbilityTask_IncrementCombo.h"
#include "AbilitySystemComponent.h"
#include "Log/AGPLogChannels.h"

bool FGameplayAbilityTargetData_NextCombo::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << NextComboCount;

	bOutSuccess = true;
	return true;
}

UAGPAbilityTask_IncrementCombo* UAGPAbilityTask_IncrementCombo::CreateIncrementCombo(UGameplayAbility* OwningAbility, int32 ComboCount, int32 MaxCount)
{
	UAGPAbilityTask_IncrementCombo* NewIncrementComboTask = NewAbilityTask<UAGPAbilityTask_IncrementCombo>(OwningAbility);
	verifyf(NewIncrementComboTask, TEXT("[%s], NewIncrementComboTask Invalid"), __FUNCTIONW__);

	NewIncrementComboTask->CurrentComboCount = ComboCount;
	NewIncrementComboTask->MaxComboCount = MaxCount;

	return NewIncrementComboTask;
}

void UAGPAbilityTask_IncrementCombo::Activate()
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

void UAGPAbilityTask_IncrementCombo::IncrementComboCount()
{
	//Combo Count 증가
	NextComboCount = FMath::Min(CurrentComboCount + 1, MaxComboCount);
}

void UAGPAbilityTask_IncrementCombo::NotifyIncrementAfter()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (CurrentComboCount == NextComboCount)
		{
			ComboMaxLimit.Broadcast();
		}
		else
		{
			IncrementSuccess.Broadcast(NextComboCount);
		}
	}
}

void UAGPAbilityTask_IncrementCombo::NotifyClientComboCountInvalid()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ClientComoboCountInvalid.Broadcast();
	}
}

void UAGPAbilityTask_IncrementCombo::OnActiveClient()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	//이 범위(SendMouseCursorData 함수 내) 모든 작업에 대한 예측키 설정,
	//생성된 예측키는 이후 클라이언트 - 서버 간의 동기화 지점 역할을 한다.
	//이후 ServerSetReplicatedTargetData와 같은 ASC 내부의 Replicate함수에서 해당 예측키를 사용하게 된다.
	//서버에서 실행 후 피드백을 받았을 때, 해당 키로 성공 / 실패 작업을 구분한다.
	//참고 : https://zhuanlan.zhihu.com/p/507862135, https://zhuanlan.zhihu.com/p/143637846
	const bool IsClientPredict = IsPredictingClient();
	FScopedPredictionWindow ScopedPrediction(ASC, IsClientPredict);

	IncrementComboCount();

	if (IsClientPredict)
	{
		// Server에 자신의 NextComboCount를 알리기 위한 TargetData_NextCombo 생성
		FGameplayAbilityTargetData_NextCombo* NextComboData = new FGameplayAbilityTargetData_NextCombo();
		checkf(NextComboData, TEXT("[%s], Invalid NextComboData"), __FUNCTIONW__);
		NextComboData->NextComboCount = NextComboCount;

		//TargetDataHandle에 TargetData_NextCombo 추가
		FGameplayAbilityTargetDataHandle DataHandle;
		DataHandle.Add(NextComboData);

		// Server에 자신이 증가시킨 NextComboCount를 알린다.
		ASC->ServerSetReplicatedTargetData(
			GetAbilitySpecHandle(),
			GetActivationPredictionKey(),
			DataHandle,
			FGameplayTag(),
			ASC->ScopedPredictionKey);
	}

	NotifyIncrementAfter();

	EndTask();
}

void UAGPAbilityTask_IncrementCombo::OnActiveServer()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
	const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

	//서버인 경우 클라이언트로 타겟데이터를 받았을때 호출할 델리게이트를 설정한다. 이후 데이터를 수신받아 처리한뒤 더이상 필요없어지면 ConsumeClientReplicatedTargetData를 호출한다.
	ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UAGPAbilityTask_IncrementCombo::OnReplicatedComboDataCallback);

	//능력이 활성화되기전에 타겟데이터를 받았으면 곧바로 델리게이트를 호출한다.
	const bool bCalledDelegate = ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

	//능력이 활성화된 시점에 아직 클라이언트로부터 타겟데이터가 전송되지 않은 경우 => 즉 델리게이트가 아직 호출되지 않은 경우
	if (!bCalledDelegate)
	{
		//클라이언트로부터 보내질 타겟데이터를 수신할때까지 대기한다.
		SetWaitingOnRemotePlayerData();
	}
}

void UAGPAbilityTask_IncrementCombo::OnReplicatedComboDataCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	const FGameplayAbilityTargetData_NextCombo* ClientNextComboData = static_cast<const FGameplayAbilityTargetData_NextCombo*>(DataHandle.Get(0));
	checkf(ClientNextComboData, TEXT("[%s], Invalid NextComboData"), __FUNCTIONW__);

	/* ConsumeClientReplicatedTargetData 호출후에는 TargetData가 소멸될 수 있음, 따라서 그전에 DataHandle 내 TargetData의 값을 복사 */
	const int32 ClientNextComboCount = ClientNextComboData->NextComboCount;

	//타겟데이터 사용이 완료되었으므로 ASC에 더이상 TargetData의 Replicate 이벤트 수신할 필요가 없음을 알림 
	ASC->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	/* 서버 자체적으로 Combo Count 증가 */
	IncrementComboCount();

	// Client에서 보내온 ComboCount와 서버 자체적으로 증가한 ComboCount의 비교, 만약 둘의 차이가 있다면 문제가 있으므로 Invalid 처리
	if (ClientNextComboCount != NextComboCount)
	{
		NotifyClientComboCountInvalid();
	}
	else
	{
		NotifyIncrementAfter();
	}

	EndTask();
}
