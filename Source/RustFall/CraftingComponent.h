#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RustFallTypes.h"
#include "CraftingComponent.generated.h"

class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrafted, EItemType, Item, int32, Amount);

UCLASS(ClassGroup = (RustFall), meta = (BlueprintSpawnableComponent))
class RUSTFALL_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	TArray<FRecipe> Recipes;

	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCrafted OnCrafted;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool CanCraft(int32 RecipeIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool Craft(int32 RecipeIndex);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool CraftFirstAvailable();

protected:
	virtual void BeginPlay() override;

	UInventoryComponent* GetInventory() const;
};
