// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AGP_GameModeBase.generated.h"

class AAGP_NPCBase;
struct FActorSpawnParameters;

/**
 * 
 */
UCLASS()
class ACTIONGAMEPROJECT_API AAGP_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

public:
	/* 해당 클래스의 NPC Spawn */
	UFUNCTION(BlueprintCallable)
	AAGP_NPCBase* SpawnNPC(TSubclassOf<AAGP_NPCBase> SpawnNPCClass, const FTransform& SpawnTransform, const ESpawnActorCollisionHandlingMethod CollisionHandlingMethod);

protected:
	virtual void OnSpawnedNPC(AAGP_NPCBase* SpawnedNPC);

protected:
	/* 현재 월드상의 타이머 매니저 반환 */
	FTimerManager& GetCurrentTimerManager() const;

protected:
	void KickAllPlayers();
	void ShutdownGame();
};
