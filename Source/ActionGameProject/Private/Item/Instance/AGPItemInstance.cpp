// KJY All Rights Reserved


#include "Item/Instance/AGPItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "Misc/DataValidation.h"
#include "Item/Instance/Functionality/AGPItemFunctionality.h"
#include "Item/Slots/AGPItemSlot.h"
#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

#define LOCTEXT_NAMESPACE "AGPItemInstance"

UAGPItemInstance::UAGPItemInstance()
{
	ItemType = AGPGameplayTags::Items_Types_None;
}

bool UAGPItemInstance::IsSupportedForNetworking() const
{
	return true;
}

void UAGPItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, StackNum, COND_OwnerOnly);
}

void UAGPItemInstance::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITORONLY_DATA
	if (!IsInBlueprint())
	{
		/* C++ Native 클래스 상태일때, Subobject들 중 ItemFunctionalities에 포함되어있지 않은 ItemFunctionality가 있으면 추가해준다 */
		/* 실제로 이럴 경우가 있을지는 좀 더 고민해봐야할듯.. */
		TArray<UObject*> SubObjectes;
		GetDefaultSubobjects(SubObjectes);

		for (UObject* SubObject : SubObjectes)
		{
			if (UAGPItemFunctionality* Functionality = Cast<UAGPItemFunctionality>(SubObject))
			{
				if (!ensureMsgf(ItemFunctionality.Contains(Functionality), TEXT("%s: %s should be added to ItemFunctionalities during the constructor or in PostInitProperties"), *GetName(), *Functionality->GetName()))
				{
					ItemFunctionality.Add(Functionality);
				}
			}
		}

		//에디터에서 AGPItemData 처음 생성 후 호출될때 각 Functionality 유효성 체크 결과 표시(ItemValidStatusInEditor에 대한 Initialize)
		FDataValidationContext DataValidationContext;
		IsDataValid(DataValidationContext);

		//유효성 검사 후 결과 구분 저장
		TArray<FText> Warnings, Errors;
		DataValidationContext.SplitIssues(Warnings, Errors);
		for (const FText& WarningText : Warnings)
		{
			AGP_LOG(LogAGPItem, Warning, TEXT("%s: %s"), *GetName(), *WarningText.ToString());
		}

		for (const FText& ErrorText : Errors)
		{
			AGP_LOG(LogAGPItem, Error, TEXT("%s: %s"), *GetName(), *ErrorText.ToString());
		}
	}
#endif
}

void UAGPItemInstance::PostLoad()
{
	Super::PostLoad();
}

void UAGPItemInstance::OnMountedInSlot(const UAGPItemSlot* InSlotInstance)
{
	verifyf(InSlotInstance, TEXT("[%s], but InSlotInstance Invalid.."), __FUNCTIONW__);

	//AGP_NET_LOG(this, LogAGPItem, Log, TEXT("Mounted In [%s] Slot"), *InSlotInstance->GetDebugString());

	for (const TObjectPtr<UAGPItemFunctionality>& Functionality : ItemFunctionality)
	{
		Functionality->OnMountedInSlot(InSlotInstance);
	}
}

void UAGPItemInstance::OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance)
{
	verifyf(InSlotInstance, TEXT("[%s], but InSlotInstance Invalid.."), __FUNCTIONW__);

	//AGP_NET_LOG(this, LogAGPItem, Log, TEXT("Removed From [%s] Slot"), *InSlotInstance->GetDebugString());

	for (const TObjectPtr<UAGPItemFunctionality>& Functionality : ItemFunctionality)
	{
		Functionality->OnRemovedFromSlot(InSlotInstance);
	}
}

void UAGPItemInstance::SetStackNum(const int32 inStackNum)
{
	StackNum = inStackNum;
}

const UAGPItemFunctionality* UAGPItemInstance::FindFunctionality(TSubclassOf<UAGPItemFunctionality> ClassToFind) const
{
	for (const TObjectPtr<UAGPItemFunctionality>& Functionality : ItemFunctionality)
	{
		if (Functionality && Functionality->IsA(ClassToFind))
		{
			return Functionality;
		}
	}

	return nullptr;
}

FString UAGPItemInstance::GetDebugString() const
{
	return GetNameSafe(this) + TEXT(", StackNum[") + FString::FromInt(StackNum) + TEXT("]");
}

#if WITH_EDITOR

EDataValidationResult UAGPItemInstance::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult ValidationResult = Super::IsDataValid(Context);

	if (ValidationResult != EDataValidationResult::Invalid)
	{
		/* 아이템 타입 유효 검사 */
		/*if (!ItemType.IsValid() || ItemType == AGPGameplayTags::Items_Types_None)
		{
			Context.AddError(LOCTEXT("Item Type Invalid", "Item Type Invalid"));
			ValidationResult = EDataValidationResult::Invalid;
			return ValidationResult;
		}*/

		//장비 기능들 유효한지 체크
		for (const UAGPItemFunctionality* Functionality : ItemFunctionality)
		{
			if (Functionality)
			{
				ValidationResult = Functionality->IsDataValid(Context);
				if (ValidationResult == EDataValidationResult::Invalid)
				{
					break;
				}
			}
			else
			{
				Context.AddWarning(LOCTEXT("FunctionalityIsNull", "Null entry in Functionality"));
			}
		}
	}

	/* 수집된 경고, 에러 메시지 구분 */
	TArray<FText> Warnings, Errors;
	Context.SplitIssues(Warnings, Errors);

	if (Errors.Num() > 0)
	{
		ValidStatusInEditor = FText::FormatOrdered(LOCTEXT("ErrorsFmt", "Error: {0}"), FText::Join(FText::FromStringView(TEXT(", ")), Errors));
	}
	else if (Warnings.Num() > 0)
	{
		ValidStatusInEditor = FText::FormatOrdered(LOCTEXT("WarningsFmt", "Warning: {0}"), FText::Join(FText::FromStringView(TEXT(", ")), Warnings));
	}
	else
	{
		ValidStatusInEditor = LOCTEXT("AllOk", "All Ok");
	}

	return ValidationResult;
}

#endif

#undef LOCTEXT_NAMESPACE