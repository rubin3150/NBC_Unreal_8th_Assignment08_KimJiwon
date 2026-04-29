#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spike.generated.h"

class UBoxComponent;
class ASpawnVolume;

UENUM()
enum class ESpikeState : uint8
{
	Hidden,
	Rising,
	Active
};

UCLASS()
class ASSIGNMENT08_API ASpike : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	ASpike();
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spike")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spike")
	UBoxComponent* Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spike")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Spike")
	float DamageAmount;

	UPROPERTY()
	ASpawnVolume* CachedSpawnVolume;
	
	bool bCanDamage;
	
	void MoveToRandomPoint();
	void SetZ(float Z);
	void TryDamagePlayer();
};