#include "MyCharacter.h"
#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "MyGameState.h"
#include "Camera/CameraComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	NormalSpeed = 600.f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	
	MaxHealth = 100;
	Health = MaxHealth;
	
	SlowMultiplier = 1.f;
	bIsControlReversed = false;
	SlowTotalDuration = 0.f;
	ReverseTotalDuration = 0.f;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;
	
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(GetMesh());
	OverHeadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateOverHeadHP();
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this,&AMyCharacter::Move);
			if (PlayerController->LookAction)
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
			if (PlayerController->JumpAction)
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Triggered, this, &AMyCharacter::StartJump);
			if (PlayerController->JumpAction)
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Completed, this, &AMyCharacter::StopJump);
			if (PlayerController->SprintAction)
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Triggered, this, &AMyCharacter::StartSprint);
			if (PlayerController->SprintAction)
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this, &AMyCharacter::StopSprint);
		}
	}
}

void AMyCharacter::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	FVector2D MoveInput = value.Get<FVector2D>();
	
	// ReverseItem
	if (bIsControlReversed)
		MoveInput = -MoveInput;
	
	if (!FMath::IsNearlyZero(MoveInput.X))
		AddMovementInput(GetActorForwardVector(), MoveInput.X);

	if (!FMath::IsNearlyZero(MoveInput.Y))
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
}

void AMyCharacter::Look(const FInputActionValue& value)
{
	FVector2d LookInput = value.Get<FVector2d>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AMyCharacter::StartJump(const FInputActionValue& value)
{
	if (value.Get<bool>())
		Jump();
}

void AMyCharacter::StopJump(const FInputActionValue& value)
{
	if (!value.Get<bool>())
		StopJumping();
}

void AMyCharacter::StartSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed * SlowMultiplier;
}

void AMyCharacter::StopSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * SlowMultiplier;
}

float AMyCharacter::GetHealth() const
{
	return Health;
}

void AMyCharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverHeadHP();
}

float AMyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                               class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);
	UpdateOverHeadHP();
	
	if (Health <= 0.0f)
		OnDeath();
		
	return ActualDamage;
}

void AMyCharacter::OnDeath()
{
	AMyGameState* MyGameState = GetWorld() ? GetWorld()->GetGameState<AMyGameState>() : nullptr;
	if (MyGameState)
		MyGameState->OnGameOver();
}

void AMyCharacter::UpdateOverHeadHP()
{
	if (!OverHeadWidget) return;
	
	UUserWidget* OverheadWidgetInstance = OverHeadWidget->GetUserWidgetObject();
	
	if (!OverheadWidgetInstance) return;
	
	// if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName("OverHeadHP")))
		// HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	
	// HPText에서 HPBar로 변경
	if (UProgressBar* HPBar = Cast<UProgressBar>(OverheadWidgetInstance->GetWidgetFromName("OverHeadHPBar")))
	{
		HPBar->SetPercent(Health / MaxHealth);
		FLinearColor BarColor = FLinearColor::LerpUsingHSV(FLinearColor::Red, FLinearColor::Green, Health / MaxHealth);
		HPBar->SetFillColorAndOpacity(BarColor);
	}
}

void AMyCharacter::ApplySlowDebuff(float Duration, float Multiplier)
{
	SlowMultiplier = Multiplier;
	SlowTotalDuration = Duration;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed * SlowMultiplier;

	GetWorldTimerManager().SetTimer(
		SlowTimerHandle,
		this,
		&AMyCharacter::EndSlowDebuff,
		Duration,
		false
	);
}

void AMyCharacter::EndSlowDebuff()
{
	SlowMultiplier = 1.f;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void AMyCharacter::ApplyReverseDebuff(float Duration)
{
	bIsControlReversed = true;
	ReverseTotalDuration = Duration;

	GetWorldTimerManager().SetTimer(
		ReverseTimerHandle,
		this,
		&AMyCharacter::EndReverseDebuff,
		Duration,
		false
	);
}

void AMyCharacter::EndReverseDebuff()
{
	bIsControlReversed = false;
}