// KJY All Rights Reserved


#include "AGPTagContainerKeyCustomization.h"
#include "DetailWidgetRow.h"
#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "Widgets/Input/SHyperlink.h"
#include "SGameplayTagContainerCombo.h"
#include "HAL/PlatformApplicationMisc.h"
#include "ScopedTransaction.h"
//#include "GameplayTagEditorUtilities.h"
#include "GameplayTagsEditor\Private\GameplayTagEditorUtilities.h"
#include "SGameplayTagPicker.h"

#define LOCTEXT_NAMESPACE "AGPTagContainerKeyCustomization"

TSharedRef<IPropertyTypeCustomization> FAGPTagContainerKeyCustomization::MakeInstance()
{
	return MakeShareable(new FAGPTagContainerKeyCustomization());
}

FAGPTagContainerKeyCustomization::FAGPTagContainerKeyCustomization()
{

}

void FAGPTagContainerKeyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;
	TagContainerProperty = StructPropertyHandle->GetChildHandle(0);

	HeaderRow
		.NameContent()
		[
			TagContainerProperty->CreatePropertyNameWidget()
		]
		.ValueContent()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
				.Padding(FMargin(0, 2, 0, 1))
				[
					SNew(SGameplayTagContainerCombo)
						.PropertyHandle(TagContainerProperty)
				]
		];
}

#undef LOCTEXT_NAMESPACE