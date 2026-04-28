#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spike.generated.h"

class UBoxComponent;

UCLASS()
class ASSIGNMENT08_API ASpike : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpike();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spike")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spike")
	UBoxComponent* Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spike")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, Category = "Spike")
	float DamageAmount;
	
	UPROPERTY(EditAnywhere, Category = "Spike")
	float RiseDuration;
	
	float ElapsedTime;
	bool bIsRising;
	bool bCanDamage;

	virtual void Tick(float DeltaTime) override;
	void RiseUp(const FVector& InTargetPosition);
	void ReturnToGround();
};