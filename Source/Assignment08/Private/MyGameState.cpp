#include "MyGameState.h"

#include "CoinItem.h"
#include "MineItem.h"
#include "MyCharacter.h"
#include "MyGameInstance.h"
#include "MyPlayerController.h"
#include "SpawnVolume.h"
#include "Spike.h"
#include "RandomBomb.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/RadialSlider.h"


AMyGameState::AMyGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	// WaveItemCounts = { 10, 15, 25 };
	WaveDurations = { 60.0f, 45.0f, 30.0f };
	TrapCounts = { 15, 20, 30 };
	CurrentWaveIndex = 0;
	CurrentLevelIndex = 0;
	MaxWaves = 3;
	MaxLevels = 3;
	
	SpikeRiseDuration = 5.0f;
	SpikeActiveDuration = 5.0f;
	SpikeHideDuration = 3.0f;
	SpikeRiseElapsed = 0.f;
	bSpikesRising = false;
}

void AMyGameState::BeginPlay()
{
	Super::BeginPlay();
	
	StartLevel();
	
	GetWorldTimerManager().SetTimer(HUDUpdateTimerHandle, this, &AMyGameState::UpdateHUD, 0.1f, true);
}

void AMyGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bSpikesRising) return;
	
	SpikeRiseElapsed += DeltaTime;
	float Alpha = FMath::Clamp(SpikeRiseElapsed / SpikeRiseDuration, 0.f, 1.f);
	float Z = FMath::Lerp(-200.f, 0.f, Alpha);
	
	for (ASpike* Spike : ActiveSpikes)
		if (IsValid(Spike))
			Spike->SetZ(Z);
	
	if (Alpha >= 1.f)
	{
		bSpikesRising = false;
		for (ASpike* Spike : ActiveSpikes)
			if (IsValid(Spike))
				Spike->bCanDamage = true;
	}
}

int32 AMyGameState::GetScore() const
{
	return Score;
}

void AMyGameState::AddScore(int32 Amount)
{
	if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GetGameInstance()))
		MyGameInstance->AddToScore(Amount);
}

void AMyGameState::StartLevel()
{
	FString CurrentMap = GetWorld()->GetMapName();
	if (CurrentMap.Contains("MenuLevel")) return;
	
	if (AMyPlayerController* PC = GetMyPC())
		PC->ShowGameHUD();
	
	if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GetGameInstance()))
		CurrentLevelIndex = MyGameInstance->CurrentLevelIndex;
	
	CurrentWaveIndex = 0;
	StartWave();
}

void AMyGameState::StartWave()
{
	// 웨이브 시작 알림
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
		FString::Printf(TEXT("Level %d - Wave %d 시작!"), CurrentLevelIndex + 1, CurrentWaveIndex + 1));
	
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
    
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	
	ASpawnVolume* SpawnVolume = (FoundVolumes.Num() > 0) ? Cast<ASpawnVolume>(FoundVolumes[0]) : nullptr;
	if (!SpawnVolume) return;
    
	// 현재 웨이브의 아이템 개수
	//const int32 ItemToSpawn = WaveItemCounts[CurrentWaveIndex];
	const int32 ItemToSpawn = (CurrentLevelIndex * MaxWaves + CurrentWaveIndex + 1) * 10; // 아이템 개수를 웨이브마다 순차적으로 늘려서 밸런스 조절
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
		FString::Printf(TEXT("아이템 생성 개수: %d"), ItemToSpawn));
    
	for (int32 i = 0; i < ItemToSpawn; ++i)
	{
		if (AActor* SpawnedActor = SpawnVolume->SpawnRandomItem(); SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
			SpawnedCoinCount++;
	}
    
	// 웨이브 2: 스파이크 스폰
	if (CurrentWaveIndex == 1 && SpikeClass)
	{
		SpawnTraps(SpawnVolume, SpikeClass, TrapCounts[CurrentLevelIndex]);
		TriggerSpikeRise();
	
		const float CycleTotal = SpikeRiseDuration + SpikeActiveDuration + SpikeHideDuration;
		GetWorldTimerManager().SetTimer(SpikeRiseTimerHandle, this, &AMyGameState::TriggerSpikeRise, CycleTotal, true);
	}
    
	// 웨이브 3: 랜덤 폭탄 스폰
	if (CurrentWaveIndex == 2 && RandomBombClass)
		SpawnTraps(SpawnVolume, RandomBombClass, TrapCounts[CurrentLevelIndex]);
	
	if (AMyPlayerController* PC = GetMyPC())
		PC->SetHUDText(FName("Coin"), FString::Printf(TEXT("Coin: %d / %d"), CollectedCoinCount, SpawnedCoinCount));
    
	GetWorldTimerManager().SetTimer(LevelTimerHandle, this, &AMyGameState::OnLevelTimeUp, WaveDurations[CurrentWaveIndex], false);
}

