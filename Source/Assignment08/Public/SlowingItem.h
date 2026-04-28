#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "SlowingItem.generated.h"

UCLASS()
class ASSIGNMENT08_API ASlowingItem : public ABaseItem
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateItem(AActor* Activator) override;
	
public:
	ASlowingItem();

	UPROPERTY(EditAnywhere, Category = "Debuff")
	float SlowDuration;

	UPROPERTY(EditAnywhere, Category = "Debuff")
	float SlowMultiplier;
};
