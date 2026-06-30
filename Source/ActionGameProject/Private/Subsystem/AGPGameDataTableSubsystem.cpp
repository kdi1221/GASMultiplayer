// KJY All Rights Reserved


#include "Subsystem/AGPGameDataTableSubsystem.h"
#include "GameData/AGPAbilityDataPrimaryAsset.h"
#include "GameData/AGPMonsterData.h"

void UAGPGameDataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//UE_LOG(LogTemp, Log, TEXT("[%s], TestValue[%d]"), __FUNCTIONW__, TestValue);
}

void UAGPGameDataTableSubsystem::Deinitialize()
{

}

bool UAGPGameDataTableSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{	
	/* 블루프린트에서 정의한 DataTable 서브시스템만 생성 가능 */
	return IsInBlueprint();
}

const FAGPAbilityData& UAGPGameDataTableSubsystem::GetAbilityData(const FGameplayTag& AbilityTag) const
{
	static const FAGPAbilityData DummyData;

	if (!AbilityDataPrimaryAsset)
	{
		return DummyData;
	}

	const FAGPAbilityData* FindAbilityData = AbilityDataPrimaryAsset->AbilityDataMap.Find(AbilityTag);
	if (!FindAbilityData)
	{
		return DummyData;
	}

	

	return *FindAbilityData;
}

const FText& UAGPGameDataTableSubsystem::GetMonsterNameText(const int32 InRowIndex) const
{
	static const FText DummyText(FText::FromString(TEXT("Invalid Name")));

	if (!MonsterNameTable)
	{
		return DummyText;
	}

	FName FindRowName(*FString::FromInt(InRowIndex));

	FAGPMonsterNameData* FindRow = MonsterNameTable->FindRow<FAGPMonsterNameData>(FindRowName, TEXT(""));

	return FindRow ? FindRow->MonsterName : DummyText;
}
