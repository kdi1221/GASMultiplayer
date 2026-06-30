// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AGPWidgetController.generated.h"


/* 위젯컨트롤러들에서 사용하는 공통적인 델리게이트 타입들 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedSignature, float, CurrentHealth, float, MaxHealth);

/**
 * 위젯에 설정되는 컨트롤러 Base Class
 * 디자인되는 위젯들이 바인딩하는 델리게이트나 기타 데이터들에 접근
 */

UCLASS()
class ACTIONGAMEPROJECT_API UAGPWidgetController : public UObject
{
	GENERATED_BODY()
};
