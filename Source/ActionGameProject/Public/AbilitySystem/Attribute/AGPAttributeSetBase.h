// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AGPAttributeSetBase.generated.h"

struct FGameplayEffectModCallbackData;
class AAGP_CharacterBase;

//각 속성 값에 접근하기 위한 접근자 선언 매크로
//AttributeSet.h 헤더파일 참고
//그러나 일반적으로 속성값을 변경하기위해 이런 접근자를 사용하지는 않는다.(GameplayEffect를 통해서 변경)
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	 GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	 GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	 GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	 GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/* 아래 PostGameplayEffectExecute에서 사용할 Source, Target 정보들 */
USTRUCT()
struct FAGPGEProperties
{
	GENERATED_BODY()

public:
	/* GameplayEffect가 적용될때의 ASC */
	TWeakObjectPtr<UAbilitySystemComponent> ASC = nullptr;

	/* GameplayEffect가 적용될때의 Controller */
	TWeakObjectPtr<AController> Controller = nullptr;

	/* GameplayEffect가 적용될때의 OwnerActor */
	TWeakObjectPtr<AActor> OwnerActor = nullptr;

	/* GameplayEffect가 적용될때의 Avatar */
	TWeakObjectPtr<AActor> AvaterActor = nullptr;

	/* GameplayEffect가 적용될때의 Pawn(일반적으로 AvatarActor와 동일, Cache의 목적 ) */
	TWeakObjectPtr<APawn> CachedPawn = nullptr;

	/* GameplayEffect가 적용될때의 Character(일반적으로 AvatarActor와 동일, Cache의 목적 ) */
	TWeakObjectPtr<AAGP_CharacterBase> CachedCharacter = nullptr;
};

/* PostGameplayEffectExecute에서 전달되는 FGameplayEffectModCallbackData를 통해 GameplayEffect가 적용될때의 Source, Target 정보 추출 */
USTRUCT()
struct FAGPGESourceTargetProperties
{
	GENERATED_BODY()

	public:
	/* 대상 GameplayEffect의 Handle */
	UPROPERTY()
	FGameplayEffectContextHandle GEContextHandle;

	/* 대상 GameplayEffect의 Source 정보 */
	UPROPERTY()
	FAGPGEProperties SourceContext;

	/* 대상 GameplayEffect의 Target 정보 */
	UPROPERTY()
	FAGPGEProperties TargetContext;

public:
	FAGPGESourceTargetProperties() = default;
};


/**
 * AttributeSet Base 클래스
 */

UCLASS(Abstract)
class ACTIONGAMEPROJECT_API UAGPAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	//특정 게임플레이 Effect가 실행되어 속성을 변경할 때 호출되는 함수
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
protected:
	void GenerateGEPropertiesFromCallbackData(const FGameplayEffectModCallbackData& InCallbackData, FAGPGESourceTargetProperties& OutGEProperties);

protected:
	virtual void OnHandlePostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data, const FAGPGESourceTargetProperties& InGEProperties) PURE_VIRTUAL(UAGPAttributeSetBase::OnHandlePostGameplayEffectExecute);
};
