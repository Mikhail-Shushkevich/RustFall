#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ToolActor.generated.h"

class UStaticMeshComponent;

/** Base class for a held tool viewmodel. Create Blueprint children (BP_Pickaxe, BP_Axe, ...)
 *  and set the mesh on the Mesh component. The character spawns it and attaches it to the hand. */
UCLASS()
class RUSTFALL_API AToolActor : public AActor
{
	GENERATED_BODY()

public:
	AToolActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	UStaticMeshComponent* Mesh;
};
