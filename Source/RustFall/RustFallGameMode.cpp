// Copyright Epic Games, Inc. All Rights Reserved.

#include "RustFallGameMode.h"
#include "RustFallCharacter.h"
#include "RustFallHUD.h"
#include "ResourceNode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ARustFallGameMode::ARustFallGameMode()
{
	HUDClass = ARustFallHUD::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ARustFallGameMode::StartPlay()
{
	Super::StartPlay();

	if (bSpawnResourceNodes)
	{
		SpawnResourceNodes();
	}
}

void ARustFallGameMode::SpawnResourceNodes()
{
	TArray<EItemType> Types;
	Types.Reserve(FMath::Max(0, WoodNodeCount) + FMath::Max(0, StoneNodeCount));

	for (int32 Index = 0; Index < WoodNodeCount; ++Index)
	{
		Types.Add(EItemType::Wood);
	}

	for (int32 Index = 0; Index < StoneNodeCount; ++Index)
	{
		Types.Add(EItemType::Stone);
	}

	for (int32 Index = Types.Num() - 1; Index > 0; --Index)
	{
		Types.Swap(Index, FMath::RandRange(0, Index));
	}

	FVector Center = FVector::ZeroVector;
	bool bHasStart = false;

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			Center = It->GetActorLocation();
			bHasStart = true;
			break;
		}
	}

	for (const EItemType Type : Types)
	{
		SpawnNodeAt(Center, Type);
	}
}

void ARustFallGameMode::SpawnNodeAt(const FVector& Center, EItemType Type)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 Attempt = 0; Attempt < 12; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Radius = FMath::FRandRange(FMath::Min(SpawnRadiusMin, SpawnRadiusMax), FMath::Max(SpawnRadiusMin, SpawnRadiusMax));
		const FVector XY = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

		const FVector Start = XY + FVector(0.0f, 0.0f, 1000.0f);
		const FVector End = XY - FVector(0.0f, 0.0f, 1000.0f);

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(SpawnResourceNode), false);

		if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			continue;
		}

		if (Hit.ImpactNormal.Z < 0.7f)
		{
			continue;
		}

		if (Hit.GetActor() && (Hit.GetActor()->IsA<AResourceNode>() || Hit.GetActor()->IsA<APawn>()))
		{
			continue;
		}

		const FVector Location = Hit.ImpactPoint + FVector(0.0f, 0.0f, 50.0f);

		const TSubclassOf<AResourceNode> RequestedClass = (Type == EItemType::Stone) ? StoneNodeClass : WoodNodeClass;
		UClass* NodeClass = RequestedClass ? RequestedClass.Get() : AResourceNode::StaticClass();

		AResourceNode* Node = World->SpawnActorDeferred<AResourceNode>(NodeClass, FTransform(Location));
		if (Node)
		{
			Node->ResourceType = Type;
			Node->MaxAmount = NodeAmount;
			Node->Amount = NodeAmount;

			if (bOverrideNodeColorWithConfig && NodeClass == AResourceNode::StaticClass())
			{
				Node->bColorByType = false;
				Node->NodeColor = (Type == EItemType::Stone) ? StoneColor : WoodColor;
			}

			Node->FinishSpawning(FTransform(Location));
		}
		return;
	}
}
