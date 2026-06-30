// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPCostumeEnum.h"
#include "GameplayTagContainer.h"
#include "AGPCostumeStruct.generated.h"


/* 장비로부터 추가되는 CostumeActor에 대한 TMap의 Key */
USTRUCT()
struct FAGPCostumeEquipmentMapKey
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag TagEquipmentSlot;

	UPROPERTY()
	FName CostumeActorName;

public:
	FAGPCostumeEquipmentMapKey() = default;
	FAGPCostumeEquipmentMapKey(const FGameplayTag& InTagEquipmentSlot, const FName& InCostumeActorName)
		:TagEquipmentSlot(InTagEquipmentSlot)
		, CostumeActorName(InCostumeActorName)
	{

	}

public:
	FORCEINLINE bool operator==(const FAGPCostumeEquipmentMapKey& Other) const
	{
		return TagEquipmentSlot == Other.TagEquipmentSlot &&
			CostumeActorName == Other.CostumeActorName;
	}
};

FORCEINLINE uint32 GetTypeHash(const FAGPCostumeEquipmentMapKey& InLinkKey)
{
	uint32 Hash = GetTypeHash(InLinkKey.TagEquipmentSlot);
	Hash = HashCombine(Hash, GetTypeHash(InLinkKey.CostumeActorName));

	return Hash;
}



/* Costume Instance 식별자 Base */
struct FCostumeInstanceIdentifierBase
{
private:
	EAGPCostumeType CostumeType = EAGPCostumeType::None;

public:
	FCostumeInstanceIdentifierBase(const EAGPCostumeType InCostumeType)
		:CostumeType(InCostumeType)
	{

	}

public:
	inline EAGPCostumeType GetCostumeIdentifierType() const { return CostumeType; }
};

/* 장비아이템으로부터 추가되는 Costume */
struct FCostumeInstanceEquipmentIdentifier : public FCostumeInstanceIdentifierBase
{
private:
	FGameplayTag TagEquipmentSlot;

public:
	FCostumeInstanceEquipmentIdentifier(const FGameplayTag& InTagEquipmentSlot)
		:FCostumeInstanceIdentifierBase(EAGPCostumeType::FromEquipment)
		,TagEquipmentSlot(InTagEquipmentSlot)
	{

	}

public:
	inline const FGameplayTag& GetTagEquipmentSlot() const { return TagEquipmentSlot; }
};


/* 블루프린트에서 FCostumeInstanceIdentifierBase의 상속 구조체들을 지정할 수 있는 Handle 구조체 */
USTRUCT(BlueprintType)
struct FCostumeInstanceIdentifierHandle
{
	GENERATED_BODY()
	
private:
	TSharedPtr<FCostumeInstanceIdentifierBase> CostumeInstanceIdentifier = nullptr;

public:
	FCostumeInstanceIdentifierHandle()
	{

	}	

	explicit FCostumeInstanceIdentifierHandle(const TSharedPtr<FCostumeInstanceIdentifierBase>& InCostumeIdentifier)
		:CostumeInstanceIdentifier(InCostumeIdentifier)
	{

	}

public:
	void SetCostumeInstanceIdentifier(TSharedPtr<FCostumeInstanceIdentifierBase> InNewCostumeInstanceIdentifier)
	{
		CostumeInstanceIdentifier = InNewCostumeInstanceIdentifier;
	}

public:
	inline TSharedPtr<FCostumeInstanceIdentifierBase> GetCostumeInstanceIdentifier() const { return CostumeInstanceIdentifier; }
	inline bool IsValid() const { return CostumeInstanceIdentifier.IsValid(); }
};