void AMyGameState::SpawnTraps(ASpawnVolume* SpawnVolume, TSubclassOf<AActor> TrapClass, int32 Count)
{
	if (!SpawnVolume || !TrapClass) return;
    
	for (int32 i = 0; i < Count; ++i)
	{
		if (AActor* SpawnedTrap = SpawnVolume->SpawnAtRandomPoint(TrapClass))
		{
			SpawnedTraps.Add(SpawnedTrap);
			if (ASpike* Spike = Cast<ASpike>(SpawnedTrap))
				ActiveSpikes.Add(Spike);
		}
	}
}

void AMyGameState::TriggerSpikeRise()
{
	for (ASpike* Spike : ActiveSpikes)
		if (IsValid(Spike))
			Spike->MoveToRandomPoint();
	
	SpikeRiseElapsed = 0.f;
	bSpikesRising = true;
    
	GetWorldTimerManager().SetTimer(SpikeHideTimerHandle, this, &AMyGameState::TriggerSpikeHide,
		SpikeRiseDuration + SpikeActiveDuration, false);
    
	if (AMyPlayerController* PC = GetMyPC())
		PC->PlayHUDAnimation(FName("PlaySpikeAlertAnim"), FName("Spike"));
}

void AMyGameState::TriggerSpikeHide()
{
	for (ASpike* Spike : ActiveSpikes)
	{
		if (IsValid(Spike))
		{
			Spike->SetZ(-200.f);
			Spike->bCanDamage = false;
		}
	}
}

void AMyGameState::OnLevelTimeUp()
{
	EndWave();
}

void AMyGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	
	if (AMyPlayerController* PC = GetMyPC())
	{
		PC->SetHUDText(FName("Coin"), FString::Printf(TEXT("Coin: %d / %d"), CollectedCoinCount, SpawnedCoinCount));
		PC->PlayHUDAnimation(FName("PlayCoinCollectedAnim"));
	}
	
	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
		EndWave();
}

void AMyGameState::EndWave()
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);
	GetWorldTimerManager().ClearTimer(SpikeRiseTimerHandle);
	GetWorldTimerManager().ClearTimer(SpikeHideTimerHandle);
	bSpikesRising = false;
	
	// 함정 전부 정리
	for (AActor* Trap : SpawnedTraps)
		if (IsValid(Trap))
			Trap->Destroy();
	SpawnedTraps.Empty();
	ActiveSpikes.Empty();
	
	// 아이템 전부 정리
	TArray<AActor*> AllItems;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseItem::StaticClass(), AllItems);
	for (AActor* Item : AllItems)
		if (IsValid(Item))
			Item->Destroy();
	
	CurrentWaveIndex++;
    
	if (CurrentWaveIndex >= MaxWaves)
	{
		EndLevel();
		return;
	}
    
	StartWave();
}

void AMyGameState::EndLevel()
{
	if (UMyGameInstance* MyGameInstance = Cast<UMyGameInstance>(GetGameInstance()))
	{
		AddScore(Score);
		CurrentLevelIndex++;
		MyGameInstance->CurrentLevelIndex = CurrentLevelIndex;
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
	if (AMyPlayerController* PC = GetMyPC())
	{
		PC->SetPause(true);
		PC->ShowMainMenu(true);
	}
}

void AMyGameState::UpdateHUD()
{
	AMyPlayerController* PC = GetMyPC();
	if (!PC) return;

	float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
	PC->SetHUDText(FName("Time"), FString::Printf(TEXT("Time: %.1f"), RemainingTime));

	if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
		PC->SetHUDText(FName("Score"), FString::Printf(TEXT("Score: %d"), GI->TotalScore));

	PC->SetHUDText(FName("Level"), FString::Printf(TEXT("Level %d : Wave %d"), CurrentLevelIndex + 1, CurrentWaveIndex + 1));
	
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

AMyPlayerController* AMyGameState::GetMyPC() const
{
	return Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
}