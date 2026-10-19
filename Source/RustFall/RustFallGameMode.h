// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RustFallTypes.h"
#include "RustFallGameMode.generated.h"

class AResourceNode;

UCLASS(minimalapi)
class ARustFallGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARustFallGameMode();

	virtual void StartPlay() override;

	/** Scatter wood/stone resource nodes around the level's player start */
	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	bool bSpawnResourceNodes = true;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	int32 WoodNodeCount = 1;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	int32 StoneNodeCount = 1;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	float SpawnRadiusMin = 300.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	float SpawnRadiusMax = 700.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	int32 NodeAmount = 50;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	FLinearColor WoodColor = FLinearColor(0.55f, 0.28f, 0.08f);

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	FLinearColor StoneColor = FLinearColor(0.30f, 0.31f, 0.33f);

	/** Blueprint/class used to spawn wood nodes. If unset, the base AResourceNode is used.
	 *  Assign a Blueprint subclass here to give wood nodes a custom mesh, color and amount. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources|Classes")
	TSubclassOf<AResourceNode> WoodNodeClass;

	/** Blueprint/class used to spawn stone nodes. If unset, the base AResourceNode is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources|Classes")
	TSubclassOf<AResourceNode> StoneNodeClass;

	/** When true, the spawned node's color is taken from WoodColor/StoneColor above.
	 *  When false, each node keeps the color defined on its own class defaults. */
	UPROPERTY(Config, EditAnywhere, Category = "Resources|Classes")
	bool bOverrideNodeColorWithConfig = true;

private:
	void SpawnResourceNodes();
	void SpawnNodeAt(const FVector& Center, EItemType Type);
};
