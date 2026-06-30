// KJY All Rights Reserved


#include "Animation/AnimInstances/AGPAnimInstanceBase.h"
#include "GameplayTagContainer.h"
#include "Characters/AGP_CharacterBase.h"
#include "Characters/Components/AGPCharPresentationComponent.h"

void UAGPAnimInstanceBase::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if (AAGP_CharacterBase* OwnerCharacter = Cast<AAGP_CharacterBase>(TryGetPawnOwner()))
    {
        OwnerCharacterPresentationComponent = OwnerCharacter->GetCharacterPresentationComponent();
    }
}

bool UAGPAnimInstanceBase::DoesOwnerHaveTag(const FGameplayTag& InTagToCheck) const
{
    if (UAGPCharPresentationComponent* CharacterPresentationComponent = OwnerCharacterPresentationComponent.Get())
    {
        return CharacterPresentationComponent->HasOwnedGameplayTag(InTagToCheck);
    }

    return false;
}
