#include "Spike.h"

#include "MyCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ASpike::ASpike()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	DamageAmount = 10.0f;
	RiseDuration = 5.0f;
	ElapsedTime = 0.f;
	bIsRising = false;
	bCanDamage = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
	
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->SetupAttachment(Scene);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Collision);
}

void ASpike::RiseUp(const FVector& InTargetPosition)
{
	SetActorLocation(FVector(InTargetPosition.X, InTargetPosition.Y, -200.f));

	ElapsedTime = 0.f;
	bIsRising = true;
	bCanDamage = true;
	SetActorTickEnabled(true);
}

void ASpike::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRising)
	{
		ElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(ElapsedTime / RiseDuration, 0.f, 1.f);

		FVector Pos = GetActorLocation();
		Pos.Z = FMath::Lerp(-200.f, 0.f, Alpha);
		SetActorLocation(Pos);

		if (Alpha >= 1.f)
			bIsRising = false;
	}

	if (bCanDamage)
	{
		TArray<AActor*> OverlappingActors;
		Collision->GetOverlappingActors(OverlappingActors, AMyCharacter::StaticClass());

		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && Actor->ActorHasTag("Player"))
			{
				UGameplayStatics::ApplyDamage(Actor, DamageAmount, nullptr, this, UDamageType::StaticClass());
				ReturnToGround(); // 데미지 주자마자 내려감
				break;
			}
		}
	}
}
void ASpike::ReturnToGround()
{
	bIsRising = false;
	bCanDamage = false;
	SetActorTickEnabled(false);

	FVector Pos = GetActorLocation();
	Pos.Z = -200.f;
	SetActorLocation(Pos);
}