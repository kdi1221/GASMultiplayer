// KJY All Rights Reserved


#include "Item/Instance/Functionality/AGPItemFunc_BindInput.h"
#include "Misc/DataValidation.h"
#include "Item/Slots/AGPEquipmentSlot.h"
#include "Item/Instance/AGPItemInstance.h"

#include "Player/AGP_PlayerStateBase.h"
#include "Characters/AGP_PlayerCharacter.h"

#include "Player/AGP_PlayerControllerBase.h"

#include "AGPGameplayTags.h"
#include "Log/AGPLogChannels.h"

#define LOCTEXT_NAMESPACE "AGPItemFunc_BindInput"

UAGPItemFunc_BindInput::UAGPItemFunc_BindInput()
{
#if WITH_EDITORONLY_DATA
	EditorPropertyName = TEXT("BindInput");
#endif
}

void UAGPItemFunc_BindInput::OnMountedInSlot(const UAGPItemSlot* InSlotInstance)
{
	verifyf(InSlotInstance, TEXT("[%s] - Invalid InSlotInstance"), __FUNCTIONW__);

	//장비 슬롯일때 처리
	if (InSlotInstance->GetSlotCategory().MatchesTagExact(AGPGameplayTags::Items_Slots_Equipment))
	{
		AAGP_PlayerControllerBase* OwnerLocalPlayerController = GetLocalPlayerControllerFromSlotOwner(InSlotInstance->GetSlotOwner<AActor>());
		if (OwnerLocalPlayerController)
		{
			//AGP_NET_LOG(this, LogAGPItem, Log, TEXT("Local PlayerController[%s], Binding Input Item[%s]"), *GetNameSafe(OwnerLocalPlayerController), *GetNameSafe(GetOwner()));

			BindingInputHandles.Empty();
			OwnerLocalPlayerController->BindInputConfig(InputConfig, BindingInputHandles);
		}
	}
}

void UAGPItemFunc_BindInput::OnRemovedFromSlot(const UAGPItemSlot* InSlotInstance)
{
	verifyf(InSlotInstance, TEXT("[%s] - Invalid InSlotInstance"), __FUNCTIONW__);

	//장비 슬롯일때 처리
	if (InSlotInstance->GetSlotCategory().MatchesTagExact(AGPGameplayTags::Items_Slots_Equipment))
	{
		AAGP_PlayerControllerBase* OwnerLocalPlayerController = GetLocalPlayerControllerFromSlotOwner(InSlotInstance->GetSlotOwner<AActor>());
		if (OwnerLocalPlayerController)
		{
			OwnerLocalPlayerController->UnbindInputConfig(InputConfig, BindingInputHandles);
		}
	}
}

/* Owner들을 탐색해서 가장 상위 Owner Actor를 가져온다.(AActor::HasLocalNetOwner 참고) */
AAGP_PlayerControllerBase* UAGPItemFunc_BindInput::GetLocalPlayerControllerFromSlotOwner(AActor* InSlotOwnerActor) const
{
	if (!InSlotOwnerActor)
	{
		return nullptr;
	}

	AActor* TopOwner = InSlotOwnerActor;
	if (AActor* CheckOwner = InSlotOwnerActor->GetOwner())
	{
		for (TopOwner = CheckOwner; TopOwner->GetOwner(); TopOwner = TopOwner->GetOwner())
		{

		}
	}

	if (TopOwner->IsA<APawn>())
	{
		const APawn* Pawn = CastChecked<APawn>(TopOwner);
		return Pawn->GetController<AAGP_PlayerControllerBase>();
	}

	if (AAGP_PlayerControllerBase* OwnerPlayerController = Cast<AAGP_PlayerControllerBase>(TopOwner))
	{
		return OwnerPlayerController->IsLocalController() ? OwnerPlayerController : nullptr;
	}
	
	return nullptr;
}

#if WITH_EDITOR
EDataValidationResult UAGPItemFunc_BindInput::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	if (Result != EDataValidationResult::Valid)
	{
		return Result;
	}

	if (!InputConfig)
	{
		Context.AddError(LOCTEXT("InputConfig Invalid", "InputConfig Invalid.."));
		Result = EDataValidationResult::Invalid;
		return Result;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE