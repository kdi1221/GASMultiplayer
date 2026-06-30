// KJY All Rights Reserved

#pragma once

#include "Abilities\GameplayAbilityTargetTypes.h"
#include "GameplayTagContainer.h"
#include "Common/AGPCommonEnums.h"
#include "AGPAbilityTypes.generated.h"


namespace AGPBaseFeatureResult
{
	/* Common */
	namespace Common
	{
		static const uint8 Unknonw_Fail = 0;
		static const uint8 Success = 1;
		static const uint8 RejectAbilityActivating = 2;
		static const uint8 RPCWaitTimeout = 3;

		static const uint8 CommonResultMax = 50;
	}
}

class UAGPGameplayAbility;

USTRUCT(BlueprintType)
struct FAGPGrantAbilitySet
{
	GENERATED_BODY()

	public:
	/* Ability가 부여될때 그 Ability에 추가할 DynamicTag */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer GrantAbilityToDynamicTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAGPGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 GrantAbilityLevel = 1;
};






USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_ReuqestInventoryItemEquip : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	/**  */
	UPROPERTY()
	int32 InventorySlotIndex = -1;
	
	UPROPERTY()
	FGameplayTag EquipSlotTag;

	FGameplayAbilityTargetData_ReuqestInventoryItemEquip()
	{ }

	FGameplayAbilityTargetData_ReuqestInventoryItemEquip(const int32 InInventorySlotIndex, const FGameplayTag& InEquipSlotTag)
		: InventorySlotIndex(InInventorySlotIndex)
		, EquipSlotTag(InEquipSlotTag)
	{ }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_ReuqestInventoryItemEquip::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_ReuqestInventoryItemEquip> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_ReuqestInventoryItemEquip>
{
	enum
	{
		WithNetSerializer = true	
	};
};

namespace AGPBaseFeatureResult
{
	namespace ReuqestInventoryItemEquip
	{
		static const uint8 InvalidSlotIndex = Common::CommonResultMax + 1;
		static const uint8 InvalidSlotInItemInstance;
		static const uint8 InvalidEquipmentSlot;
	}
}



USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_DisrobeEquipmentToInventory : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	/**  */
	UPROPERTY()
	FGameplayTag EquipSlotTag;

	UPROPERTY()
	int32 InventorySlotIndex = -1;

	FGameplayAbilityTargetData_DisrobeEquipmentToInventory()
	{ }

	FGameplayAbilityTargetData_DisrobeEquipmentToInventory(const FGameplayTag& InTagEquipSlot, const int32 InInventorySlotIndex)
		: EquipSlotTag(InTagEquipSlot)
		, InventorySlotIndex(InInventorySlotIndex)
	{ }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_DisrobeEquipmentToInventory::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_DisrobeEquipmentToInventory> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_DisrobeEquipmentToInventory>
{
	enum
	{
		WithNetSerializer = true
	};
};

namespace AGPBaseFeatureResult
{
	namespace DisrobeEquipmentToInventory
	{
		static const uint8 InvalidEquipmentSlot = Common::CommonResultMax + 1;
		static const uint8 InvalidSlotInItemInstance;
		static const uint8 InvalidInventorySlot;
	}
}




UENUM(BlueprintType)
enum class EAGPHitReactPlayMontageType : uint8
{
	NotPlayMontage		UMETA(DisplayName = "Not Play Montage"),
	RandomMontage		UMETA(DisplayName = "Random Montage Play"),
	DirectionalMontage	UMETA(DisplayName = "Directional Montage Play"),
};

UENUM(BlueprintType, meta = (BitFlags))
enum class EAGPHitReactFlags : uint8
{
	None				= 0			UMETA(Hidden),
	RotateToSource		= 1 << 0	UMETA(DisplayName = "Rotate To Source"),
};
ENUM_CLASS_FLAGS(EAGPHitReactFlags);

UENUM(BlueprintType, meta = (BitFlags))
enum class EAGPDeathFlags : uint8
{
	None = 0					UMETA(Hidden),
	RotateToSource = 1 << 0		UMETA(DisplayName = "Rotate To Source"),
};
ENUM_CLASS_FLAGS(EAGPDeathFlags);


/* HitReact(Direction) 활성화 */
USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_HitReactDirection : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	/* Hit 방향(자신 기준 HitReact를 발동하게한 대상의 위치) - DirectionalMontage */
	UPROPERTY()
	EAGPDirection HitDirection = EAGPDirection::Front;

public:
	FGameplayAbilityTargetData_HitReactDirection() = default;

	FGameplayAbilityTargetData_HitReactDirection(const EAGPDirection InHitDirection)
		:HitDirection(InHitDirection)
	{

	}

public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_HitReactDirection::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_HitReactDirection> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_HitReactDirection>
{
	enum
	{
		WithNetSerializer = true
	};
};

/* HitReact(Random) 활성화 */
USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_HitReactRandom : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	/* 플레이할 몽타주 Key 이름 - RandomMontage */
	UPROPERTY()
	FName RandomHitReactMontageKeyName = NAME_None;

public:
	FGameplayAbilityTargetData_HitReactRandom() = default;

	FGameplayAbilityTargetData_HitReactRandom(const FName& InMontageKeyName)
		:RandomHitReactMontageKeyName(InMontageKeyName)
	{


	}

public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_HitReactRandom::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_HitReactRandom> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_HitReactRandom>
{
	enum
	{
		WithNetSerializer = true
	};
};

/* Death Ability 활성화 시 필요한 정보 */
USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_DeathAbility : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	FName DeathPoseName;

public:
	FGameplayAbilityTargetData_DeathAbility() = default;

	FGameplayAbilityTargetData_DeathAbility(const FName& InDeathPoseName)
		:DeathPoseName(InDeathPoseName)
	{

	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_DeathAbility::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_DeathAbility> : public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_DeathAbility>
{
	enum
	{
		WithNetSerializer = true
	};
};




/* Custom Gameplay Effect Context */
USTRUCT(BlueprintType)
struct FAGPGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

protected:
	/* HitReact과정에서 사용할 AttackType(EX : Axe, Sword, Explosion...) */
	TSharedPtr<FGameplayTag> GC_AttackTypeTag;

public:
	/* 리플렉션 시스템을 위해 해당 구조체의 정적 구조체 정보를 반환하기 위한 함수 */
	/* serialization을 위해 이 함수를 필수적으로 오버라이드 해줘야 함 */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FAGPGameplayEffectContext* Duplicate() const override
	{
		FAGPGameplayEffectContext* NewContext = new FAGPGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	//NetSerialize를 위해 이를 재정의 해줘야 함
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

public:
	void SetGCAttackType(const FGameplayTag& InAttackType);

public:
	const FGameplayTag& GetGCAttackType() const;
};

template<>
struct TStructOpsTypeTraits<FAGPGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAGPGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};