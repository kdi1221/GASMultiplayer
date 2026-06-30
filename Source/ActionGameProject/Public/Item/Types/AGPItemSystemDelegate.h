// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class UAGPItemSlot;
class UAGPItemInstance;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemSlotDelegate, const UAGPItemSlot* /*InItemSlot*/, UAGPItemInstance* /*InItemInstanceInSlot*/);