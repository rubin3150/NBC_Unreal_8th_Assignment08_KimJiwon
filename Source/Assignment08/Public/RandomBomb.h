#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RandomBomb.generated.h"

UCLASS()
class ASSIGNMENT08_API ARandomBomb : public AActor
{
	GENERATED_BODY()
	
public:	
	ARandomBomb();

protected:
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bomb", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	UParticleSystem* ExplosionParticle;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float DamageAmount;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float MinInterval;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float MaxInterval;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float KnockbackPower;
	
	UPROPERTY(EditAnywhere, Category = "Bomb")
	float MinScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "Bomb")
	float MaxScale = 6.f;
	
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	float ElapsedTime = 0.f;
	float CurrentInterval = 0.f;
	FTimerHandle ExplosionTimerHandle;
	
	void Explode();
	void ScheduleNextExplosion();
};
