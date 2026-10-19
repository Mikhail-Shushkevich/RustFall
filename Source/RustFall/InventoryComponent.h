#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RustFallTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (RustFall), meta = (BlueprintSpawnableComponent))
class RUSTFALL_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TMap<EItemType, int32> Items;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(EItemType Type, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(EItemType Type, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(EItemType Type) const;
};
