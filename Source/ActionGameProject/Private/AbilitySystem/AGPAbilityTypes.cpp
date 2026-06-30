// KJY All Rights Reserved


#include "AbilitySystem/AGPAbilityTypes.h"

bool FGameplayAbilityTargetData_ReuqestInventoryItemEquip::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << InventorySlotIndex;

	EquipSlotTag.NetSerialize(Ar, Map, bOutSuccess);

	bOutSuccess = true;
	return true;
}

bool FGameplayAbilityTargetData_DisrobeEquipmentToInventory::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	EquipSlotTag.NetSerialize(Ar, Map, bOutSuccess);

	Ar << InventorySlotIndex;

	bOutSuccess = true;
	return true;
}

bool FGameplayAbilityTargetData_HitReactDirection::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << HitDirection;

	bOutSuccess = true;
	return true;
}

bool FGameplayAbilityTargetData_HitReactRandom::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << RandomHitReactMontageKeyName;

	bOutSuccess = true;
	return true;
}

bool FGameplayAbilityTargetData_DeathAbility::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << DeathPoseName;

	bOutSuccess = true;
	return true;
}

bool FAGPGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 RepBits = 0;

	/* 쓰기 상태에서 각 멤버변수(부모 포함)들에 대한 리플리케이션 여부확인(Valid 등)후 비트플래그 설정 */
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}

		if (bReplicateEffectCauser && EffectCauser.IsValid())
		{
			RepBits |= 1 << 1;
		}

		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}

		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}

		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}

		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}

		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}

		/* 추가된 변수 : GameplayCue를위한 AttackType Tag */
		if (GC_AttackTypeTag.IsValid())
		{
			RepBits |= 1 << 7;
		}
	}

	/* 쓰기, 읽기 상태에서 설정된 비트플래그 Save / Load(위에서 설정된 비트플래그 횟수만큼 지정해줘야함) */
	Ar.SerializeBits(&RepBits, 8);

	/* 비트플래그에따라 각 멤버변수들에 대한 Save / Load 처리 */
	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}

	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}

	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}

	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}

	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}

	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}

	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}

	/* 추가된 변수 : GameplayCue를위한 AttackType Tag */
	if (RepBits & (1 << 7))
	{
		if (Ar.IsLoading())
		{
			/* 로딩중일때는 기존 AttackType이 유효한지 확인 후 유효하지 않으면 새로운 GameplayTag를 생성 */
			if (!GC_AttackTypeTag.IsValid())
			{
				GC_AttackTypeTag = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}

		GC_AttackTypeTag->NetSerialize(Ar, Map, bOutSuccess);
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}

	bOutSuccess = true;
	return true;
}

void FAGPGameplayEffectContext::SetGCAttackType(const FGameplayTag& InAttackType)
{
	if (!InAttackType.IsValid())
	{
		GC_AttackTypeTag.Reset();
	}
	else
	{
		GC_AttackTypeTag = MakeShared<FGameplayTag>(InAttackType);
	}
}

const FGameplayTag& FAGPGameplayEffectContext::GetGCAttackType() const
{
	if (!GC_AttackTypeTag.IsValid())
	{
		return FGameplayTag::EmptyTag;
	}
	
	return *GC_AttackTypeTag.Get();
}
