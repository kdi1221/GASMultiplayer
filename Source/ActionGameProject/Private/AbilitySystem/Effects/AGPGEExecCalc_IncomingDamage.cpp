// KJY All Rights Reserved


#include "AbilitySystem/Effects/AGPGEExecCalc_IncomingDamage.h"
#include "AbilitySystem/Attribute/AGPAttributeSetCommon.h"
#include "AGPGameplayTags.h"

struct FAGPDamageStatics
{
	//매크로 내부적으로 FGameplayEffectAttributeCaptureDefinition 변수 생성
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageValue)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageApplyRate)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefenseRate)
	DECLARE_ATTRIBUTE_CAPTUREDEF(IncomingDamage)

	FAGPDamageStatics()
	{
		/* 데미지를 주는 Source의 DamageValue 수치 */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAGPAttributeSetCommon, DamageValue, Source, false)

		/* 데미지를 주는 Source의 DamageApplyRate 수치 */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAGPAttributeSetCommon, DamageApplyRate, Source, false)
		
		/* 데미지를 받는 Target의 DefenseRate 수치 */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAGPAttributeSetCommon, DefenseRate, Target, false)

		/* 데미지를 받는 Target의 IncomingDamage 수치 */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAGPAttributeSetCommon, IncomingDamage, Target, false)
	}
};

static const FAGPDamageStatics& GetDamageStatics()
{
	static FAGPDamageStatics DamageStatics;

	return DamageStatics;
}

UAGPGEExecCalc_IncomingDamage::UAGPGEExecCalc_IncomingDamage()
{
	//캡처 정의 등록
	const FAGPDamageStatics& DamageStatics = GetDamageStatics();
	RelevantAttributesToCapture.Add(DamageStatics.DamageValueDef);
	RelevantAttributesToCapture.Add(DamageStatics.DamageApplyRateDef);
	RelevantAttributesToCapture.Add(DamageStatics.DefenseRateDef);
	RelevantAttributesToCapture.Add(DamageStatics.IncomingDamageDef);
}

void UAGPGEExecCalc_IncomingDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FAGPDamageStatics& DamageStatics = GetDamageStatics();

	const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();

	//해당 게임플레이 Effect와 연관된 EffectContext에 접근
	FGameplayEffectContextHandle EffectContextHandle = EffectSpec.GetContext();

	const FGameplayTagContainer* SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	//Source의 DamageValue 
	float SourceDamageValue = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics.DamageValueDef, EvaluationParameters, SourceDamageValue);

	//Source의 DamageApplyRate 
	float SourceDamageApplyRate = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics.DamageApplyRateDef, EvaluationParameters, SourceDamageApplyRate);

	//Target의 DefenseRate
	float TargetDefenseRate = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics.DefenseRateDef, EvaluationParameters, TargetDefenseRate);

	/* SetByCaller로 지정한 ComboAttackCount, ComboAttackRate 참조 */
	float BaseDamageValue = SourceDamageValue;
	const float* FindComboAttackCountValue = EffectSpec.SetByCallerTagMagnitudes.Find(AGPGameplayTags::GameplayEffect_SetByCaller_ComboAttackCount);
	const float* FindComboAttackRateValue = EffectSpec.SetByCallerTagMagnitudes.Find(AGPGameplayTags::GameplayEffect_SetByCaller_ComboAttackRate);
	if (FindComboAttackCountValue && FindComboAttackRateValue)
	{
		int32 ComboAttackCount = static_cast<int32>(*FindComboAttackCountValue);
		float ComboAttackRate = *FindComboAttackRateValue;

		/* ComboCount에 따른 증가된 Damage Rate 계산 = 1 + (ComboCount * ComboAttackRate) */
		const float DamageIncreaseRateByCombo = FMath::Max(ComboAttackCount, 0)	* FMath::Max(ComboAttackRate, 0.f) + 1.f;
		BaseDamageValue *= DamageIncreaseRateByCombo;
	}

	/* 최종 데미지 계산 : ComboCount에 따라 증가된 BaseDamage * Source의 DamageApplyRate / Target의 DefenseRate */
	const float ApplyFinalDamageValue = BaseDamageValue * SourceDamageApplyRate / TargetDefenseRate;

	/* 최종 데미지 적용 : IncomingDamage에 Override하여 이후 AttributeSet내에서 Health감소 처리 */
	if (ApplyFinalDamageValue > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics.IncomingDamageProperty, EGameplayModOp::Override, ApplyFinalDamageValue));
	}
}
