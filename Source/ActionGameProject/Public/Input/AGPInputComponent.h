// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "AGPInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "AGPInputComponent.generated.h"


template <class UserClass>
struct FAGPInputBindFunctions
{
public:
	const TMap<ETriggerEvent, void (UserClass::*)(const FInputActionValue&, FGameplayTag)> BindFunctions;

public:
	FAGPInputBindFunctions(const std::initializer_list<TPairInitializer<const ETriggerEvent&, void (UserClass::* const&)(const FInputActionValue&, FGameplayTag)>>& InArguments)
		:BindFunctions(InArguments)
	{

	}
};

USTRUCT()
struct FAGPInputBindingHandles
{
	GENERATED_BODY()
	
public:
	TArray<uint32> BindingInputHandles;
};

/**
 * 
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template<class UserClass>
	void BindingInputConfig(const UAGPInputConfig* InInputConfig, APlayerController* InPlayerController, UserClass* InContextObject, const FAGPInputBindFunctions<UserClass>& InNativeInputBindFunctions, const FAGPInputBindFunctions<UserClass>& InAbilityInputBindFunctions);

	template<class UserClass>
	void BindingInputConfig(const UAGPInputConfig* InInputConfig, APlayerController* InPlayerController, UserClass* InContextObject, const FAGPInputBindFunctions<UserClass>& InNativeInputBindFunctions, const FAGPInputBindFunctions<UserClass>& InAbilityInputBindFunctions, TArray<uint32>& OutBindingHandles);

	void UnbindingInputMapping(const UAGPInputConfig* InInputConfig, const TArray<uint32>& InBindingHandles, APlayerController* InPlayerController);

private:
	template<class UserClass>
	void BindInputFunctions(UserClass* InContextObject, const TMap<FGameplayTag, FAGPInputBindInfo>& InMapInputActions, const FAGPInputBindFunctions<UserClass>& InInputBindFunctions, TArray<uint32>& OutBindingHandles);

	template<class UserClass>
	uint32 BindFunctionToAction(UserClass* InContextObject, const FAGPInputBindInfo& InBindInfo, const FAGPInputBindFunctions<UserClass>& InBindFunctions, const ETriggerEvent InTriggerEvent, const FGameplayTag& InInputTag);
};

template<class UserClass>
void UAGPInputComponent::BindingInputConfig(const UAGPInputConfig* InInputConfig, APlayerController* InPlayerController, UserClass* InContextObject, const FAGPInputBindFunctions<UserClass>& InNativeInputBindFunctions, const FAGPInputBindFunctions<UserClass>& InAbilityInputBindFunctions)
{
	TArray<uint32> BindingInputHandles;

	BindingInputConfig(InInputConfig, InPlayerController, InContextObject, InNativeInputBindFunctions, InAbilityInputBindFunctions, BindingInputHandles);
}

/*
	InputConfig 기반으로 MappingContext를 등록하고
	Native / Ability 입력을 분리하여 바인딩
*/
template<class UserClass>
void UAGPInputComponent::BindingInputConfig(
	const UAGPInputConfig* InInputConfig, 
	APlayerController* InPlayerController, 
	UserClass* InContextObject, 
	const FAGPInputBindFunctions<UserClass>& InNativeInputBindFunctions, 
	const FAGPInputBindFunctions<UserClass>& InAbilityInputBindFunctions, 
	TArray<uint32>& OutBindingHandles)
{
	checkf(InInputConfig, TEXT("Input config invalid, can not proceed with binding"));
	checkf(InPlayerController, TEXT("PlayerController invalid, can not proceed with binding"));

	UEnhancedInputLocalPlayerSubsystem* Subsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			InPlayerController->GetLocalPlayer());
	checkf(Subsystem, TEXT("Invalid EnhancedInput Subsystem, can not Input binding"));

	/* Input Mapping Context 추가 */
	Subsystem->AddMappingContext(InInputConfig->MappingContext, 
								static_cast<int32>(InInputConfig->MappingPriority));

	//Native Input Actions Binding
	BindInputFunctions(InContextObject, 
						InInputConfig->NativeInputActions, 
						InNativeInputBindFunctions, 
						OutBindingHandles);

	//Ability Input Actions Binding
	BindInputFunctions(InContextObject, 
						InInputConfig->AbilityInputActions, 
						InAbilityInputBindFunctions, 
						OutBindingHandles);
}

template<class UserClass>
void UAGPInputComponent::BindInputFunctions(UserClass* InContextObject, const TMap<FGameplayTag, FAGPInputBindInfo>& InMapInputActions, const FAGPInputBindFunctions<UserClass>& InInputBindFunctions, TArray<uint32>& OutBindingHandles)
{
	auto CallBindFunctionLambda = [&](UserClass* InContextObject, const FAGPInputBindInfo& InBindInfo, const FAGPInputBindFunctions<UserClass>& InBindFunctions, const ETriggerEvent InTriggerEvent, const FGameplayTag& InInputTag)
	{
		const uint32 BindingInputHandle = BindFunctionToAction(InContextObject, InBindInfo, InInputBindFunctions, InTriggerEvent, InInputTag);
		if (0 == BindingInputHandle)
		{
			return;
		}

		verifyf(INDEX_NONE == OutBindingHandles.Find(BindingInputHandle), TEXT("[%s] - Already Binding Input Handle, InputTag[%s], HandleIndex[%d]"), __FUNCTIONW__, *InInputTag.ToString(), BindingInputHandle);

		OutBindingHandles.AddUnique(BindingInputHandle);
	};

	for (const TPair<FGameplayTag, FAGPInputBindInfo>& MapInputPair : InMapInputActions)
	{
		const FGameplayTag& InputTag = MapInputPair.Key;
		const FAGPInputBindInfo& InputBindInfo = MapInputPair.Value;

		CallBindFunctionLambda(InContextObject, InputBindInfo, InInputBindFunctions, ETriggerEvent::Started, InputTag);
		CallBindFunctionLambda(InContextObject, InputBindInfo, InInputBindFunctions, ETriggerEvent::Triggered, InputTag);
		CallBindFunctionLambda(InContextObject, InputBindInfo, InInputBindFunctions, ETriggerEvent::Completed, InputTag);
		CallBindFunctionLambda(InContextObject, InputBindInfo, InInputBindFunctions, ETriggerEvent::Ongoing, InputTag);
		CallBindFunctionLambda(InContextObject, InputBindInfo, InInputBindFunctions, ETriggerEvent::Canceled, InputTag);
	}
}

template<class UserClass>
uint32 UAGPInputComponent::BindFunctionToAction(UserClass* InContextObject, const FAGPInputBindInfo& InBindInfo, const FAGPInputBindFunctions<UserClass>& InBindFunctions, const ETriggerEvent InTriggerEvent, const FGameplayTag& InInputTag)
{
	if (!InBindInfo.IsTriggerEventFlagOn(InTriggerEvent))
	{
		return 0;
	}

	void (UserClass::* const& BindingFunc)(const FInputActionValue&, FGameplayTag) = InBindFunctions.BindFunctions.FindChecked(InTriggerEvent);
	const FEnhancedInputActionEventBinding& BindingInfo = BindAction(InBindInfo.BindInputAction, InTriggerEvent, InContextObject, BindingFunc, InInputTag);

	return BindingInfo.GetHandle();
}