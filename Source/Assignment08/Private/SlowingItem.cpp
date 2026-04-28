#include "SlowingItem.h"
#include "MyCharacter.h"

ASlowingItem::ASlowingItem()
{
	SlowDuration = 5.f;
	SlowMultiplier = 0.5f;
	ItemType = "Slow";
}

void ASlowingItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (AMyCharacter* Character = Cast<AMyCharacter>(Activator))
		Character->ApplySlowDebuff(SlowDuration, SlowMultiplier);

	DestroyItem();
}