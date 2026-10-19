#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	BroadcastHealth();
}

void UHealthComponent::BroadcastHealth()
{
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void UHealthComponent::ApplyDamage(float Amount)
{
	if (bIsDead || Amount <= 0.0f)
	{
		return;
	}

	Health = FMath::Max(0.0f, Health - Amount);
	BroadcastHealth();

	if (Health <= 0.0f)
	{
		bIsDead = true;
		OnDeath.Broadcast();
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.0f)
	{
		return;
	}

	Health = FMath::Min(MaxHealth, Health + Amount);
	BroadcastHealth();
}

void UHealthComponent::Kill()
{
	if (bIsDead)
	{
		return;
	}

	Health = 0.0f;
	BroadcastHealth();
	bIsDead = true;
	OnDeath.Broadcast();
}
