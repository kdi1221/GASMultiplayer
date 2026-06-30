// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Common/AGPCommonEnums.h"
#include "AGP_AIController.generated.h"

struct FOnAttributeChangeData;
class UAIPerceptionComponent;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class AAGP_PlayerCharacter;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnChangedAIPawnHealthUpdated, AAGP_AIController*, float, float);


/**
 *  AIController Base Class
 */

UCLASS()
class ACTIONGAMEPROJECT_API AAGP_AIController : public AAIController
{
	GENERATED_BODY()
	
protected:
	/* AI 인식(시야, 청각 등) 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	/* AI 비헤이비어 트리 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBehaviorTreeComponent> AIBehaviorTreeComponent;

	/* AI 블랙보드 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBlackboardComponent> AIBlackboardComponent;

protected:
	/* 감지한 플레이어에게 상태 알림(체력 등, ex : 보스몬스터) */
	UPROPERTY(EditDefaultsOnly, Category = "Initialize", meta = (AllowPrivateAccess = "true"))
	bool bNotifyStateToPlayer = false;

	UPROPERTY(EditDefaultsOnly, Category = "Initialize", meta = (AllowPrivateAccess = "true"))
	EAGPTeamGroup InitializeTeamGroup = EAGPTeamGroup::NonPlayerGroup_1;

	/* Crowd(군중 이동 회피) 활성화 */
	UPROPERTY(EditDefaultsOnly, Category = "Detour Crowd Avoidance Config")
	EAGPCrowdEnableFlag CrwodEnableFlag = EAGPCrowdEnableFlag::Enabled;

	/* Crowd Avoid Quality */
	UPROPERTY(EditDefaultsOnly, Category = "Detour Crowd Avoidance Config", meta = (EditCondition = "CrwodEnableFlag != EAGPCrowdEnableFlag::Disabled"))
	EAGPCrowdAvoidQuality CrowdAvoidQuality = EAGPCrowdAvoidQuality::High;

	/* 지정할 Crowd Avoid Group */
	UPROPERTY(EditDefaultsOnly, Category = "Detour Crowd Avoidance Config", meta = (EditCondition = "CrwodEnableFlag != EAGPCrowdEnableFlag::Disabled"))
	int32 AvoidGroup = 1;

	/* 회피할 Avoid Group */
	UPROPERTY(EditDefaultsOnly, Category = "Detour Crowd Avoidance Config", meta = (EditCondition = "CrwodEnableFlag != EAGPCrowdEnableFlag::Disabled"))
	int32 GroupToAvoid = 1;

	/* Crowd Query Range */
	UPROPERTY(EditDefaultsOnly, Category = "Detour Crowd Avoidance Config", meta = (EditCondition = "CrwodEnableFlag != EAGPCrowdEnableFlag::Disabled"))
	float CrowdCollisionQueryRange = 600.f;

public:
	FOnChangedAIPawnHealthUpdated OnAIPawnHealthUpdated;

	/* 감지한 플레이어 캐릭터들 */
	TSet<TWeakObjectPtr<AAGP_PlayerCharacter>> PerceptionPlayerCharacter;

private:
	float CurrentHealthValue = 0.f;
	float MaxHealthValue = 0.f;

public:
	AAGP_AIController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

public:
	/* AIPerceptionComponent내에 센싱된 적대적 Actor들에 대한 반복 처리 */
	void ForEachHostileActors(const TFunctionRef<void(AActor*)>& FunctionPerActor) const;

protected:
	/* PerceptionComponent를 통해 특정 객체가 인식되었을때 호출 */
	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/* PerceptionComponent를 통해 특정 객체에 대한 인식을 종료했을때 호출 */
	UFUNCTION()
	virtual void OnPerceptionForgotten(AActor* Actor);

	/* 인식했던 캐릭터가 Destroy되었을때 호출 */
	UFUNCTION()
	void OnPerceptionCharacterDestroyed(AActor* DestroyedActor);

	/* 인식했던 캐릭터의 Death상태 변경에 대한 호출 */
	UFUNCTION()
	void OnPerceptionCharacterDeath(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState);

	/* 컨트롤중인 캐릭터의 Death 상태 변경시 호출 */
	UFUNCTION()
	void OnControlPawnDeath(AAGP_CharacterBase* UpdatedCharacter, EAGPCharacterDeath UpdateState);

protected:
	/* AI Character의 현재 체력 업데이트 이벤트 수신 */
	void OnChangeCurrentHealth(const FOnAttributeChangeData& Data);

	/* AI Character의 최대 체력 업데이트 이벤트 수신 */
	void OnChangeMaxHealth(const FOnAttributeChangeData& Data);

	/* 감지된 플레이어 정보 저장 */
	void RegisterPerceptionPlayer(AAGP_PlayerCharacter* PlayerCharacter);

	/* 감지해제된 플레이어 정보 초기화 */
	void UnregisterPerceptionPlayer(AAGP_PlayerCharacter* PlayerCharacter);

	/* 모든 센싱 활성화 / 비활성화 */
	void SetAllSenseEnable(bool bEnable);

	/* 모든 센싱정보 초기화*/
	void ResetAllSenseInfo();

	/* Possess등 컨트롤 중인 캐릭터의 Enable 이벤트 처리 */
	void OnControlChracterEnable();

	/* Unpossess, Death등 컨트롤 중인 캐릭터의 Disable 이벤트 처리 */
	void OnControlChracterDisable();


public:
	/* 해당 Actor에 대한 감지정보가 유효한지 확인 */
	UFUNCTION(BlueprintPure)
	bool IsPerceptionActor(AActor* InCheckActor) const;

	UFUNCTION(BlueprintPure)
	int32 GetMonsterNameID() const;

public:
	FORCEINLINE UAIPerceptionComponent* GetAGPPerceptionComponent() const { return AIPerceptionComponent; }
	FORCEINLINE float GetPawnCurrentHealth() const { return CurrentHealthValue; }
	FORCEINLINE float GetPawnMaxHealth() const { return MaxHealthValue; }
};
