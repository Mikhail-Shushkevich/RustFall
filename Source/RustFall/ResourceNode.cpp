#include "ResourceNode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

static FLinearColor ColorForItem(EItemType Type)
{
	switch (Type)
	{
	case EItemType::Wood: return FLinearColor(0.55f, 0.28f, 0.08f);
	case EItemType::Stone: return FLinearColor(0.30f, 0.31f, 0.33f);
	default: return FLinearColor(0.60f, 0.60f, 0.60f);
	}
}

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylFinder.Succeeded())
	{
		WoodMesh = CylFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StoneFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (StoneFinder.Succeeded())
	{
		StoneMesh = StoneFinder.Object;
	}
}

void AResourceNode::BeginPlay()
{
	Super::BeginPlay();

	Amount = MaxAmount;

	if (bColorByType)
	{
		NodeColor = ColorForItem(ResourceType);
	}

	ApplyMesh();
	ApplyColor();
}

void AResourceNode::ApplyMesh()
{
	if (!Mesh)
	{
		return;
	}

	UStaticMesh* Chosen = (ResourceType == EItemType::Stone) ? StoneMesh : WoodMesh;
	if (Chosen)
	{
		Mesh->SetStaticMesh(Chosen);
	}
}

void AResourceNode::ApplyColor()
{
	if (!Mesh)
	{
		return;
	}

	Mesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(NodeColor.R, NodeColor.G, NodeColor.B));
}

int32 AResourceNode::Gather(int32 Requested)
{
	if (Amount <= 0 || Requested <= 0)
	{
		return 0;
	}

	const int32 Gathered = FMath::Min(Amount, Requested);
	Amount -= Gathered;

	if (Amount <= 0)
	{
		OnDepleted.Broadcast(this);
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		if (RespawnDelay > 0.0f)
		{
			GetWorldTimerManager().SetTimer(RespawnTimer, this, &AResourceNode::Respawn, RespawnDelay, false);
		}
	}

	return Gathered;
}

void AResourceNode::Respawn()
{
	Amount = MaxAmount;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}
