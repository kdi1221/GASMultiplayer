// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Costume/AGPCostumeEnum.h"
#include "GameplayTagContainer.h"
#include "AGPCostumeActorBase.generated.h"

struct FGameplayTag;
class UAGPCharPresentationComponent;

class UShapeComponent;

UCLASS(Abstract)
class ACTIONGAMEPROJECT_API AAGPCostumeActorBase : public AActor
{
	GENERATED_BODY()
	
private:
	/* Costume Actor 이름 */
	UPROPERTY(VisibleInstanceOnly)
	FName CostumeName;

	/* Owner Costume Instance로부터 설정된 Type */
	UPROPERTY(VisibleInstanceOnly)
	EAGPCostumeType CostumeType;

	/* CostumeType이 FromEquipment일때 사용되는 Costume Instnace 구분 변수(장비한 슬롯에 대한 Tag) */
	UPROPERTY(VisibleInstanceOnly, meta = (EditCondition = "CostumeType == EAGPCostumeType::FromEquipment", EditConditionHides))
	FGameplayTag SoruceEquipmentSlotTag;

	/* Costume Actor의 현재 상태(Actor별로 고유 상태에 대한 명칭 지정) */
	UPROPERTY(VisibleInstanceOnly)
	FName CurrentStatus;

private:
	TWeakObjectPtr<UAGPCharPresentationComponent> OwnerPresentationComponent;

public:	
	AAGPCostumeActorBase();
	
public:
	void SetCostumeActorName(const FName& InName);
	void SetCostumeType(const EAGPCostumeType InCostumeType);
	void SetCostumeEquipmentSlot(const FGameplayTag& InCostumeEquipmentSlotTag);

	void AttachToOwner(UAGPCharPresentationComponent* InCharacterPresentationComponent);
	
public:
	UFUNCTION(BlueprintCallable, Meta = (AutoCreateRefTerm = "InNewStatus"))
	void SetCurrentStatus(const FName& InNewStatus);

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnNotifyOwnerGameplayTagEvents(const FGameplayTag& InEventTag, const bool InAdded);

	UFUNCTION(BlueprintImplementableEvent)
	void OnOwnerHitEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void OnOwnerDeathDestroy(const bool IsShowDissolveFX);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTransform GetCostumeActorSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnAttachedToOwner();

	UFUNCTION(BlueprintImplementableEvent)
	void OnChangeCurrentStatus(const FName& InBeforeStatus);


protected:
	UFUNCTION(BlueprintPure, Meta = (AutoCreateRefTerm = "InAbstractSocketName"))
	const FName& QueryConcreteSocketName(const FName& InAbstractSocketName) const;

	UFUNCTION(BlueprintPure, Meta = (AutoCreateRefTerm = "InCheckTag"))
	bool QueryOwnerStateTag(const FGameplayTag& InCheckTag) const;

	UFUNCTION(BlueprintPure)
	USceneComponent* GetOwnerSkeletalMeshComponent() const;

public:
	UFUNCTION(BlueprintPure)
	const FName& GetCostumeName() const { return CostumeName; }

	UFUNCTION(BlueprintPure)
	EAGPCostumeType GetCostumeType() const { return CostumeType; }

	UFUNCTION(BlueprintPure)
	const FGameplayTag& GetSoruceEquipmentSlotTag() const { return SoruceEquipmentSlotTag; }

	UFUNCTION(BlueprintPure)
	const FName& GetCurrentStatus() const { return CurrentStatus; }
};
