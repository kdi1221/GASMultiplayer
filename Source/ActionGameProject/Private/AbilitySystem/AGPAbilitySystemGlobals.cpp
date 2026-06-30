// KJY All Rights Reserved


#include "AbilitySystem/AGPAbilitySystemGlobals.h"
#include "AbilitySystem/AGPAbilityTypes.h"

FGameplayEffectContext* UAGPAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FAGPGameplayEffectContext();
}
