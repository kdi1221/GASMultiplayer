#pragma once

#include "CoreMinimal.h"
#include "AGPCommonEnums.generated.h"

UENUM()
enum class EAGPExecutionResult : uint8
{
	Success,
	Fail
};

/* 캐릭터들의 소속 구성 */
UENUM(BlueprintType)
enum class EAGPTeamGroup : uint8
{
	NoTeamGroup = 0			UMETA(Hidden),

	PlayerGroup_1 = 1		UMETA(DisplayName = "PlayerGroup1"),

	NonPlayerGroup_1 = 30	UMETA(DisplayName = "NonPlayerGroup1"),
	NonPlayerGroup_2		UMETA(DisplayName = "NonPlayerGroup2"),
	NonPlayerGroup_3		UMETA(DisplayName = "NonPlayerGroup3")
};

/* 게임 내 사용되는 방향 정의 */
UENUM(BlueprintType)
enum class EAGPDirection : uint8
{
	Front = 0,
	Left,
	Right,
	Back
};

/* HitReact 종류 구분 */
UENUM(BlueprintType)
enum class EAGPHitReactType : uint8
{
	/* HitReact Montage Player 안함 */
	NoHitReactMontage = 0,

	/* Source의 위치에 따른 방향성있는 HitReact */
	DirectionHitReact,

	/* Source를 향해 회전 */
	HitReactToRotateToSource,
};

/* Character Death 구분 */
UENUM(BlueprintType)
enum class EAGPCharacterDeath : uint8
{
	None = 0	UMETA(DisplayName = "None"),
	Dying		UMETA(DisplayName = "To Dying"),
	Death		UMETA(DisplayName = "Death"),
	Destroy		UMETA(DisplayName = "Death Destory"),
	Rebirth		UMETA(DisplayName = "Rebirth")
};

/* AI Crowd Enable Flag (ECrowdSimulationState) */
UENUM(BlueprintType)
enum class EAGPCrowdEnableFlag : uint8
{
	Enabled = 0,
	ObstacleOnly	UMETA(DisplayName = "Disabled, avoided by others"),
	Disabled		UMETA(DisplayName = "Disabled, ignored by others"),
};

/* AI Crowd Avoid Quality (ECrowdAvoidanceQuality) */
UENUM(BlueprintType)
enum class EAGPCrowdAvoidQuality : uint8
{
	Low = 0,
	Medium,
	Good,
	High,
};

/* FAGPCountdownLatentAction에 사용되는 Input Enum */
UENUM()
enum class EAGPCountdownActionInput : uint8
{
	Start,
	Cancel
};

/* FAGPCountdownLatentAction에 사용되는 Output Enum */
UENUM()
enum class EAGPCountdownActionOutput : uint8
{
	Updated,
	Completed,
	Cancelled,
};

/* 웨이브 생존 게임모드 - 각 상태 정의 */
UENUM(BlueprintType)
enum class EAGPSurvialGameModeState : uint8
{
	None = 0,
	WaitStart,

	WaitForNextWave,
	PreCurrentWave,
	InProgressCurrentWave,
	CompleteCurrentWave,
	PreWaveComplete,

	AllWavesClear,
	PlayersDefeat,

	GameEndPlayerKick,
	Shutdown
};

/* Survival Mode에서의 PlayerState의 상태 */
UENUM()
enum class EAGPSurvivalModePlayerState : uint8
{
	None = 0,
	Prepare,
	Active,
	Inactive,
};

/* 필드 내 오브젝트와의 인터랙션 모드 */
UENUM(BlueprintType)
enum class EAGPInteractionMode : uint8
{
	None = 0,
	InteractableObject,
	FieldStone
};

/* 필드 내 오브젝트의 상태 정의 */
UENUM(BlueprintType)
enum class EAGPInteractableActorState : uint8
{
	Available,   // 활성화 가능
	Activated    // 이미 활성화됨
};

/* 대상에 대한 Damage 적용 방식 구분 */
UENUM()
enum class EAGPDamageApplyType : uint8
{
	/* 대상에 GameplayEffect 적용 */
	ApplyGameplayEffect = 0,

	/* 대상에 대해 통상적인 ApplyDamage함수 적용 */
	ApplyStandardDamageType
};