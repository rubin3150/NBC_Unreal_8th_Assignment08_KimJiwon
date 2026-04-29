#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

class AMyPlayerController;
class ASpike;
class ARandomBomb;
class ASpawnVolume;

UCLASS()
class ASSIGNMENT08_API AMyGameState : public AGameState
{
    GENERATED_BODY()
    
protected:
    virtual void BeginPlay() override;
    
public:
    AMyGameState();
    
    virtual void Tick(float DeltaTime) override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Score")
    int32 Score;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
    int SpawnedCoinCount;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
    int32 CollectedCoinCount;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
    int32 CurrentLevelIndex;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
    int32 MaxLevels;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
    int32 CurrentWaveIndex;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
    int32 MaxWaves;
    
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    // TArray<int32> WaveItemCounts;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
    TArray<float> WaveDurations;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
    TArray<FName> LevelMapNames;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traps")
    TArray<AActor*> SpawnedTraps;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traps")
    TArray<int32> TrapCounts;
    UPROPERTY(EditDefaultsOnly, Category="Traps")
    TSubclassOf<ASpike> SpikeClass;
    UPROPERTY(EditDefaultsOnly, Category="Traps")
    TSubclassOf<ARandomBomb> RandomBombClass;
    
    UPROPERTY(EditAnywhere, Category = "Spike")
    float SpikeRiseDuration;
    UPROPERTY(EditAnywhere, Category = "Spike")
    float SpikeActiveDuration;
    UPROPERTY(EditAnywhere, Category = "Spike")
    float SpikeHideDuration;
    
    UPROPERTY()
    TArray<ASpike*> ActiveSpikes;
    
    float SpikeRiseElapsed;
    bool bSpikesRising;
    
    FTimerHandle LevelTimerHandle;
    FTimerHandle HUDUpdateTimerHandle;
    FTimerHandle SpikeRiseTimerHandle;
    FTimerHandle SpikeHideTimerHandle;

    UFUNCTION(BlueprintPure, Category="Score")
    int32 GetScore() const;
    
    UFUNCTION(BlueprintCallable, Category="Score")
    void AddScore(int32 Amount);
    UFUNCTION(BlueprintCallable, Category="Level")
    void OnGameOver();

    void StartLevel();
    void OnLevelTimeUp();
    void OnCoinCollected();
    void StartWave();
    void EndWave();
    void EndLevel();
    
    void SpawnTraps(ASpawnVolume* SpawnVolume, TSubclassOf<AActor> TrapClass, int32 Count);
    void TriggerSpikeRise();
    void TriggerSpikeHide();
    
    void UpdateHUD();
    AMyPlayerController* GetMyPC() const;
};