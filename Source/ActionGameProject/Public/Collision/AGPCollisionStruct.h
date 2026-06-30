// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AGPCollisionEnum.h"
#include "Costume/AGPCostumeStruct.h"
#include "AGPCollisionStruct.generated.h"

/* Collision 식별자 Base */
struct FAGPCollisionIdentifierBase
{
private:
	EAGPCollisionType CollisionType = EAGPCollisionType::None;
	FName CollisionName;

public:
	FAGPCollisionIdentifierBase(const EAGPCollisionType InCollisionType, const FName& InCollisionName)
		:CollisionType(InCollisionType)
		,CollisionName(InCollisionName)
	{

	}

public:
	inline EAGPCollisionType GetCollisionType() const { return CollisionType; }
	inline const FName& GetCollisionName() const { return CollisionName; }
};

/* CostumeActor에 대한 CollisionIdentifier */
struct FAGPCollisionIdentifierCostumeActor : public FAGPCollisionIdentifierBase
{
private:
	TSharedPtr<FCostumeInstanceIdentifierBase> CostumeInstanceIdentifier = nullptr;
	FName CostumeActorName;

public:
	FAGPCollisionIdentifierCostumeActor(const TSharedPtr<FCostumeInstanceIdentifierBase>& InCostumeIdentifier, const FName& InCollisionName, const FName& InCostumeActorName)
		:FAGPCollisionIdentifierBase(EAGPCollisionType::CostumeActor, InCollisionName)
		, CostumeInstanceIdentifier(InCostumeIdentifier)
		, CostumeActorName(InCostumeActorName)
	{

	}

public:
	inline TSharedPtr<FCostumeInstanceIdentifierBase> GetCostumeInstanceIdentifier() const { return CostumeInstanceIdentifier; }
	inline const FName& GetCostumeActorName() const { return CostumeActorName; }
};

/* 캐릭터에 붙은 Collision 식별자 */
struct FAGPCollisionIdentifierAttachCharacter : public FAGPCollisionIdentifierBase
{
public:
	FAGPCollisionIdentifierAttachCharacter(const FName& InCollisionName)
		:FAGPCollisionIdentifierBase(EAGPCollisionType::AttachCharacter, InCollisionName)
	{

	}
};


/* 블루프린트에서 FAGPCollisionIdentifierBase의 상속 구조체들을 지정할 수 있는 Handle 구조체 */
USTRUCT(BlueprintType)
struct FAGPCollisionIdentifierHandle
{
	GENERATED_BODY()
	
private:
	TSharedPtr<FAGPCollisionIdentifierBase> CollisionIdentifier = nullptr;
	
public:
	FAGPCollisionIdentifierHandle()
	{

	}

	explicit FAGPCollisionIdentifierHandle(const TSharedPtr<FAGPCollisionIdentifierBase>& InCollisionIdentifier)
		:CollisionIdentifier(InCollisionIdentifier)
	{

	}

public:
	inline TSharedPtr<FAGPCollisionIdentifierBase> GetCollisionIdentifier() const { return CollisionIdentifier; }
};


USTRUCT()
struct FAGPCollisionCostumeEquipmentMapKey
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FAGPCostumeEquipmentMapKey CostumeEquipmentKey;

	UPROPERTY()
	FName CollisionName;

public:
	FAGPCollisionCostumeEquipmentMapKey() = default;
	FAGPCollisionCostumeEquipmentMapKey(const FGameplayTag& InTagEquipmentSlot, const FName& InCostumeActorName, const FName& InCollisionName)
		:CostumeEquipmentKey(InTagEquipmentSlot, InCostumeActorName)
		,CollisionName(InCollisionName)
	{

	}

public:
	FORCEINLINE bool operator==(const FAGPCollisionCostumeEquipmentMapKey& Other) const
	{
		return CostumeEquipmentKey == Other.CostumeEquipmentKey &&
			CollisionName == Other.CollisionName;
	}
};

FORCEINLINE uint32 GetTypeHash(const FAGPCollisionCostumeEquipmentMapKey& InLinkKey)
{
	uint32 Hash = GetTypeHash(InLinkKey.CostumeEquipmentKey);
	Hash = HashCombine(Hash, GetTypeHash(InLinkKey.CollisionName));

	return Hash;
}

