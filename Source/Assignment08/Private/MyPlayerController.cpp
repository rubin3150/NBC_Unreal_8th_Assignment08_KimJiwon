#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "MyGameInstance.h"
#include "MyGameState.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

AMyPlayerController::AMyPlayerController()
	: InputMappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr),
	JumpAction(nullptr),
	SprintAction(nullptr),
	HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr)
{

}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			if (InputMappingContext)
				Subsystem->AddMappingContext(InputMappingContext, 0);
	
	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MenuLevel"))
		ShowMainMenu(false);
}
 
UUserWidget* AMyPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

void AMyPlayerController::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}
	
	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			
			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());

			if (AMyGameState* MyGameState = GetWorld() ? GetWorld()->GetGameState<AMyGameState>() : nullptr)
				MyGameState->UpdateHUD();
		}
	}
}

void AMyPlayerController::ShowMainMenu(bool bIsRestart)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}
	
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}
	
	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();
			
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
			
			if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
			{
				if (bIsRestart)
					ButtonText->SetText(FText::FromString(TEXT("Restart")));
				else
					ButtonText->SetText(FText::FromString(TEXT("Start")));
			}
			
			if (bIsRestart)
			{
				UFunction* PlayAnimFunc = MainMenuWidgetClass->FindFunctionByName(FName("PlayGameOverAnim"));
				if (PlayAnimFunc)
					MainMenuWidgetInstance->ProcessEvent(PlayAnimFunc, nullptr);
				
				if (UTextBlock* TotalScoreText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("TotalScoreText"))))
				{
					if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
						TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("Total Score : %d"), MyGameInstance->TotalScore)));
				}
			}
		}
	}
}

void AMyPlayerController::StartGame()
{
	if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		MyGameInstance->CurrentLevelIndex = 0;
		MyGameInstance->TotalScore = 0;
	}
	
	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
	SetPause(false);
}

void AMyPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}

void AMyPlayerController::ReturnToMainMenu()
{
	if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		MyGameInstance->CurrentLevelIndex = 0;
		MyGameInstance->TotalScore = 0;
	}
    
	UGameplayStatics::OpenLevel(GetWorld(), FName("MenuLevel"));
	SetPause(false);
}

void AMyPlayerController::PlayHUDAnimation(FName AnimFunctionName, FName WidgetName)
{
	if (!HUDWidgetInstance) return;

	if (!WidgetName.IsNone())
		if (UWidget* Widget = HUDWidgetInstance->GetWidgetFromName(WidgetName))
			Widget->SetVisibility(ESlateVisibility::Visible);

	if (UFunction* Func = HUDWidgetInstance->GetClass()->FindFunctionByName(AnimFunctionName))
		HUDWidgetInstance->ProcessEvent(Func, nullptr);
}

void AMyPlayerController::SetHUDText(FName WidgetName, const FString& Text)
{
	if (!HUDWidgetInstance) return;

	if (UTextBlock* TextBlock = Cast<UTextBlock>(HUDWidgetInstance->GetWidgetFromName(WidgetName)))
		TextBlock->SetText(FText::FromString(Text));
}