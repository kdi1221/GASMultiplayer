// KJY All Rights Reserved


#include "Animation/AnimInstances/AGPCharacterAnimInstance.h"
#include "Characters/AGP_CharacterBase.h"
#include "Characters/Components/AGPCharacterMovementComponent.h"
#include "Animation/AnimInstances/AGPLinkedAnimLayerBase.h"
#include "KismetAnimationLibrary.h"
#include "Log/AGPLogChannels.h"

//#if WITH_EDITOR
//#include "Framework/Notifications/NotificationManager.h"
//#include "Widgets/Notifications/SNotificationList.h"
//#include "Misc/DataValidation.h"
//#endif // WITH_EDITOR

#define LOCTEXT_NAMESPACE "AGPCharacterAnimInstance"


//#if WITH_EDITORONLY_DATA
//TWeakPtr<class SNotificationItem> UAGPCharacterAnimInstance::EditorInvalidOperationError = nullptr;
//#endif // WITH_EDITORONLY_DATA


void FAGPAnimInstanceProxy::Initialize(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::Initialize(InAnimInstance);

	//반드시 게임스레드에서만 실행되어야 함
	verify(IsInGameThread());
	verify(InAnimInstance);

	if (AAGP_CharacterBase* OwnerAGPCharacter = Cast<AAGP_CharacterBase>(InAnimInstance->TryGetPawnOwner()))
	{
		OwningCharacter = OwnerAGPCharacter;
		OwningMovementComponent = OwnerAGPCharacter->GetCharacterMovementComponent();

		UpdateOwnerInfo();

		//AGP_LOG(LogAGPAnimation, Log, TEXT("AnimInstanceProxy Initialize, Only Called GameThread"));
	}

	
}

void FAGPAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);
}

void FAGPAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);

	UpdateOwnerInfo();
}

void FAGPAnimInstanceProxy::Update(float DeltaSeconds)
{
	
}

void FAGPAnimInstanceProxy::UpdateOwnerInfo()
{
	//여기서는 Owner Character와 MovementComponent로부터 값의 복사만 이루어진다.
	//상세 계산은 이후 NativeThreadSafeUpdateAnimation, 애니메이션 스레드에서 진행

	//반드시 게임스레드에서만 실행되어야 함
	verify(IsInGameThread());

	AAGP_CharacterBase* OwnerAGPCharacter = OwningCharacter.Get();
	UAGPCharacterMovementComponent* OwnerMovementComponent = OwningMovementComponent.Get();

	IsValidOwner = OwnerAGPCharacter && OwnerMovementComponent;

	if (IsValidOwner)
	{
		Velocity = OwnerAGPCharacter->GetVelocity();
		Rotation = OwnerAGPCharacter->GetActorRotation();
		Acceleration = OwnerMovementComponent->GetCurrentAcceleration();
		IsTargetLockMove = OwnerAGPCharacter->IsTargetLockMove();
		OwnerDeathStatus = OwnerAGPCharacter->GetDeathState();
		OwnerDeathPoseName = OwnerAGPCharacter->GetDeathPoseName();

	}
	else
	{
		Velocity = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
		Acceleration = FVector::ZeroVector;
		IsTargetLockMove = false;
		OwnerDeathStatus = EAGPCharacterDeath::None;
		OwnerDeathPoseName = NAME_None;
	}
}



void UAGPCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	/* 테스트용 */
	/*if (GetWorld()->WorldType != EWorldType::EditorPreview)
	{
		FAGPTagContainerKey TestAnimLinkKey;

		TestAnimLinkKey.TagContainer.AddTag(AGPGameplayTags::Character_State_WeaponHandling_Holstered);
		TestAnimLinkKey.TagContainer.AddTag(AGPGameplayTags::Character_State_WeaponHandling_Equipped);
		TestAnimLinkKey.TagContainer.AddTag(AGPGameplayTags::Character_State_WeaponHandling_Transitioning);

		if (const TSubclassOf<UAGPAnimInstanceBase>* FindAnimLink = AnimationLinkMap.Find(TestAnimLinkKey))
		{
			AGP_NET_LOG(this, LogAGPAnimation, Log, TEXT("Find Anim Link[%s]"), *FindAnimLink->Get()->GetName());
		}
		else
		{
			AGP_NET_LOG(this, LogAGPAnimation, Warning, TEXT("Not Find.."));
		}
	}*/
}

void UAGPCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	const FAGPAnimInstanceProxy& AGPAnimInstanceProxy = GetProxyOnAnyThread<FAGPAnimInstanceProxy>();

	if (!AGPAnimInstanceProxy.IsValidOwner)
	{
		return;
	}

	GroundSpeed = AGPAnimInstanceProxy.Velocity.Size2D();
	bHasAcceleration = AGPAnimInstanceProxy.Acceleration.SizeSquared2D() > 0.f;
	bIsTargetLockMove = AGPAnimInstanceProxy.IsTargetLockMove;
	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(AGPAnimInstanceProxy.Velocity, AGPAnimInstanceProxy.Rotation);	

	CurrentDeathStatus = AGPAnimInstanceProxy.OwnerDeathStatus;
	CurrentDeathPoseName = AGPAnimInstanceProxy.OwnerDeathPoseName;
}

void UAGPCharacterAnimInstance::SetAnimationLinkLayer(TSubclassOf<UAGPLinkedAnimLayerBase> InNewAnimLinkLayer)
{
	if (!InNewAnimLinkLayer)
	{
		AGP_NET_LOG(this, LogAGPAnimation, Warning, TEXT("Invalid AnimLinkLayer, Owner[%s]"), *GetNameSafe(this));
		return;
	}

	LinkAnimClassLayers(InNewAnimLinkLayer);
}







