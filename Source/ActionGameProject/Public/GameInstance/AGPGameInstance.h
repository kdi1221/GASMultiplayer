// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AGPGameInstance.generated.h"

class UAGPUserWidget;

/**
 * GameInstance 클래스
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "LoadingScreen", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAGPUserWidget> LoadingWidgetClass;
	
public:
	virtual void Init() override;

protected:
	virtual void OnPreLoadMap(const FString& MapName);
	virtual void OnDestinationWorldLoaded(UWorld* LoadedWorld);
};
