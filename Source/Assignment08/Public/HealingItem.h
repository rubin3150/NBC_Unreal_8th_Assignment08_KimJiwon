#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "HealingItem.generated.h"

UCLASS()
class ASSIGNMENT08_API AHealingItem : public ABaseItem
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateItem(AActor* Activator) override;
	
public:
	AHealingItem();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 HealAmount;
};