//#if WITH_EDITOR
//void UAGPCharacterAnimInstance::PreEditChange(FEditPropertyChain& PropertyAboutToChange)
//{
//	Super::PreEditChange(PropertyAboutToChange);
//
//	/* 에디터에서 AnimationLinkMap의 특정 Key값을 편집하는 경우 키값이 중복될수 있기때문에 이후 PostEditChangeChainProperty에서 진행되는 중복검사에서 롤백시키기 위해 별도의 백업변수에 저장해둔다. */
//	if (IsEditingAnimationLinkMap(PropertyAboutToChange))
//	{
//		BackupAnimationLinkMap = AnimationLinkMap;
//	}
//}
//
//void UAGPCharacterAnimInstance::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
//{
//	Super::PostEditChangeChainProperty(PropertyChangedEvent);
//
//	if (IsEditingAnimationLinkMap(PropertyChangedEvent.PropertyChain))
//	{
//		TSet<FAGPTagContainerKey> PostEditAnimationLinkKey;
//
//		/* AnimationLinkMap에서 특정 키 값을 수정했을때, 그 키 값이 중복되는지 확인한다. */
//		bool bHasDuplicate = false;
//		for (const TPair<FAGPTagContainerKey, TSubclassOf<UAGPAnimInstanceBase>>& AnimationLinkTagPair : AnimationLinkMap)
//		{
//			if (PostEditAnimationLinkKey.Contains(AnimationLinkTagPair.Key))
//			{
//				bHasDuplicate = true;
//				break;
//			}
//
//			PostEditAnimationLinkKey.Add(AnimationLinkTagPair.Key);
//		}
//
//		/* 위 검사 로직에서 중복된다고 판단된 경우 이전 값으로 돌린다. */
//		if (bHasDuplicate)
//		{
//			EditorShowInvalidOperationError(LOCTEXT("AnimationLinkMap_Key_Duplicate", "AnimationLinkMap에 동일한 키를 가진 요소가 존재합니다."));
//
//			AnimationLinkMap = BackupAnimationLinkMap;
//			BackupAnimationLinkMap.Empty();
//		}
//	}
//}
//
//bool UAGPCharacterAnimInstance::IsEditingAnimationLinkMap(const FEditPropertyChain& InPropertyChain) const
//{
//	FEditPropertyChain::TDoubleLinkedListNode* ActiveMemberNode = InPropertyChain.GetActiveMemberNode();
//	if (!ActiveMemberNode)
//	{
//		return false;
//	}
//
//	FProperty* ActiveMemberNodeProperty = ActiveMemberNode->GetValue();
//	if (!ActiveMemberNodeProperty)
//	{
//		return false;
//	}
//
//	if (ActiveMemberNodeProperty->GetFName() != GET_MEMBER_NAME_CHECKED(UAGPCharacterAnimInstance, AnimationLinkMap))
//	{
//		return false;
//	}
//
//	FEditPropertyChain::TDoubleLinkedListNode* ActivePropertyNode = InPropertyChain.GetActiveNode();
//	if (!ActivePropertyNode)
//	{
//		return false;
//	}
//
//	FProperty* ActiveProperty = ActivePropertyNode->GetValue();
//	if (!ActiveProperty)
//	{
//		return false;
//	}
//
//	return ActiveProperty->GetFName() == GET_MEMBER_NAME_CHECKED(FAGPTagContainerKey, TagContainer);
//}
//
//void UAGPCharacterAnimInstance::EditorShowInvalidOperationError(const FText& ErrorText) const
//{
//	if (!EditorInvalidOperationError.IsValid())
//	{
//		FNotificationInfo InvalidOperation(ErrorText);
//		InvalidOperation.ExpireDuration = 3.0f;
//		EditorInvalidOperationError = FSlateNotificationManager::Get().AddNotification(InvalidOperation);
//	}
//}
//
//EDataValidationResult UAGPCharacterAnimInstance::IsDataValid(FDataValidationContext& Context) const
//{
//	EDataValidationResult ValidationResult = Super::IsDataValid(Context);
//
//	if (ValidationResult != EDataValidationResult::Invalid)
//	{
//		/* AnimationLinkMap에 동일한 키값을 가진 요소들이 존재하는지 확인해서 블루프린트 컴파일 실패 처리 */
//		TSet<FAGPTagContainerKey> PostEditAnimationLinkKey;
//
//		/* AnimationLinkMap에서 키 값이 중복되는지 확인한다. */
//		bool bHasDuplicate = false;
//		for (const TPair<FAGPTagContainerKey, TSubclassOf<UAGPAnimInstanceBase>>& AnimationLinkTagPair : AnimationLinkMap)
//		{
//			if (PostEditAnimationLinkKey.Contains(AnimationLinkTagPair.Key))
//			{
//				bHasDuplicate = true;
//				break;
//			}
//
//			PostEditAnimationLinkKey.Add(AnimationLinkTagPair.Key);
//		}
//
//		/* 위 검사 로직에서 중복된다고 판단된 경우 오류로 판단. */
//		if (bHasDuplicate)
//		{
//			Context.AddError(LOCTEXT("AnimationLinkMap_Key_Duplicate", "AnimationLinkMap에 동일한 키를 가진 요소가 존재합니다."));
//			ValidationResult = EDataValidationResult::Invalid;
//		}
//	}
//
//	return ValidationResult;
//}
//
//#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE