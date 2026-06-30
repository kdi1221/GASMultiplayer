// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Widget/WidgetController/AGPWidgetController.h"
#include "AGPHeadUPWidgetController.generated.h"

struct FOnAttributeChangeData;
class AAGP_CharacterBase;

/**
 *  캐릭터 머리 위 상태 표시(ex : HP Bar)
 */

UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPHeadUPWidgetController : public UAGPWidgetController
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStringDisplayNameChanged, const FText&, CurrentDisplayName);
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthValueChanged;

	UPROPERTY(BlueprintAssignable)
	FOnStringDisplayNameChanged OnCurrentDisplayNameChanged;
	
private:
	float CurrentHealthValue = 0.f;
	float MaxHealthValue = 0.f;
	FText CurrentStringDisplayName;
	
public:
	void SetOwnerCharacter(AAGP_CharacterBase* InCharacter);
	void UninitializeOwnerChracter(AAGP_CharacterBase* InCharacter);
	void SetCurrentDisplayName(const FText& NewName);

private:
	void OnChangeCurrentHealth(const FOnAttributeChangeData& Data);
	void OnChangeMaxHealth(const FOnAttributeChangeData& Data);

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentHeatlhValue() const {return CurrentHealthValue;}

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHeatlhValue() const {return MaxHealthValue;}

	UFUNCTION(BlueprintPure)
	FORCEINLINE FText GetDisplayNameText() const { return CurrentStringDisplayName; }
};
