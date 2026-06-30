// KJY All Rights Reserved


#include "Animation/AnimInstances/AGPLinkedAnimLayerBase.h"
#include "Animation/AnimInstances/AGPCharacterAnimInstance.h"

UAGPCharacterAnimInstance* UAGPLinkedAnimLayerBase::GetOwnerCharacterAnimInstance() const
{
	return Cast<UAGPCharacterAnimInstance>(GetOwningComponent()->GetAnimInstance());
}
