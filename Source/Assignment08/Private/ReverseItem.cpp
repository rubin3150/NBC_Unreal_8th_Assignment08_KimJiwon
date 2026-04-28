#include "ReverseItem.h"
#include "MyCharacter.h"

AReverseItem::AReverseItem()
{
	ReverseDuration = 5.f;
	ItemType = "Reverse";
}

void AReverseItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (AMyCharacter* Character = Cast<AMyCharacter>(Activator))
		Character->ApplyReverseDebuff(ReverseDuration);

	DestroyItem();
}