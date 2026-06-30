// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AGPItemFunctionality.generated.h"

class UAGPItemInstance;
class UAGPItemSlot;
class AAGP_CharacterBase;

/**
 * 아이템 각 고유 기능 정의 
 */

UCLASS(Abstract, Const, EditInlineNew, DefaultToInstanced, CollapseCategories, Within = AGPItemInstance)
class ACTIONGAMEPROJECT_API UAGPItemFunctionality : public UObject
{
	GENERATED_BODY()
	
public:
	UAGPItemFunctionality();

protected:
	UAGPItemInstance* GetOwner() const;

public:
	virtual void OnMountedInSlot(const UAGPItemSlot* InSlotInstance);
	virtual void OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance);

#if WITH_EDITORONLY_DATA
protected:
	/* 에디터 상에서 해당 아이템 기능이 표시될때 이름 */
	UPROPERTY(VisibleDefaultsOnly, Transient, Category = AlwaysHidden, Meta = (EditCondition = False, EditConditionHides))
	FString EditorPropertyName;
#endif	

#if WITH_EDITOR
public:
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
