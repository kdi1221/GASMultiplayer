// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGameProjectEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "AGPTagContainerKeyCustomization.h"
#include "UnrealEd.h"

IMPLEMENT_GAME_MODULE(FActionGameProjectEditorModule, ActionGameProjectEditor);

void FActionGameProjectEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout("AGPTagContainerKey", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAGPTagContainerKeyCustomization::MakeInstance));

	//UE_LOG(LogTemp, Log, TEXT("[%s]"), __FUNCTIONW__);
}

void FActionGameProjectEditorModule::ShutdownModule()
{
	if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyEditorModule->UnregisterCustomPropertyTypeLayout("AGPTagContainerKey");
	}

	//UE_LOG(LogTemp, Log, TEXT("[%s]"), __FUNCTIONW__);
}

