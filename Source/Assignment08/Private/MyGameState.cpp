#include "MyGameState.h"

#include "CoinItem.h"
#include "EngineUtils.h"
#include "MineItem.h"
#include "MyCharacter.h"
#include "MyGameInstance.h"
#include "MyPlayerController.h"
#include "SpawnVolume.h"
#include "Spike.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/RadialSlider.h"


AMyGameState::AMyGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	LevelItemCounts = { 50, 40, 30 };
	LevelDurations = { 60.0f, 45.0f, 30.0f };
	CurrentLevelIndex = 0;
	MaxLevels = 3;
}

void AMyGameState::BeginPlay()
{
	Super::BeginPlay();
	
	StartLevel();
	
	GetWorldTimerManager().SetTimer(HUDUpdateTimerHandle, this, &AMyGameState::UpdateHUD, 0.1f, true);
}

int32 AMyGameState::GetScore() const
{
	return Score;
}

void AMyGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance))
			MyGameInstance->AddToScore(Amount);
	}
}

void AMyGameState::StartLevel()
{
	FString CurrentMap = GetWorld()->GetMapName();
	if (CurrentMap.Contains("MenuLevel")) return;
	
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		if (AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController))
			MyPlayerController->ShowGameHUD();
	
	if (UGameInstance* GameInstance = GetGameInstance())
		if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance))
			CurrentLevelIndex = MyGameInstance->CurrentLevelIndex;
	
	// 웨이브 시작 알림
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,FString::Printf(TEXT("Wave %d 시작!"), CurrentLevelIndex + 1));
	
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	
	// 아이템 스폰 개수 50, 40, 30
	const int32 ItemToSpawn = LevelItemCounts[CurrentLevelIndex]; 
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("아이템 생성 개수: %d"), LevelItemCounts[CurrentLevelIndex]));
	for (int32 i = 0; i < ItemToSpawn; ++i)
	{
		if (FoundVolumes.Num() > 0)
		{
			if (ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]))
			{
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
					SpawnedCoinCount++;
			}
		}
	}
	
	// 코인 개수 초기화
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
		PC->SetHUDText(FName("Coin"), FString::Printf(TEXT("Coin: %d / %d"), CollectedCoinCount, SpawnedCoinCount));
	
	// 웨이브 제한 시간 60.0f, 45.0f, 30.0f
	GetWorldTimerManager().SetTimer(
		LevelTimerHandle, 
		this, 
		&AMyGameState::OnLevelTimeUp, 
		LevelDurations[CurrentLevelIndex], 
		false
	);

	// 스파이크 추가
	if (CurrentLevelIndex == 1)
	{
		for (TActorIterator<ASpike> It(GetWorld()); It; ++It)
			AllSpikes.Add(*It);

		InitSpikePositions();
		ActivateRandomSpikes();
		
		GetWorldTimerManager().SetTimer(
			SpikeTimerHandle,
			this,
			&AMyGameState::ActivateRandomSpikes,
			10.0f,
			true
		);
	}
}

void AMyGameState::OnLevelTimeUp()
{
	EndLevel();
}

void AMyGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		FString Msg = FString::Printf(TEXT("Coin: %d / %d"), CollectedCoinCount, SpawnedCoinCount);
		PC->SetHUDText(FName("Coin"), Msg);
		PC->PlayHUDAnimation(FName("PlayCoinCollectedAnim"));
	}
	
	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
		EndLevel();
}

void AMyGameState::EndLevel()
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);
	GetWorldTimerManager().ClearTimer(SpikeTimerHandle);
	AllSpikes.Empty();
	
	TArray<AActor*> AllItems;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseItem::StaticClass(), AllItems);
	for (AActor* Item : AllItems)
		Item->GetWorldTimerManager().ClearAllTimersForObject(Item);
	
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GameInstance))
		{
			AddScore(Score);
			CurrentLevelIndex++;
			MyGameInstance->CurrentLevelIndex = CurrentLevelIndex;
		}
	}
	
	if (CurrentLevelIndex >= MaxLevels)
	{
		OnGameOver();
		return;
	}
	
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	else
		OnGameOver();
}

void AMyGameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		if (AMyPlayerController* MyPlayerController = Cast<AMyPlayerController>(PlayerController))
		{
			MyPlayerController->SetPause(true);
			MyPlayerController->ShowMainMenu(true);
		}
}

void AMyGameState::UpdateHUD()
{
	AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC) return;

	float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
	PC->SetHUDText(FName("Time"), FString::Printf(TEXT("Time: %.1f"), RemainingTime));

	if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
		PC->SetHUDText(FName("Score"), FString::Printf(TEXT("Score: %d"), GI->TotalScore));

	PC->SetHUDText(FName("Level"), FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1));
	
	// 디버프 슬라이더 갱신
	AMyCharacter* Character = Cast<AMyCharacter>(PC->GetPawn());
	if (!Character) return;

	UUserWidget* HUDWidget = PC->GetHUDWidget();
	if (!HUDWidget) return;

	if (URadialSlider* SlowSlider = Cast<URadialSlider>(HUDWidget->GetWidgetFromName("SlowSlider")))
	{
		float Remaining = GetWorldTimerManager().GetTimerRemaining(Character->GetSlowTimerHandle());
		float Total = Character->GetSlowTotalDuration();
		float Angle = (Total > 0.f) ? (Remaining / Total) * 360.f : 0.f;
		SlowSlider->SetSliderHandleEndAngle(Angle);
	}

	if (URadialSlider* ReverseSlider = Cast<URadialSlider>(HUDWidget->GetWidgetFromName("ReverseSlider")))
	{
		float Remaining = GetWorldTimerManager().GetTimerRemaining(Character->GetReverseTimerHandle());
		float Total = Character->GetReverseTotalDuration();
		float Angle = (Total > 0.f) ? (Remaining / Total) * 360.f : 0.f;
		ReverseSlider->SetSliderHandleEndAngle(Angle);
	}
}

void AMyGameState::InitSpikePositions()
{
	SpikePositions.Empty();

	const float XMin = -2390.f, XMax = 2390.f;
	const float YMin = -2200.f, YMax = 2200.f;
	const float Step = 200.f;

	for (float X = XMin; X <= XMax; X += Step)
		for (float Y = YMin; Y <= YMax; Y += Step)
			SpikePositions.Add(FVector(X, Y, 0.f));
}

void AMyGameState::ActivateRandomSpikes()
{
	for (ASpike* Spike : AllSpikes)
		if (IsValid(Spike))
			Spike->ReturnToGround();

	TArray<int32> Indices;
	for (int32 i = 0; i < SpikePositions.Num(); i++)
		Indices.Add(i);

	for (int32 i = Indices.Num() - 1; i > 0; i--)
		Indices.Swap(i, FMath::RandRange(0, i));

	for (int32 i = 0; i < AllSpikes.Num(); i++)
		if (IsValid(AllSpikes[i]))
			AllSpikes[i]->RiseUp(SpikePositions[Indices[i]]);

	if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController()))
		PC->PlayHUDAnimation(FName("PlaySpikeAlertAnim"), FName("Spike"));
}