#pragma once

#include "CoreMinimal.h"
#include "RustFallTypes.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None UMETA(DisplayName = "None"),
	Wood UMETA(DisplayName = "Wood"),
	Stone UMETA(DisplayName = "Stone"),
	Planks UMETA(DisplayName = "Planks"),
	StoneBlock UMETA(DisplayName = "Stone Block"),
	Pickaxe UMETA(DisplayName = "Pickaxe")
};

USTRUCT(BlueprintType)
struct FRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TMap<EItemType, int32> Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EItemType OutputItem = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 OutputAmount = 1;
};
