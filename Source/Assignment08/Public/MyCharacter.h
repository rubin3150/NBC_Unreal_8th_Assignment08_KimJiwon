#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacter.generated.h"

class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class ASSIGNMENT08_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverHeadWidget;
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const;
	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float Amount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="health")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="health")
	float Health;
	
	float NormalSpeed;
	float SprintSpeedMultiplier;
	float SprintSpeed;
	float SlowMultiplier;
	bool bIsControlReversed;
	float SlowTotalDuration;
	float ReverseTotalDuration;

	FTimerHandle SlowTimerHandle;
	FTimerHandle ReverseTimerHandle;

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);
	
	void OnDeath();
	void UpdateOverHeadHP();
	
public:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	void ApplySlowDebuff(float Duration, float Multiplier);
	void ApplyReverseDebuff(float Duration);
	void EndSlowDebuff();
	void EndReverseDebuff();
	FTimerHandle& GetSlowTimerHandle() { return SlowTimerHandle; }
	FTimerHandle& GetReverseTimerHandle() { return ReverseTimerHandle; }
	float GetSlowTotalDuration() const { return SlowTotalDuration; }
	float GetReverseTotalDuration() const { return ReverseTotalDuration; }
};
