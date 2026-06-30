// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "AGPGameDataTableSubsystem.generated.h"

struct FAGPAbilityData;
class UAGPAbilityDataPrimaryAsset;

/**
 *  게임 내 데이터 테이블들 관리 
 */

UCLASS(Blueprintable)
class ACTIONGAMEPROJECT_API UAGPGameDataTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Ability"))
	TObjectPtr<UAGPAbilityDataPrimaryAsset> AbilityDataPrimaryAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Monster", RowType = "/Script/ActionGameProject.FAGPMonsterNameData"))
	TObjectPtr<UDataTable> MonsterNameTable;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const;

public:
	UFUNCTION(BlueprintPure, Meta = (AutoCreateRefTerm = "AbilityTag"))
	const FAGPAbilityData& GetAbilityData(const FGameplayTag& AbilityTag) const;

	UFUNCTION(BlueprintPure, Meta = (Categories = "Monster"))
	const FText& GetMonsterNameText(const int32 InRowIndex) const;
};
