#include "HotbarComponent.h"

UHotbarComponent::UHotbarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHotbarComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Slots.Num() == 0)
	{
		Slots = {
			EItemType::Pickaxe,
			EItemType::StoneBlock,
			EItemType::Planks,
			EItemType::Wood,
			EItemType::Stone,
			EItemType::None
		};
	}

	Slots.SetNum(SlotCount);
}

void UHotbarComponent::SelectSlot(int32 Index)
{
	if (SlotCount <= 0)
	{
		return;
	}

	SelectedSlot = ((Index % SlotCount) + SlotCount) % SlotCount;
}

void UHotbarComponent::CycleSlot(int32 Direction)
{
	SelectSlot(SelectedSlot + Direction);
}

EItemType UHotbarComponent::GetSlotItem(int32 Index) const
{
	return Slots.IsValidIndex(Index) ? Slots[Index] : EItemType::None;
}
