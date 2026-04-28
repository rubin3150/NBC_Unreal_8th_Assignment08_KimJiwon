#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

class ASpike;

UCLASS()
class ASSIGNMENT08_API AMyGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AMyGameState();
	
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Score;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int SpawnedCoinCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 CollectedCoinCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TArray<int32> LevelItemCounts;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<float> LevelDurations;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevels;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spike")
	TArray<ASpike*> AllSpikes;
	
	TArray<FVector> SpikePositions;

	FTimerHandle LevelTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;
	FTimerHandle SpikeTimerHandle;

	UFUNCTION(BlueprintPure, Category="Score")
	int32 GetScore() const;
	
	UFUNCTION(BlueprintCallable, Category="Score")
	void AddScore(int32 Amount);
	UFUNCTION(BlueprintCallable, Category="Level")
	void OnGameOver();

	void StartLevel();
	void OnLevelTimeUp();
	void OnCoinCollected();
	void EndLevel();
	void UpdateHUD();
	void InitSpikePositions();
	void ActivateRandomSpikes();
};
