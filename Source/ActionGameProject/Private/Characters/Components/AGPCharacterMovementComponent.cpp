// KJY All Rights Reserved


#include "Characters/Components/AGPCharacterMovementComponent.h"
#include "AbilitySystem/AGP_AbilitySystemComponent.h"
#include "AGPGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/AGP_CharacterBase.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

#pragma region[FSavedMove_AGPCharacter]

void FSavedMove_AGPCharacter::Clear()
{
	Super::Clear();

	bMoveTargetLock = false;

	TargetLockLocation = FVector::ZeroVector;
}

//uint8 FSavedMove_AGPCharacter::GetCompressedFlags() const
//{
//	uint8 Result = Super::GetCompressedFlags();
//
//	/* 타겟 잠금에 대한 Flag 설정 */
//	if (bMoveTargetLock)
//	{
//		Result |= FLAG_Custom_0;
//	}
//
//	return Result;
//}

bool FSavedMove_AGPCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_AGPCharacter* NewCustomMove = static_cast<const FSavedMove_AGPCharacter*>(NewMove.Get());
	if (!NewCustomMove)
	{
		return false;
	}

	/* 두 이동 정보간 타겟 잠금이 다르면 합칠 수 없음 */
	if (bMoveTargetLock != NewCustomMove->bMoveTargetLock)
	{
		return false;
	}

	/* 두 이동 정보가 모두 타겟잠금이 같으면 타겟 위치가 동일한지 확인 */
	if (bMoveTargetLock)
	{
		if (!TargetLockLocation.Equals(NewCustomMove->TargetLockLocation, 1.0f))
		{
			return false;
		}
	}

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_AGPCharacter::SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(InCharacter, InDeltaTime, NewAccel, ClientData);

	AAGP_CharacterBase* OwnerCharacter = Cast<AAGP_CharacterBase>(InCharacter);
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	/* MovementComponent => SavedMove */
	bMoveTargetLock = OwnerCharacter->IsTargetLockMove();
	TargetLockLocation = OwnerCharacter->GetTargetLockLocation();
}

//void FSavedMove_AGPCharacter::PrepMoveFor(ACharacter* InCharacter)
//{
//	Super::PrepMoveFor(InCharacter);
//
//	if (!IsValid(InCharacter))
//	{
//		return;
//	}
//
//	/* SavedMove => MovementComponent */
//	UAGPCharacterMovementComponent* MovementComponent = InCharacter->GetCharacterMovement<UAGPCharacterMovementComponent>();
//	if (MovementComponent)
//	{
//		if (bMoveTargetLock)
//		{
//			MovementComponent->StartTargetLockMove();
//			MovementComponent->UpdateTargetLockPostion(TargetLockLocation);
//		}
//		else
//		{
//			MovementComponent->StopTargetLockMove();
//		}
//	}
//}

#pragma endregion

#pragma region[FNetworkPredictionData_Client_AGPCharacter]

FNetworkPredictionData_Client_AGPCharacter::FNetworkPredictionData_Client_AGPCharacter(const UCharacterMovementComponent& ClientMovement)
	:FNetworkPredictionData_Client_Character(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_AGPCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_AGPCharacter());
}

#pragma endregion

#pragma region[FAGPCharacterNetworkMoveData]

FAGPCharacterNetworkMoveData::FAGPCharacterNetworkMoveData()
	:CustomMoveFlag(0)
	,TargetLockLocation(ForceInitToZero)
{

}

void FAGPCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	FCharacterNetworkMoveData::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedMove_AGPCharacter& SavedMoveCustom = static_cast<const FSavedMove_AGPCharacter&>(ClientMove);

	/* 플래그 초기화 */
	CustomMoveFlag = 0;

	/* 타겟잠금 이동에 대한 플래그 설정 */
	if (SavedMoveCustom.bMoveTargetLock)
	{
		CustomMoveFlag |= eCustomMoveFlag_TargetLock;
		TargetLockLocation = SavedMoveCustom.TargetLockLocation;
	}
	else
	{
		TargetLockLocation = FVector_NetQuantize100::ZeroVector;
	}
}

bool FAGPCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	if (!FCharacterNetworkMoveData::Serialize(CharacterMovement, Ar, PackageMap, MoveType))
	{
		return false;
	}

	bool bLocalSuccess = true;
	const bool bIsSaving = Ar.IsSaving();

	SerializeOptionalValue<uint8>(bIsSaving, Ar, CustomMoveFlag, 0);

	/* 타겟잠금 이동이 설정되어있으면 Target 정보까지 직렬화 */
	if (CheckCustomFlag(eCustomMoveFlag_TargetLock))
	{
		TargetLockLocation.NetSerialize(Ar, PackageMap, bLocalSuccess);
	}

	return !Ar.IsError();
}

bool FAGPCharacterNetworkMoveData::CheckCustomFlag(eCustomMoveFlag InCheckFlag) const
{
	return (CustomMoveFlag & InCheckFlag) != 0;
}

