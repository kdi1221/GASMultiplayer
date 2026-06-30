// KJY All Rights Reserved


#include "Item/Instance/Functionality/AGPItemFunctionality.h"
#include "Item/Instance/AGPItemInstance.h"
#include "Misc/DataValidation.h"
#include "Log/AGPLogChannels.h"

#define LOCTEXT_NAMESPACE "AGPItemFunctionality"

UAGPItemFunctionality::UAGPItemFunctionality()
{
#if WITH_EDITORONLY_DATA
	EditorPropertyName = FName::NameToDisplayString(GetNameSafe(GetClass()), false);
	EditorPropertyName.ReplaceInline(TEXT("AGP"), TEXT(""));
#endif
}

UAGPItemInstance* UAGPItemFunctionality::GetOwner() const
{
	return GetTypedOuter<UAGPItemInstance>();
}

void UAGPItemFunctionality::OnMountedInSlot(const UAGPItemSlot* InSlotInstance)
{

}

void UAGPItemFunctionality::OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance)
{

}

#if WITH_EDITOR
EDataValidationResult UAGPItemFunctionality::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	TSubclassOf<UAGPItemFunctionality> ThisFunctionalityClassType = GetClass();

	const UAGPItemInstance* OwnerItemInstance = GetOwner();

	if (OwnerItemInstance)
	{
		const UAGPItemFunctionality* FindFirstFunctionalityThisType = OwnerItemInstance->FindFunctionality(ThisFunctionalityClassType);

		if (!FindFirstFunctionalityThisType)
		{
			/* Owner에 이 아이템 기능이 존재하지 않음 */
			Context.AddError(LOCTEXT("FunctionalNotOnItem", "Functionality does not exist in its Owner's EquipmentFunctionality"));
			Result = EDataValidationResult::Invalid;

		}
		else if (FindFirstFunctionalityThisType != this)
		{
			/* 한 아이템 정보 안에는 하나의 아이템 기능만 포함 가능 */
			Context.AddError(FText::FormatOrdered(LOCTEXT("MultipleFunctionalitiesOfSameTypeError", "Two or more types of {0} exist on Owner Item"), FText::FromString(ThisFunctionalityClassType->GetName())));
			Result = EDataValidationResult::Invalid;
		}
	}
	else
	{
		/* Outer는 반드시 EquipmentData 이어야 한다. */
		FText ErrorText = FText::FormatOrdered(LOCTEXT("No EquipmentData Owner", "{0} has invalid Outer: {1}"), FText::FromString(GetNameSafe(this)), FText::FromString(GetNameSafe(GetOuter())));
		Context.AddError(ErrorText);

		AGP_LOG(LogAGPItem, Error, TEXT("%s"), *ErrorText.ToString());

		Result = EDataValidationResult::Invalid;
	}

	//여기까지 오면 일단 AGPItemFunctionalityBase내에서는 오류가 없는것이므로 Valid로 Return할수있게 한다.
	if (Result == EDataValidationResult::NotValidated)
	{
		Result = EDataValidationResult::Valid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE