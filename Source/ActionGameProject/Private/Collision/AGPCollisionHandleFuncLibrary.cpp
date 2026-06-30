// KJY All Rights Reserved


#include "Collision/AGPCollisionHandleFuncLibrary.h"

FAGPCollisionIdentifierHandle UAGPCollisionHandleFuncLibrary::CreateCollisionIdentifierCostumeActor(const FName& InCollisionName, const FCostumeInstanceIdentifierHandle& InCostumeIdentifierHandle, const FName& InCostumeActorName)
{
    checkf(InCostumeIdentifierHandle.GetCostumeInstanceIdentifier().IsValid(), TEXT("[%s], Invalid CostumeInstance Identifier"), __FUNCTIONW__);

    return FAGPCollisionIdentifierHandle(MakeShared<FAGPCollisionIdentifierCostumeActor>(InCostumeIdentifierHandle.GetCostumeInstanceIdentifier(), InCollisionName, InCostumeActorName));
}

FAGPCollisionIdentifierHandle UAGPCollisionHandleFuncLibrary::CreateCollisionIdentifierAttachCharacter(const FName& InCollisionName)
{
    return FAGPCollisionIdentifierHandle(MakeShared<FAGPCollisionIdentifierAttachCharacter>(InCollisionName));
}
