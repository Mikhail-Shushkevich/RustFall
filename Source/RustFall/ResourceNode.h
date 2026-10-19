#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RustFallTypes.h"
#include "ResourceNode.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceNodeDepleted, AResourceNode*, Node);

UCLASS()
class RUSTFALL_API AResourceNode : public AActor
{
	GENERATED_BODY()

public:
	AResourceNode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EItemType ResourceType = EItemType::Wood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "1"))
	int32 MaxAmount = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	bool bColorByType = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FLinearColor NodeColor = FLinearColor::White;

	/** Mesh used for Wood nodes (falls back to an engine cylinder) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Visual")
	TObjectPtr<UStaticMesh> WoodMesh;

	/** Mesh used for Stone nodes (falls back to an engine cube) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Visual")
	TObjectPtr<UStaticMesh> StoneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource", meta = (ClampMin = "0"))
	float RespawnDelay = 30.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
	int32 Amount = 50;

	UPROPERTY(BlueprintAssignable, Category = "Resource|Events")
	FOnResourceNodeDepleted OnDepleted;

	UFUNCTION(BlueprintCallable, Category = "Resource")
	int32 Gather(int32 Requested);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void Respawn();

	UFUNCTION(BlueprintPure, Category = "Resource")
	bool IsDepleted() const { return Amount <= 0; }

protected:
	virtual void BeginPlay() override;

private:
	void ApplyMesh();

	void ApplyColor();

	FTimerHandle RespawnTimer;
};
