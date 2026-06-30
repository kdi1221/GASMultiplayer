// KJY All Rights Reserved


#include "Costume/AttachCostumeBase.h"
#include "Net/UnrealNetwork.h"
#include "Costume/AGPCostumeActorBase.h"
#include "Log/AGPLogChannels.h"

UAttachCostumeBase::UAttachCostumeBase()
{

}

bool UAttachCostumeBase::IsSupportedForNetworking() const
{
	return true;
}

void UAttachCostumeBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UAttachCostumeBase::BeginDestroy()
{
	Super::BeginDestroy();

	int32 a = 10;
	a = a;
}

void UAttachCostumeBase::FinishDestroy()
{
	Super::FinishDestroy();

	int32 a = 10;
	a = a;
}

void UAttachCostumeBase::SetCostumeType(const EAGPCostumeType InCostumeType)
{
	CostumeType = InCostumeType;
}

void UAttachCostumeBase::SetCostumeEquipmentSlot(const FGameplayTag& InCostumeEquipmentSlotTag)
{
	SoruceEquipmentSlotTag = InCostumeEquipmentSlotTag;
}

void UAttachCostumeBase::SpawnCostumeActors(AActor* InAttachTargetActor, APawn* InCostumeActorInstigator, const FCostumeActorSpawnDelegate& InOnSpawnDelegate)
{
	verifyf(InAttachTargetActor, TEXT("[%s] - Invalid AttachTargetActor"), __FUNCTIONW__);

	UWorld* WorldContext = GetWorld();
	verifyf(WorldContext, TEXT("[%s] WorldContext Invalid.."), *GetNameSafe(this));

	const FTransform InitSpawnTransform(InAttachTargetActor->GetActorLocation());

	for (const TPair<FName, FAGPCostumeActorToSpawn>& ActorToSpawn : SpawnComstumeActorClassesMap)
	{
		const FName& InCostumeActorName = ActorToSpawn.Key;
		const FAGPCostumeActorToSpawn& InCostumeActorInfo = ActorToSpawn.Value;

		verifyf(InCostumeActorInfo.AttachedActorClass, TEXT("AttachedActorClass Invalid, CostumeName[%s]"), *InCostumeActorName.ToString());

		AAGPCostumeActorBase* SpawnedCostumeActor = WorldContext->SpawnActorDeferred<AAGPCostumeActorBase>(
			InCostumeActorInfo.AttachedActorClass,
			FTransform::Identity,
			InAttachTargetActor,
			InCostumeActorInstigator,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		verifyf(SpawnedCostumeActor, TEXT("SpawnedCostumeActor Invalid, CostumeName[%s]"), *InCostumeActorName.ToString());

		/* CostumeActor 공용 속성들 지정 */
		SpawnedCostumeActor->SetCostumeType(GetCostumeType());
		SpawnedCostumeActor->SetCostumeActorName(InCostumeActorName);

		/* CostumeType별 속성들 지정 */
		switch (GetCostumeType())
		{
		case EAGPCostumeType::FromEquipment:
			{
				SpawnedCostumeActor->SetCostumeEquipmentSlot(GetSoruceEquipmentSlotTag());
			}
			break;
		}

		SpawnedCostumeActor->FinishSpawning(InitSpawnTransform, true);

		//Actor가 Spawn되었음을 델리게이트로 알림(스폰된 이후 후 처리)
		InOnSpawnDelegate.ExecuteIfBound(SpawnedCostumeActor);
	}
}