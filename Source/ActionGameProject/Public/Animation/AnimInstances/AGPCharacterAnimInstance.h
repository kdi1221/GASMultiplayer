// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstances/AGPAnimInstanceBase.h"
#include "Animation/AnimInstanceProxy.h"
#include "Common/AGPCommonEnums.h"
#include "AGPCharacterAnimInstance.generated.h"

class AAGP_CharacterBase;
class UAGPLinkedAnimLayerBase;
class UAGPCharacterMovementComponent;



/* 애니메이션 멀티스레드 환경에서 캐릭터 = > 애니메이션 인스턴스로 필요한 정보들(Velocity, 가속도, 방향 등) 동기화에 사용됨
* NativeThreadSafeUpdateAnimation의 경우 최초에는 GameThread, 이후에는 WorkerThread(ParallelAnimationEvaluationTask) 에서 호출됨
* 즉 다른 스레드에서 호출되며, 이 때 GameThread에 있는 정보들을 동기화 하면서 수신받기 위해 해당 구조체를 상속받아서 사용함
* GetProxyOnAnyThread, GetProxyOnGameThread 등으로 Proxy 인스턴스에 접근하는데
* 이때 GameThread에서는 HandleExistingParallelEvaluationTask함수 내에서 애니메이션 스레드의 한 프레임이 종료될때까지 대기하게 된다.
* 즉 하나의 애니메이션 프레임이 완료된 후에 게임스레드에서 변경된 정보를 업데이트 할 수 있게 구성되어 있다.
*/
USTRUCT()
struct FAGPAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()
	
	friend class UAGPCharacterAnimInstance;
	
	//Owner 캐릭터 및 MovementComponent에 대한 약 포인터
	//절대로 애니메이션 스레드에서 이 객체에 직접 접근하지 말것
private:
	TWeakObjectPtr<AAGP_CharacterBase> OwningCharacter = nullptr;
	TWeakObjectPtr<UAGPCharacterMovementComponent> OwningMovementComponent = nullptr;

private:
	bool IsValidOwner = false;
	FVector Velocity = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector Acceleration = FVector::ZeroVector;
	bool IsTargetLockMove = false;
	EAGPCharacterDeath OwnerDeathStatus = EAGPCharacterDeath::None;
	FName OwnerDeathPoseName = NAME_None;
	
#pragma region [FAnimInstanceProxy Interfaces]
protected:
	//처음 초기화 시 호출
	virtual void Initialize(UAnimInstance* InAnimInstance) override;

	//매 애니메이션 프레임마다 호출(GameThread에서만 호출)
	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;

	//매 애니메이션 프레임마다 호출(NativeThreadSafeUpdateAnimation호출 이후 호출, GameThread 및 WorkerThread)
	virtual void Update(float DeltaSeconds) override;
#pragma endregion

private:
	void UpdateOwnerInfo();
};


UCLASS()
class ACTIONGAMEPROJECT_API UAGPCharacterAnimInstance : public UAGPAnimInstanceBase
{
	GENERATED_BODY()
	
	
//private:
//	/* 테스트 - GameplayTagContainer와 AGPAnimInstance 매핑 */
//	UPROPERTY(EditDefaultsOnly, meta = (ForceInlineRow))
//	TMap<FAGPTagContainerKey, TSubclassOf<UAGPAnimInstanceBase>> AnimationLinkMap;

private:
	UPROPERTY(Transient)
	FAGPAnimInstanceProxy AGPProxyInstance;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	float GroundSpeed;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bHasAcceleration;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bIsTargetLockMove;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	float LocomotionDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|CharacterState")
	EAGPCharacterDeath CurrentDeathStatus;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|CharacterState")
	FName CurrentDeathPoseName;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return &AGPProxyInstance; }

	//멤버 변수 AGPProxyInstance를 직접 반환하기때문에 따로 delete해주지 않음
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}

public:
	void SetAnimationLinkLayer(TSubclassOf<UAGPLinkedAnimLayerBase> InNewAnimLinkLayer);




// 테스트 - GameplayTagContainer와 AGPAnimInstance 매핑
//#if WITH_EDITORONLY_DATA
//private:
//	UPROPERTY()
//	TMap<FAGPTagContainerKey, TSubclassOf<UAGPAnimInstanceBase>> BackupAnimationLinkMap;
//
//private:
//	static TWeakPtr<class SNotificationItem> EditorInvalidOperationError;
//#endif // WITH_EDITORONLY_DATA
//
//#if WITH_EDITOR
//public:
//	virtual void PreEditChange(class FEditPropertyChain& PropertyAboutToChange) override;
//	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
//
//private:
//	/* AnimationLinkMap Edit중인지 확인 */
//	bool IsEditingAnimationLinkMap(const FEditPropertyChain& InPropertyChain) const;
//
//	/* Editor에서 편집 도중 발생하는 오류상황에 대해 우측하단 Overlay 오류 메시지 출력 */
//	void EditorShowInvalidOperationError(const FText& ErrorText) const;
//
//public:
//	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
//#endif // WITH_EDITOR

};
