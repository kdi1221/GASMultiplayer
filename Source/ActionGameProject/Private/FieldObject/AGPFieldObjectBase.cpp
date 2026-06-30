// KJY All Rights Reserved


#include "FieldObject/AGPFieldObjectBase.h"

AAGPFieldObjectBase::AAGPFieldObjectBase()
{
	bReplicates = true;

	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

