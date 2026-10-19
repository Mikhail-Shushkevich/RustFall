#include "NeedsComponent.h"

UNeedsComponent::UNeedsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNeedsComponent::BeginPlay()
{
	Super::BeginPlay();

	BroadcastNeed("Hunger", Hunger, 100.0f);
	BroadcastNeed("Thirst", Thirst, 100.0f);
	BroadcastNeed("Stamina", Stamina, 100.0f);
	BroadcastNeed("Temperature", Temperature, 100.0f);
}

void UNeedsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float ActivityMultiplier = bIsSprinting ? 1.6f : 1.0f;

	Hunger = FMath::Max(0.0f, Hunger - HungerDrainPerSecond * ActivityMultiplier * DeltaTime);
	BroadcastNeed("Hunger", Hunger, 100.0f);

	Thirst = FMath::Max(0.0f, Thirst - ThirstDrainPerSecond * ActivityMultiplier * DeltaTime);
	BroadcastNeed("Thirst", Thirst, 100.0f);

	if (bIsSprinting && Stamina > 0.0f)
	{
		Stamina = FMath::Max(0.0f, Stamina - StaminaDrainPerSecond * DeltaTime);
		BroadcastNeed("Stamina", Stamina, 100.0f);
	}
	else if (!bIsSprinting && !bSprintInputHeld && Stamina < 100.0f)
	{
		const float RegenMultiplier = (IsStarving() || IsDehydrated()) ? 0.5f : 1.0f;
		Stamina = FMath::Min(100.0f, Stamina + StaminaRegenPerSecond * RegenMultiplier * DeltaTime);
		BroadcastNeed("Stamina", Stamina, 100.0f);
	}

	float TargetTemp;
	if (AmbientTemperature >= NeutralTemperatureMin && AmbientTemperature <= NeutralTemperatureMax)
	{
		TargetTemp = NominalBodyTemperature;
	}
	else
	{
		TargetTemp = FMath::Clamp(AmbientTemperature, 0.0f, 60.0f);
	}

	if (!FMath::IsNearlyEqual(Temperature, TargetTemp, 0.05f))
	{
		Temperature = FMath::FInterpConstantTo(Temperature, TargetTemp, DeltaTime, TemperatureAdjustSpeed);
		BroadcastNeed("Temperature", Temperature, 100.0f);
	}

	if (IsStarving() && !bHungerDepleted)
	{
		bHungerDepleted = true;
		OnNeedDepleted.Broadcast("Hunger");
	}
	else if (!IsStarving())
	{
		bHungerDepleted = false;
	}

	if (IsDehydrated() && !bThirstDepleted)
	{
		bThirstDepleted = true;
		OnNeedDepleted.Broadcast("Thirst");
	}
	else if (!IsDehydrated())
	{
		bThirstDepleted = false;
	}
}

void UNeedsComponent::BroadcastNeed(FName NeedName, float Value, float Max)
{
	OnNeedChanged.Broadcast(NeedName, Value, Max);
}

void UNeedsComponent::SetSprintInputHeld(bool bHeld)
{
	bSprintInputHeld = bHeld;
}

void UNeedsComponent::AddHunger(float Amount)
{
	Hunger = FMath::Clamp(Hunger + Amount, 0.0f, 100.0f);
	BroadcastNeed("Hunger", Hunger, 100.0f);
}

void UNeedsComponent::AddThirst(float Amount)
{
	Thirst = FMath::Clamp(Thirst + Amount, 0.0f, 100.0f);
	BroadcastNeed("Thirst", Thirst, 100.0f);
}

void UNeedsComponent::AddStamina(float Amount)
{
	Stamina = FMath::Clamp(Stamina + Amount, 0.0f, 100.0f);
	BroadcastNeed("Stamina", Stamina, 100.0f);
}

void UNeedsComponent::Eat(float Amount)
{
	AddHunger(Amount);
}

void UNeedsComponent::Drink(float Amount)
{
	AddThirst(Amount);
}
