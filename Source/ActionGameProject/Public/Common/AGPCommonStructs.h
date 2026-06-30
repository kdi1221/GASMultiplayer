#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AGPCommonEnums.h"
#include "AGPCommonStructs.generated.h"

/* GameplayTagContainer를 TMap, TSet에 사용할 수 있게 정의한 구조체 */
USTRUCT()
struct FAGPTagContainerKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer TagContainer;

public:
	FAGPTagContainerKey();

public:
	uint32 GetKeyHash() const;

public:
	FORCEINLINE bool operator==(const FAGPTagContainerKey& Other) const
	{
		return GetKeyHash() == Other.GetKeyHash();
	}
};

FORCEINLINE uint32 GetTypeHash(const FAGPTagContainerKey& InLinkKey)
{
	return InLinkKey.GetKeyHash();
}

/* 요청 패킷에 대한 PacketKey 생성 */
class FAGPRequestPacketKeyGenerator
{
public:
	static constexpr int32 INVALID_REQUEST_PACKET_KEY = 0;
	static constexpr int64 INVALID_REQUEST_PACKET_TIMESTAMP = 0;

private:
	static int32 NextBaseFeaturePacketKey;
	static int32 CurrentBaseFeaturePacketKey;

private:
	FAGPRequestPacketKeyGenerator() = default;
	~FAGPRequestPacketKeyGenerator() = default;

public:
	static FAGPRequestPacketKey GenerateRequestPacketKey();
};


/* 기본 기능 요청 RPC의 응답대기 Delegate TMap의 Key 구조체 */
USTRUCT()
struct FAGPRequestPacketKey
{
	GENERATED_BODY()

private:
	/* 패킷에 할당되는 고유 Key */
	UPROPERTY()
	int32 BaseFeaturePacketKey = FAGPRequestPacketKeyGenerator::INVALID_REQUEST_PACKET_KEY;

	/* 패킷을 보낸 시간 */
	UPROPERTY()
	int64 PacketTimeStamp = FAGPRequestPacketKeyGenerator::INVALID_REQUEST_PACKET_TIMESTAMP;

public:
	FAGPRequestPacketKey() = default;
	FAGPRequestPacketKey(const int32 InPacketKey, const int64 InTimeStamp)
		:BaseFeaturePacketKey(InPacketKey)
		,PacketTimeStamp(InTimeStamp)
	{

	}

public:
	FORCEINLINE bool operator==(const FAGPRequestPacketKey& Other) const
	{
		return BaseFeaturePacketKey == Other.BaseFeaturePacketKey &&
			PacketTimeStamp == Other.PacketTimeStamp;
	}

public:
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << BaseFeaturePacketKey;
		Ar << PacketTimeStamp;

		bOutSuccess = true;

		return true;
	}

public:
	FORCEINLINE int32 GetPacketKey() const { return BaseFeaturePacketKey; }
	FORCEINLINE int64 GetPacketTimeStamp() const { return PacketTimeStamp; }

public:
	void Reset()
	{
		BaseFeaturePacketKey = FAGPRequestPacketKeyGenerator::INVALID_REQUEST_PACKET_KEY;
		PacketTimeStamp = FAGPRequestPacketKeyGenerator::INVALID_REQUEST_PACKET_TIMESTAMP;
	}

	FString ToString() const
	{
		return FString::Format(TEXT("PacketKey{0}, TimeStamp{1}"), { BaseFeaturePacketKey, PacketTimeStamp });
	}

	bool IsValid() const
	{
		return (BaseFeaturePacketKey != FAGPRequestPacketKeyGenerator::INVALID_REQUEST_PACKET_KEY) && 
			(PacketTimeStamp != FAGPRequestPacketKeyGenerator::INVALID_REQUEST_PACKET_TIMESTAMP);
	}
};

FORCEINLINE uint32 GetTypeHash(const FAGPRequestPacketKey& InRPCKey)
{
	uint32 HashResult = GetTypeHash(InRPCKey.GetPacketKey());
	HashResult = HashCombine(HashResult, GetTypeHash(InRPCKey.GetPacketTimeStamp()));
	return HashResult;
}

template<>
struct TStructOpsTypeTraits<FAGPRequestPacketKey> : public TStructOpsTypeTraitsBase2<FAGPRequestPacketKey>
{
	enum
	{
		WithNetSerializer = true
	};
};




USTRUCT(BlueprintType)
struct FAGPCharacterDeathStatus
{
	GENERATED_BODY()
	
public:
	/* 캐릭터 사망 상태 구분 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	EAGPCharacterDeath DeathState = EAGPCharacterDeath::None;

	/* DeatState가 None이 아닐때, 표시할 Death Pose Name */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FName DeathPoseName = NAME_None;

public:
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << DeathState;

		if (EAGPCharacterDeath::None != DeathState)
		{
			Ar << DeathPoseName;
		}
		else
		{ 
			if (Ar.IsLoading())
			{
				DeathPoseName = NAME_None;
			}
		}

		bOutSuccess = true;

		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FAGPCharacterDeathStatus> : public TStructOpsTypeTraitsBase2<FAGPCharacterDeathStatus>
{
	enum
	{
		WithNetSerializer = true
	};
};


class UAnimMontage;

/* 애니메이션 블루프린트에서 DeathPose별로 플레이할 Death, Rebirth Montage */
USTRUCT(BlueprintType)
struct FAGPDeathPoseAnimMontages
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> DeathAnimMontage;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> RebirthAnimMontage;
};




/* 플레이어 캐릭터를 감지한 몬스터들의 상태정보 */
USTRUCT(BlueprintType)
struct FAGPBossStatusInfoCommon
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 AIControllerUniqueID = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float CurrentHealth = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float MaxHealth = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 NameID = -1;

public:
	FAGPBossStatusInfoCommon() = default;

public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	bool Identical(const FAGPBossStatusInfoCommon* Other, uint32 PortFlags) const;

public:
	FString ToString() const;
};

template<>
struct TStructOpsTypeTraits<FAGPBossStatusInfoCommon> : public TStructOpsTypeTraitsBase2<FAGPBossStatusInfoCommon>
{
	enum
	{
		WithNetSerializer = true,
		WithIdentical = true
	};
};

/* Survial GameMode에서의 각 상태 정보 */
USTRUCT(BlueprintType)
struct FAGPSurvialGameModeContext
{
	GENERATED_BODY()

public:
	/* 해당 모드 상태 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	EAGPSurvialGameModeState ModeState = EAGPSurvialGameModeState::None;

	/* 해당 모드의 시작 시간 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	double ModeStartTime = 0.0;

	/* 해당 모드가 종료되기까지 필요한 시간 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	float ModeDuration = 0.f;

	/* 현재 Wave Num, ModeState가 EAGPSurvialGameModeState::InProgressCurrentWave일때 사용 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	int32 CurrentWaveRound = 0;

public:
	FString ToString() const;

public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	bool Identical(const FAGPSurvialGameModeContext* Other, uint32 PortFlags) const;
};

template<>
struct TStructOpsTypeTraits<FAGPSurvialGameModeContext> : public TStructOpsTypeTraitsBase2<FAGPSurvialGameModeContext>
{
	enum
	{
		WithNetSerializer = true,
		WithIdentical = true
	};
};