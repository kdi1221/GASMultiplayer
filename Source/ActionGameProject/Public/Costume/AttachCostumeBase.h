// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AGPCostumeEnum.h"
#include "GameplayTagContainer.h"
#include "AttachCostumeBase.generated.h"

class AAGPCostumeActorBase;

DECLARE_DELEGATE_OneParam(FCostumeActorSpawnDelegate, AAGPCostumeActorBase* /*InSpawnedCostumeActor*/);

USTRUCT()
struct FAGPCostumeActorToSpawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "AGP|Costume")
	TSubclassOf<AAGPCostumeActorBase> AttachedActorClass = nullptr;
};

/**
 * 캐릭터에 붙는 Costume들에 대한 클래스
 */

UCLASS(Abstract, Blueprintable)
class ACTIONGAMEPROJECT_API UAttachCostumeBase : public UObject
{
	GENERATED_BODY()	

protected:
	UPROPERTY(EditDefaultsOnly, meta = (ForceInlineRow, ShowOnlyInnerProperties), Category = "AGP|Costume")
	TMap<FName, FAGPCostumeActorToSpawn> SpawnComstumeActorClassesMap;

	/* Costume의 Type */
	UPROPERTY(VisibleInstanceOnly)
	EAGPCostumeType CostumeType;

	/* CostumeType이 FromEquipment일때 사용되는 Costume Instnace 구분 변수(장비한 슬롯에 대한 Tag) */
	UPROPERTY(VisibleInstanceOnly, meta = (EditCondition = "CostumeType == EAGPCostumeType::FromEquipment", EditConditionHides))
	FGameplayTag SoruceEquipmentSlotTag;

public:
	UAttachCostumeBase();

#pragma region[UObject Interface]
public:
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginDestroy() override;
	virtual void FinishDestroy() override;
#pragma endregion

public:
	void SetCostumeType(const EAGPCostumeType InCostumeType);
	void SetCostumeEquipmentSlot(const FGameplayTag& InCostumeEquipmentSlotTag);
	void SpawnCostumeActors(AActor* InAttachTargetActor, APawn* InCostumeActorInstigator, const FCostumeActorSpawnDelegate& InOnSpawnDelegate);


public:
	FORCEINLINE EAGPCostumeType GetCostumeType() const { return CostumeType; }
	FORCEINLINE const FGameplayTag& GetSoruceEquipmentSlotTag() const { return SoruceEquipmentSlotTag; }
};