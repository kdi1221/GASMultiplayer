// KJY All Rights Reserved


#include "GameMode/AGP_GameModeBase.h"
#include "Characters/AGP_NPCBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameMapsSettings.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

FString AAGP_GameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	if (!ErrorMessage.IsEmpty())
	{
		return ErrorMessage;
	}

	/* 플레이어가 지정한 닉네임 설정 */
	FString NewPlayerName = UGameplayStatics::ParseOption(Options, TEXT("PlayerCustomName"));
	if (!NewPlayerName.IsEmpty())
	{
		ChangeName(NewPlayerController, NewPlayerName, false);
	}

	return ErrorMessage;
}

AAGP_NPCBase* AAGP_GameModeBase::SpawnNPC(TSubclassOf<AAGP_NPCBase> SpawnNPCClass, const FTransform& SpawnTransform, const ESpawnActorCollisionHandlingMethod CollisionHandlingMethod)
{
	UWorld* CurrentWorld = GetWorld();
	checkf(CurrentWorld, TEXT("[%s], Invalid CurrentWorld"), __FUNCTIONW__);

	FActorSpawnParameters SpawnNPCParamters;
	SpawnNPCParamters.SpawnCollisionHandlingOverride = CollisionHandlingMethod;

	AAGP_NPCBase* SpawnedNPC = CurrentWorld->SpawnActor<AAGP_NPCBase>(SpawnNPCClass, SpawnTransform, SpawnNPCParamters);
	
	if (SpawnedNPC)
	{
		OnSpawnedNPC(SpawnedNPC);
	}
	
	return SpawnedNPC;
}

void AAGP_GameModeBase::OnSpawnedNPC(AAGP_NPCBase* SpawnedNPC)
{
	checkf(SpawnedNPC, TEXT("[%s]. Invalid SpawnedNPC"), __FUNCTIONW__);

	// Intro 알림 및 Intro몽타주 표시를 위한 SkeletalMesh Bone 갱신
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(SpawnedNPC, AGPGameplayTags::Event_Character_Ability_Spawn_Intro, FGameplayEventData());
	if (SpawnedNPC->HasTagInASC(AGPGameplayTags::State_SpawnIntro))
	{
		SpawnedNPC->RefreshSkeletalMeshBone(true);
	}

	//TODO : GameMode에서 등록?(EX : Survial Mode)
}

FTimerManager& AAGP_GameModeBase::GetCurrentTimerManager() const
{
	UWorld* CurrentWorld = GetWorld();
	checkf(CurrentWorld, TEXT("[%s], Invalid CurrentWorld"), __FUNCTIONW__);

	return CurrentWorld->GetTimerManager();
}

void AAGP_GameModeBase::KickAllPlayers()
{
	/* 남은 플레이어들 쫓아내기 */
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("Game Exit")));
		}
	}
}

void AAGP_GameModeBase::ShutdownGame()
{
	//에디터 실행환경에서는 별도로 서버 종료를 하지 않음
	if (GIsEditor)
	{
		AGP_LOG(LogAGPGameMode, Log, TEXT("Shutdown Server, Play In Editor.."));
		return;
	}

	//종료
	FGenericPlatformMisc::RequestExit(false);
}
