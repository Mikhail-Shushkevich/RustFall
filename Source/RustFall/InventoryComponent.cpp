#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddItem(EItemType Type, int32 Amount)
{
	if (Type == EItemType::None || Amount <= 0)
	{
		return;
	}

	int32& Count = Items.FindOrAdd(Type);
	Count += Amount;
	OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::RemoveItem(EItemType Type, int32 Amount)
{
	if (Type == EItemType::None || Amount <= 0)
	{
		return false;
	}

	int32* Count = Items.Find(Type);
	if (!Count || *Count < Amount)
	{
		return false;
	}

	*Count -= Amount;
	if (*Count <= 0)
	{
		Items.Remove(Type);
	}

	OnInventoryChanged.Broadcast();
	return true;
}

int32 UInventoryComponent::GetItemCount(EItemType Type) const
{
	const int32* Count = Items.Find(Type);
	return Count ? *Count : 0;
}
