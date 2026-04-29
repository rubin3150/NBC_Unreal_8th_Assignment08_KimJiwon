#include "SpawnVolume.h"
#include "Components/BoxComponent.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
	
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);
	
	ItemDataTable = nullptr;
}

AActor* ASpawnVolume::SpawnRandomItem()
{
	if (FItemSpawnRow* SelectedRow = GetRandomItem())
		if (UClass* ActualClass = SelectedRow->ItemClass.Get())
			return SpawnItem(ActualClass);
	
	return nullptr;
}

FItemSpawnRow* ASpawnVolume::GetRandomItem() const
{
	if (!ItemDataTable) return nullptr;
	
	TArray<FItemSpawnRow*> AllRows;
	static const FString ContextString(TEXT("ItemSpawnContext"));
	ItemDataTable->GetAllRows(ContextString, AllRows);
	
	if (AllRows.IsEmpty()) return nullptr;
	
	float TotalChance = 0.0f;
	for (FItemSpawnRow* Row : AllRows)
	{
		if (Row)
			TotalChance += Row->SpawnChance;
	}
	
	const float RandValue = FMath::RandRange(0.0f, TotalChance);
	float AccmulateChance = 0.0f;
	
	for (FItemSpawnRow* Row : AllRows)
	{
		AccmulateChance += Row->SpawnChance;
		if (RandValue <= AccmulateChance)
			return Row;
	}
	
	return nullptr;
}

FVector ASpawnVolume::GetRandomPointInVolume() const
{
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	FVector BoxOrigin = SpawningBox->GetComponentLocation();
	
	return BoxOrigin + FVector(
		FMath::RandRange(-BoxExtent.X, BoxExtent.X),
		FMath::RandRange(-BoxExtent.Y, BoxExtent.Y),
		FMath::RandRange(-BoxExtent.Z, BoxExtent.Z)
		);
}

AActor* ASpawnVolume::SpawnItem(TSubclassOf<AActor> ItemClass)
{
	if (!ItemClass) return nullptr;
	
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ItemClass, GetRandomPointInVolume(), FRotator::ZeroRotator);
	return SpawnedActor;
}

AActor* ASpawnVolume::SpawnAtRandomPoint(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass) return nullptr;
	return GetWorld()->SpawnActor<AActor>(ActorClass, GetRandomPointInVolume(), FRotator::ZeroRotator);
}
