// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
///D:\Unreal\UE_5.4\Engine\Source\Runtime\Engine\Classes\GameFramework\CharacterMovementReplication.h
#include "AGPCharacterMovementComponent.generated.h"

class UAGP_AbilitySystemComponent;

#pragma region[FSavedMove_AGPCharacter]

/* 커스텀 SavedMove 구조체 */
class FSavedMove_AGPCharacter : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

public:
	/* 타겟 잠금 이동 상태 */
	uint32 bMoveTargetLock : 1;

	/* 타겟 위치 */
	FVector TargetLockLocation;

public:
	virtual void Clear();
	//virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	//virtual void PrepMoveFor(ACharacter* InCharacter) override;
};

#pragma endregion

#pragma region[FNetworkPredictionData_Client_AGPCharacter]

class FNetworkPredictionData_Client_AGPCharacter : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_AGPCharacter(const UCharacterMovementComponent& ClientMovement);

public:
	virtual FSavedMovePtr AllocateNewMove() override;
};

#pragma endregion

#pragma region[FAGPCharacterNetworkMoveData]

struct FAGPCharacterNetworkMoveData : public FCharacterNetworkMoveData
{
public:
	enum eCustomMoveFlag
	{
		eCustomMoveFlag_TargetLock = 0x01,	//타겟잠금에 대한 플래그
		//기타 추가
	};

public:
	/* 별도 Custom Flag(타겟 잠금 이동 등) */
	uint8 CustomMoveFlag;

	/* 타겟 위치 */
	FVector_NetQuantize100 TargetLockLocation;

public:
	FAGPCharacterNetworkMoveData();

public:
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override;

public:
	bool CheckCustomFlag(eCustomMoveFlag InCheckFlag) const;
};

#pragma endregion

#pragma region[FAGPCharacterNetworkMoveDataContainer]

struct FAGPCharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
private:
	FAGPCharacterNetworkMoveData CustomMoveDataBuffer[3];

public:
	FAGPCharacterNetworkMoveDataContainer();
};

#pragma endregion


#pragma region[FAGPCharacterNetworkMoveDataContainer]

/**
 * Character의 MovementComponent 클래스
 *
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
		
protected:
	/* 타겟 잠금 상태일때의 이동 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock")
	float TargetLockMaxWalkSpeed = 150.f;

	/* 타겟 잠금 상태일때의 회전 보간 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "TargetLock")
	float TargetLockRotationInterpSpeed = 5.f;

	/* 커스텀 이동 동기화를 위한 MoveData Container */
private:
	FAGPCharacterNetworkMoveDataContainer CustomNetworkMoveDataContainer;

public:
	UAGPCharacterMovementComponent();

protected:
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel);
	virtual bool ServerCheckClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& ClientWorldLocation, const FVector& RelativeClientLocation, UPrimitiveComponent* ClientMovementBase, FName ClientBaseBoneName, uint8 ClientMovementMode) override;

public:
	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const override;

public:
	virtual float GetMaxSpeed() const override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	/* MoveAutonomous에서 타겟 잠금에 대한 Flag Update */
	void OnMoveAutonomous_TargetLockUpdate(const FAGPCharacterNetworkMoveData* InNetworkMoveData);

protected:
	/* 타겟 잠금 이동 여부(캐릭터에 저장된 플래그 반환) */
	bool IsTargetLockMove() const;

	/* 타겟 잠금 위치 반환(캐릭터에 저장된 위치 반환) */
	FVector GetTargetLockLocation() const;

};

#pragma endregion