#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "ReverseItem.generated.h"

UCLASS()
class ASSIGNMENT08_API AReverseItem : public ABaseItem
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateItem(AActor* Activator) override;
	
public:
	AReverseItem();

	UPROPERTY(EditAnywhere, Category = "Debuff")
	float ReverseDuration;
};