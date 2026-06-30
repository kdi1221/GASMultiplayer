// KJY All Rights Reserved

#include "Common/AGPCommonStructs.h"

FAGPTagContainerKey::FAGPTagContainerKey()
{

}

uint32 FAGPTagContainerKey::GetKeyHash() const
{
	TArray<FGameplayTag> TagsInContainer(TagContainer.GetGameplayTagArray());
	TagsInContainer.Sort();

	uint32 CombinedHash = 0;
	for (const FGameplayTag& InTag : TagsInContainer)
	{
		CombinedHash = HashCombine(CombinedHash, GetTypeHash(InTag));
	}

	return CombinedHash;
}


int32 FAGPRequestPacketKeyGenerator::CurrentBaseFeaturePacketKey = FAGPRequestPacketKeyGenerator::INVALID_REQUEST_PACKET_KEY;
int32 FAGPRequestPacketKeyGenerator::NextBaseFeaturePacketKey = 1;

FAGPRequestPacketKey FAGPRequestPacketKeyGenerator::GenerateRequestPacketKey()
{
	/* 이번 요청에 대한 Packet Key 생성 및 다음번 Packet Key에 대한 증가(int32 limitMax이후에는 다시 1부터 시작) */
	CurrentBaseFeaturePacketKey = NextBaseFeaturePacketKey++;
	if (NextBaseFeaturePacketKey <= 0)
	{
		NextBaseFeaturePacketKey = 1;
	}

	return FAGPRequestPacketKey(CurrentBaseFeaturePacketKey, FDateTime::UtcNow().GetTicks());
}

bool FAGPBossStatusInfoCommon::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << AIControllerUniqueID;
	Ar << CurrentHealth;
	Ar << MaxHealth;
	Ar << NameID;

	bOutSuccess = true;

	return true;
}

bool FAGPBossStatusInfoCommon::Identical(const FAGPBossStatusInfoCommon* Other, uint32 PortFlags) const
{
	if (!Other)
	{
		return false;
	}

	const bool bSameNPCAIController = AIControllerUniqueID == Other->AIControllerUniqueID;
	const bool bSameCurrentHealth = CurrentHealth == Other->CurrentHealth;
	const bool bSameMaxHealth = MaxHealth == Other->MaxHealth;
	const bool bSameNameID = NameID == Other->NameID;

	return bSameNPCAIController && bSameCurrentHealth && bSameMaxHealth && bSameNameID;
}

FString FAGPBossStatusInfoCommon::ToString() const
{
	return FString::Printf(TEXT("UniqueID[%d], Health[%f / %f], NameID[%d]"), AIControllerUniqueID, CurrentHealth, MaxHealth, NameID);
}

FString FAGPSurvialGameModeContext::ToString() const
{
	return FString::Printf(TEXT("State[%s], ModeStartTime[%lf], Duration[%.2f], WaveRound[%d]"), *UEnum::GetDisplayValueAsText(ModeState).ToString(), ModeStartTime, ModeDuration, CurrentWaveRound);
}

bool FAGPSurvialGameModeContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << ModeState;
	Ar << ModeStartTime;
	Ar << ModeDuration;

	/* 한 웨이브 흐름(Wait => Pre => Progress => Complete)내에서만 리플리케이션 */
	if (ModeState >= EAGPSurvialGameModeState::WaitForNextWave &&
		ModeState <= EAGPSurvialGameModeState::CompleteCurrentWave)
	{
		Ar << CurrentWaveRound;
	}

	bOutSuccess = true;

	return true;
}

bool FAGPSurvialGameModeContext::Identical(const FAGPSurvialGameModeContext* Other, uint32 PortFlags) const
{
	if (!Other)
	{
		return false;
	}

	const bool bSameModeState = ModeState == Other->ModeState;
	const bool bSameModeStartTime = ModeStartTime == Other->ModeStartTime;
	const bool bSameModeDuration = ModeDuration == Other->ModeDuration;

	if (!bSameModeState || !bSameModeStartTime || !bSameModeDuration)
	{
		return false;
	}

	/* 한 웨이브 흐름(Wait => Pre => Progress => Complete)일때 WaveRound 체크 */
	if (ModeState >= EAGPSurvialGameModeState::WaitForNextWave &&
		ModeState <= EAGPSurvialGameModeState::CompleteCurrentWave)
	{
		return CurrentWaveRound == Other->CurrentWaveRound;
	}

	return true;
}