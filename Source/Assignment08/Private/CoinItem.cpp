#include "CoinItem.h"

#include "MyGameState.h"
#include "Engine/World.h"

ACoinItem::ACoinItem()
{
    PointValue = 0;
    ItemType = "DefaultCoin";
}

void ACoinItem::ActivateItem(AActor* Activator)
{
    Super::ActivateItem(Activator);
    
    if (Activator && Activator->ActorHasTag("Player"))
    {
        if (UWorld* World = GetWorld())
            if (AMyGameState* GameState = World->GetGameState<AMyGameState>())
            {
                GameState->AddScore(PointValue);
                GameState->OnCoinCollected();
            }
        
        DestroyItem();
    }
}