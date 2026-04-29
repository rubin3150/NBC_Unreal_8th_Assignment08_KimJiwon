#include "Spike.h"

#include "MyCharacter.h"
#include "SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ASpike::ASpike()
{
	PrimaryActorTick.bCanEverTick = true;
	
	DamageAmount = 10.0f;
	bCanDamage = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
	
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->SetupAttachment(Scene);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Collision);
}

void ASpike::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	if (FoundVolumes.Num() > 0)
		CachedSpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
	
	SetZ(-200.f);
}

void ASpike::MoveToRandomPoint()
{
	FVector NewPos = CachedSpawnVolume ? CachedSpawnVolume->GetRandomPointInVolume() : GetActorLocation();
	SetActorLocation(FVector(NewPos.X, NewPos.Y, -200.f));
}

void ASpike::SetZ(float Z)
{
	FVector Pos = GetActorLocation();
	Pos.Z = Z;
	SetActorLocation(Pos);
}

void ASpike::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bCanDamage)
		TryDamagePlayer();
}

void ASpike::TryDamagePlayer()
{
	TArray<AActor*> OverlappingActors;
	Collision->GetOverlappingActors(OverlappingActors, AMyCharacter::StaticClass());
	
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			UGameplayStatics::ApplyDamage(Actor, DamageAmount, nullptr, this, UDamageType::StaticClass());
			SetZ(-200.f);
			bCanDamage = false;
			break;
		}
	}
}