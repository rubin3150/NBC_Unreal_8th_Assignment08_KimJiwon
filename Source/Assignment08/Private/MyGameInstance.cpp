#include "MyGameInstance.h"

UMyGameInstance::UMyGameInstance()
{
	TotalScore = 0;
	CurrentLevelIndex = 0;
}

void UMyGameInstance::AddToScore(int32 Amount)
{
	TotalScore += Amount;
	UE_LOG(LogTemp, Warning, TEXT("TotalScore Updated = %d"), TotalScore);
}