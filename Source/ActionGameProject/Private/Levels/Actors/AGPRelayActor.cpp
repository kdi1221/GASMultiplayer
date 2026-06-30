// KJY All Rights Reserved


#include "Levels/Actors/AGPRelayActor.h"
#include "AbilitySystem/AGPAbilitySystemFunctionLibrary.h"
#include "Common/AGPCommonFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Log/AGPLogChannels.h"

AAGPRelayActor::AAGPRelayActor()
{
	/* 빈 루트 컴포넌트 */
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	bNetLoadOnClient = false;
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetActorHiddenInGame(true);

#if WITH_EDITORONLY_DATA
	RangeSphereForEditor = CreateDefaultSubobject<USphereComponent>(TEXT("RangeSphereForEditor"));
	RangeSphereForEditor->SetupAttachment(GetRootComponent());
	RangeSphereForEditor->SetSphereRadius(NotifyRange);
	RangeSphereForEditor->SetHiddenInGame(true);
	RangeSphereForEditor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RangeSphereForEditor->bIsEditorOnly = true;
	RangeSphereForEditor->ShapeColor = FColor::Red;
#endif
}

FGenericTeamId AAGPRelayActor::GetGenericTeamId() const
{
	return FGenericTeamId(static_cast<uint8>(NotifyTeamGroup));
}

void AAGPRelayActor::NotifyPlayerExistenceToNPC(APawn* InPlayerPawn)
{
	if (!InPlayerPawn || UAGPAbilitySystemFunctionLibrary::IsTargetDeath(InPlayerPawn))
	{
		return;
	}

	const FVector LastKnownLocaiton = InPlayerPawn->GetActorLocation();
	UAGPCommonFunctionLibrary::ReportAITeamStimulusEvent(this, InPlayerPawn, LastKnownLocaiton, NotifyRange, InfoAge, Strength);
}

#if WITH_EDITOR
void AAGPRelayActor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AAGPRelayActor, NotifyRange))
	{
		if (RangeSphereForEditor)
		{
			RangeSphereForEditor->SetSphereRadius(NotifyRange);
		}
	}
}
#endif // WITH_EDITOR