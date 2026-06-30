// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Collision/AGPCollisionStruct.h"
#include "AGPCollisionHandleComponent.generated.h"

struct FAGPCollisionIdentifierHandle;
class UAGP_AbilitySystemComponent;
class UShapeComponent;

USTRUCT()
struct FAGPCollisionElement
{
	GENERATED_BODY()

public:
	TWeakObjectPtr<UShapeComponent> CachedCollisionComponent;

	/* 이전에 Overlap 이벤트에 감지된 Actor들 */
	TArray<TWeakObjectPtr<AActor>> DetectedOverlapActors;
};

/* CotumeActor들의 Collision들에 대해 등록된 정보를 저장 */
USTRUCT(BlueprintType)
struct FAGPCollisionCostumeDelegateMap
{
	GENERATED_BODY()
	
public:
	/* EAGPCostumeType::FromEquipment인 CostumeActor들 */
	UPROPERTY()
	TMap<FAGPCollisionCostumeEquipmentMapKey, FAGPCollisionElement> FromEquipmentCollisionMap;
};




/* 캐릭터의 여러 Collision들(ex : CostumeActor의 무기 Collision등)에 대한 이벤트 등록 및 송수신 처리 */
UCLASS(BlueprintType)
class ACTIONGAMEPROJECT_API UAGPCollisionHandleComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	TWeakObjectPtr<UAGP_AbilitySystemComponent> CachedOwnerASC;

private:
	/* CostumeActor로부터 추가되는 Collision들 */
	UPROPERTY()
	FAGPCollisionCostumeDelegateMap CollisionCostumeDelegateMap;

	/* 캐릭터 자체에 붙는 Collision들 */
	UPROPERTY()
	TMap<FName, FAGPCollisionElement> CollisionAttachCharacterMap;

public:	
	UAGPCollisionHandleComponent();
		
public:
	void InitializeOnPossedToOwner(UAGP_AbilitySystemComponent* InOwnerASC);
	void OnUnpossedFromOwner();
	void AllCollisionDisable();

public:
	/* Collision Register */
	UFUNCTION(BlueprintCallable)
	void RegisterCollision(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle, UShapeComponent* InCollisionComponent);
	
	/* Collision Unregister */
	UFUNCTION(BlueprintCallable)
	void UnregisterCollision(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle);

	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm = "InHitEventTag", Categories = "Event.CollisionHit"))
	void OnOverlapNotifyHitEvent(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult);

public:
	void CollisionActiveToggle(const FAGPCollisionIdentifierHandle& InCollisionIdentifierHandle, bool IsActive);

private:
	void RegisterCollision_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor, UShapeComponent* InCollisionComponent);
	void RegisterCollision_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter, UShapeComponent* InCollisionComponent);

	void UnregisterCollision_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor);
	void UnregisterCollision_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter);

	void OnOverlapNotifyHitEvent_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult);
	void OnOverlapNotifyHitEvent_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult);
	
	void CollisionActiveToggle_CostumeActor(const FAGPCollisionIdentifierCostumeActor& InCollisionIdentifierCostumeActor, bool IsActive);
	void CollisionActiveToggle_AttachCharacter(const FAGPCollisionIdentifierAttachCharacter& InCollisionIdentifierAttachCharacter, bool IsActive);

private:
	void ToggleCollisionActive(FAGPCollisionElement& InCollisionElement, bool IsActive);
	void NotifyHitEvent(FAGPCollisionElement& InCollisionElement, const FGameplayTag& InHitEventTag, AActor* InInstigatorActor, AActor* InCollisionOtherActor, const FHitResult& SweepResult);
};