#pragma endregion

#pragma region[FAGPCharacterNetworkMoveDataContainer]

FAGPCharacterNetworkMoveDataContainer::FAGPCharacterNetworkMoveDataContainer()
{
	NewMoveData = &CustomMoveDataBuffer[0];
	PendingMoveData = &CustomMoveDataBuffer[1];
	OldMoveData = &CustomMoveDataBuffer[2];
}

#pragma endregion

UAGPCharacterMovementComponent::UAGPCharacterMovementComponent()
{
	SetNetworkMoveDataContainer(CustomNetworkMoveDataContainer);
}

void UAGPCharacterMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel)
{
	const FAGPCharacterNetworkMoveData* CurrentCustomNetworkMoveData = static_cast<FAGPCharacterNetworkMoveData*>(GetCurrentNetworkMoveData());
	ensureMsgf(CurrentCustomNetworkMoveData, TEXT("[%s], Invalid CurrentNetworkMoveData"), __FUNCTIONW__);

	/* 타겟잠금 상태 갱신 */
	OnMoveAutonomous_TargetLockUpdate(CurrentCustomNetworkMoveData);

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}

bool UAGPCharacterMovementComponent::ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	if (Super::ServerCheckClientError(ClientTimeStamp, DeltaTime, Accel, ClientWorldLocation, RelativeClientLocation, ClientMovementBase, ClientBaseBoneName, ClientMovementMode))
	{
		return true;
	}

	const FAGPCharacterNetworkMoveData* CurrentMoveData = static_cast<const FAGPCharacterNetworkMoveData*>(GetCurrentNetworkMoveData());
	checkf(CurrentMoveData, TEXT("[%s], Invalid CurrentMoveData"), __FUNCTIONW__);

	/* Client, Server의 Target잠금 상태를 비교 */
	const bool IsClientTargetLock = CurrentMoveData->CheckCustomFlag(FAGPCharacterNetworkMoveData::eCustomMoveFlag_TargetLock);
	const bool IsServerTargetLock = IsTargetLockMove();
	if (IsClientTargetLock != IsServerTargetLock)
	{
		return true;
	}
	
	return false;
}

FRotator UAGPCharacterMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const
{
	if (IsTargetLockMove())
	{
		/* 타겟 잠금 상태일때는 타겟을 향해 회전하도록 지정 */
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GetTargetLockLocation());

		/* 회전 보간 */
		const FRotator TargetRot = FMath::RInterpTo(CurrentRotation, LookAtRot, DeltaTime, TargetLockRotationInterpSpeed);

		return FRotator(0.f, TargetRot.Yaw, 0.f);
	}

	/* 기본은 현재 가속도 방향을 기준으로 캐릭터 회전 */
	return Super::ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRotation);
}

float UAGPCharacterMovementComponent::GetMaxSpeed() const
{
	const float MoveMaxSpeed = Super::GetMaxSpeed();

	/* 타겟 잠금 상태일때는 Walking Speed 조정 */
	if (IsTargetLockMove())
	{
		if ((MOVE_Walking == MovementMode || MOVE_NavWalking == MovementMode) &&
			!IsCrouching())
		{
			return TargetLockMaxWalkSpeed;
		}
	}
	
	return MoveMaxSpeed;
}

FNetworkPredictionData_Client* UAGPCharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UAGPCharacterMovementComponent* MutableThis = const_cast<UAGPCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_AGPCharacter(*this);
	}

	return ClientPredictionData;
}

void UAGPCharacterMovementComponent::OnMoveAutonomous_TargetLockUpdate(const FAGPCharacterNetworkMoveData* InNetworkMoveData)
{
	checkf(InNetworkMoveData, TEXT("[%s], Invalid InNetworkMoveData"), __FUNCTIONW__);

	AAGP_CharacterBase* OwnerCharacter = Cast<AAGP_CharacterBase>(GetCharacterOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (InNetworkMoveData->CheckCustomFlag(FAGPCharacterNetworkMoveData::eCustomMoveFlag_TargetLock))
	{
		OwnerCharacter->StartTargetLockMove();

		const FVector NewTargetLocation = InNetworkMoveData->TargetLockLocation;
		OwnerCharacter->UpdateTargetLockPostion(NewTargetLocation);
	}
	else
	{
		OwnerCharacter->StopTargetLockMove();
	}
}

bool UAGPCharacterMovementComponent::IsTargetLockMove() const
{
	AAGP_CharacterBase* OwnerCharacter = Cast<AAGP_CharacterBase>(GetCharacterOwner());
	return OwnerCharacter ? OwnerCharacter->IsTargetLockMove() : false;
}

FVector UAGPCharacterMovementComponent::GetTargetLockLocation() const
{
	AAGP_CharacterBase* OwnerCharacter = Cast<AAGP_CharacterBase>(GetCharacterOwner());
	return OwnerCharacter ? OwnerCharacter->GetTargetLockLocation() : FVector::ZeroVector;
}