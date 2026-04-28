#pragma once

#include "CoreMinimal.h"
#include "engine/DataTable.h"
#include "ItemSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct FItemSpawnRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance = 0.0f;
};