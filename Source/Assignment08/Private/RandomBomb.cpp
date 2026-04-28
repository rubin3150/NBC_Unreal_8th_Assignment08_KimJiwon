#include "RandomBomb.h"
#include "MyCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ARandomBomb::ARandomBomb()
{
    PrimaryActorTick.bCanEverTick = true;
    
    DamageAmount = 10.f;
    ExplosionRadius = 300.f;
    MinInterval = 1.f;
    MaxInterval = 5.f;
    KnockbackPower = 1500.f;

    Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
    SetRootComponent(Scene);

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));
    Collision->InitSphereRadius(ExplosionRadius);
    Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    Collision->SetupAttachment(Scene);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(Collision);
}

void ARandomBomb::BeginPlay()
{
    Super::BeginPlay();
    
    DynamicMaterial = StaticMesh->CreateAndSetMaterialInstanceDynamic(0);
    
    ScheduleNextExplosion();
}

void ARandomBomb::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ElapsedTime += DeltaTime;
    float Alpha = FMath::Clamp(ElapsedTime / CurrentInterval, 0.f, 1.f);
    
    // 메쉬 크기 보간
    float NewScale = FMath::Lerp(MinScale, MaxScale, Alpha);
    StaticMesh->SetWorldScale3D(FVector(NewScale));
    
    // 머티리얼 색상 보간
    if (DynamicMaterial)
    {
        FLinearColor CurrentColor(Alpha, 0.216f, 0.41f, 1.f);
        DynamicMaterial->SetVectorParameterValue(TEXT("Fresnel color"), CurrentColor);
    }
}

void ARandomBomb::ScheduleNextExplosion()
{
    CurrentInterval = FMath::FRandRange(MinInterval, MaxInterval);
    ElapsedTime = 0.f;
    
    StaticMesh->SetWorldScale3D(FVector(MinScale));

    GetWorld()->GetTimerManager().SetTimer(
        ExplosionTimerHandle,
        this,
        &ARandomBomb::Explode,
        CurrentInterval,
        false
    );
}

void ARandomBomb::Explode()
{
    if (ExplosionParticle)
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation(), GetActorRotation(), FVector(MaxScale), true);

    if (ExplosionSound)
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());

    TArray<AActor*> OverlappingActors;
    Collision->GetOverlappingActors(OverlappingActors, AMyCharacter::StaticClass());

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor->ActorHasTag("Player"))
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Actor Found"));
            UGameplayStatics::ApplyDamage(Actor, DamageAmount, nullptr, this, UDamageType::StaticClass());

            // 폭발 반대 방향으로 넉백
            FVector KnockbackDir = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
            KnockbackDir.Z = 0.2f;
            KnockbackDir.Normalize();
            
            if (ACharacter* Character = Cast<ACharacter>(Actor))
                Character->LaunchCharacter(KnockbackDir * KnockbackPower, true, true);
        }
    }

    ScheduleNextExplosion();
}