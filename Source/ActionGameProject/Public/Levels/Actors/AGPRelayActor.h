// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/AGPCommonEnums.h"
#include "GenericTeamAgentInterface.h"
#include "AGPRelayActor.generated.h"

class USphereComponent;

/* 월드상에 배치되어 스폰되는 NPC들에게 플레이어 존재 알림 */
UCLASS(Abstract, Blueprintable)
class ACTIONGAMEPROJECT_API AAGPRelayActor : public AActor
											, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditInstanceOnly, Category = "Initialize", meta = (AllowPrivateAccess = "true"))
	EAGPTeamGroup NotifyTeamGroup = EAGPTeamGroup::NonPlayerGroup_1;

	UPROPERTY(EditInstanceOnly, Category = "Initialize", meta = (AllowPrivateAccess = "true"))
	float NotifyRange = 1600.f;

	UPROPERTY(EditInstanceOnly, Category = "Initialize", meta = (AllowPrivateAccess = "true"))
	float InfoAge = 0.f;

	UPROPERTY(EditInstanceOnly, Category = "Initialize", meta = (AllowPrivateAccess = "true"))
	float Strength = 1.f;

#if WITH_EDITORONLY_DATA
private:
	/* 에디터 상에 Sphere 표시 */
	UPROPERTY(VisibleInstanceOnly, Category = "Editor")
	TObjectPtr<USphereComponent> RangeSphereForEditor;
#endif // WITH_EDITORONLY_DATA
	
public:	
	AAGPRelayActor();

public:
	virtual FGenericTeamId GetGenericTeamId() const override;

public:
	void NotifyPlayerExistenceToNPC(APawn* InPlayerPawn);

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
