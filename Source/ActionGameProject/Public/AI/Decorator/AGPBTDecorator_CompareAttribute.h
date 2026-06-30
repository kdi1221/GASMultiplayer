// KJY All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "AttributeSet.h"
#include "BehaviorTree\Blackboard\BlackboardKeyEnums.h"
#include "AGPBTDecorator_CompareAttribute.generated.h"

struct FOnAttributeChangeData;

UENUM()
enum class EAGPCheckAttributeValue : uint8
{
	Rate = 0,
	Value
};

/**
 * Owner Pawn의 Attribute값 비교 데코레이터
 */
UCLASS()
class ACTIONGAMEPROJECT_API UAGPBTDecorator_CompareAttribute : public UBTDecorator
{
	GENERATED_BODY()

	
private:
	/* 부모의 BehaviorTreeComponent, 이벤트 콜백함수에서 사용하기 위해 캐싱 */
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> OwnerBehaviorTreeComponent;

private:
	/* Rate / Value 구분 */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	EAGPCheckAttributeValue CheckOperation;

	/* AttributeSet들 중 확인할 Attribute(Current) */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	FGameplayAttribute CurrentAttribute;

	/* AttributeSet들 중 확인할 Attribute(Max) */
	UPROPERTY(EditAnywhere, Category = "Attribute", meta = (EditCondition = "CheckOperation == EAGPCheckAttributeValue::Rate", EditConditionHides))
	FGameplayAttribute MaxAttribute;

	/* 비교 동작 */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	TEnumAsByte<EArithmeticKeyOperation::Type> CompareOperation;

	/* 비교값 */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	float ValueThreshold = 0.f;

public:
	UAGPBTDecorator_CompareAttribute();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	virtual FString GetStaticDescription() const override;

private:
	float GetAttributeValue(UBehaviorTreeComponent* OwnerComp, const FGameplayAttribute& FindAttribute, bool& bIsFoundValue) const;
	bool CompareValue(const float InCheckValue) const;

private:
	/* Current Attribute 변경될 때 호출 */
	void OnChangeCurrentAttribute(const FOnAttributeChangeData& Data);

	/* Max Attribute 변경될 때 호출(CheckOperation가 Rate일때) */
	void OnChangeMaxAttribute(const FOnAttributeChangeData& Data);

	/* Current, Max Attribute가 변경될때, 조건을 확인해서 실행 또는 중지 */
	void OnChangeAttributeEvaluateAbort(const float CheckValue);
	
};
