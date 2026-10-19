#include "CraftingComponent.h"
#include "InventoryComponent.h"
#include "GameFramework/Actor.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Recipes.Num() == 0)
	{
		FRecipe Planks;
		Planks.DisplayName = TEXT("Planks");
		Planks.Cost.Add(EItemType::Wood, 10);
		Planks.OutputItem = EItemType::Planks;
		Planks.OutputAmount = 5;
		Recipes.Add(Planks);

		FRecipe StoneBlock;
		StoneBlock.DisplayName = TEXT("Stone Block");
		StoneBlock.Cost.Add(EItemType::Stone, 20);
		StoneBlock.OutputItem = EItemType::StoneBlock;
		StoneBlock.OutputAmount = 1;
		Recipes.Add(StoneBlock);

		FRecipe Pickaxe;
		Pickaxe.DisplayName = TEXT("Pickaxe");
		Pickaxe.Cost.Add(EItemType::Wood, 20);
		Pickaxe.Cost.Add(EItemType::Stone, 30);
		Pickaxe.OutputItem = EItemType::Pickaxe;
		Pickaxe.OutputAmount = 1;
		Recipes.Add(Pickaxe);
	}
}

UInventoryComponent* UCraftingComponent::GetInventory() const
{
	if (const AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UInventoryComponent>();
	}
	return nullptr;
}

bool UCraftingComponent::CanCraft(int32 RecipeIndex) const
{
	UInventoryComponent* Inventory = GetInventory();
	if (!Inventory || !Recipes.IsValidIndex(RecipeIndex))
	{
		return false;
	}

	const FRecipe& Recipe = Recipes[RecipeIndex];
	for (const TPair<EItemType, int32>& Cost : Recipe.Cost)
	{
		if (Inventory->GetItemCount(Cost.Key) < Cost.Value)
		{
			return false;
		}
	}
	return true;
}

bool UCraftingComponent::Craft(int32 RecipeIndex)
{
	if (!CanCraft(RecipeIndex))
	{
		return false;
	}

	UInventoryComponent* Inventory = GetInventory();
	const FRecipe& Recipe = Recipes[RecipeIndex];

	for (const TPair<EItemType, int32>& Cost : Recipe.Cost)
	{
		Inventory->RemoveItem(Cost.Key, Cost.Value);
	}

	Inventory->AddItem(Recipe.OutputItem, Recipe.OutputAmount);
	OnCrafted.Broadcast(Recipe.OutputItem, Recipe.OutputAmount);
	return true;
}

bool UCraftingComponent::CraftFirstAvailable()
{
	for (int32 Index = 0; Index < Recipes.Num(); ++Index)
	{
		if (Craft(Index))
		{
			return true;
		}
	}
	return false;
}
