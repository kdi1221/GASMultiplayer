// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "AGPItemInstance.generated.h"

class UAGPItemFunctionality;
class UAGPItemSlot;
class AAGP_CharacterBase;

/*
 * 아이템 인스턴스 클래스
 */

UCLASS(Abstract, Blueprintable, DefaultToInstanced, CollapseCategories)
class ACTIONGAMEPROJECT_API UAGPItemInstance : public UObject
{
	GENERATED_BODY()
	
protected:
	// 아이템의 Type
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperty", meta = (Categories = "Items.Types"))
	FGameplayTag ItemType;

	//아이템의 고유 기능들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Functionality", meta = (DisplayName = "Functionalities", TitleProperty = EditorPropertyName, ShowOnlyInnerProperties))
	TArray<TObjectPtr<UAGPItemFunctionality>> ItemFunctionality;

protected:
	//현재 아이템의 갯수
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Replicated, Category = "ItemProperty")
	int32 StackNum;

public:
	UAGPItemInstance();

#pragma region[UObject Interface]
public:
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
#pragma endregion

#pragma region[UAGPItemInstance Interface]
public:
	virtual void OnMountedInSlot(const UAGPItemSlot* InSlotInstance);
	virtual void OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance);

public:
	void SetStackNum(const int32 inStackNum);

public:
	template <typename FunctionalClass>
	const FunctionalClass* FindFunctionality() const;

public:
	const UAGPItemFunctionality* FindFunctionality(TSubclassOf<UAGPItemFunctionality> ClassToFind) const;

	FString GetDebugString() const;
#pragma endregion

#pragma region [Only Editor]

#if WITH_EDITORONLY_DATA
private:
	/* IsDataValid 내에서 해당 아이템에 설정된 기능들 및 데이터들의 유효여부 표시(에디터 전용) */
	UPROPERTY(VisibleDefaultsOnly, Transient, Category = "DataValidStatus")
	mutable FText ValidStatusInEditor;
#endif

#if WITH_EDITOR
public:
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

#pragma endregion
};


#pragma region [template Functions]
template <typename FunctionalClass>
const FunctionalClass* UAGPItemInstance::FindFunctionality() const
{
	static_assert(TIsDerivedFrom<FunctionalClass, UAGPItemFunctionality>::IsDerived, "FunctionalClass must be derived from UAGPItemFunctionality");

	return Cast<FunctionalClass>(FindFunctionality(FunctionalClass::StaticClass()));
}
#pragma endregion