#include "RustFallHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "CanvasItem.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "RustFallCharacter.h"
#include "NeedsComponent.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "HotbarComponent.h"
#include "CraftingComponent.h"
#include "RustFallTypes.h"

static FString ItemShortName(EItemType Item)
{
	switch (Item)
	{
	case EItemType::Wood: return TEXT("Wood");
	case EItemType::Stone: return TEXT("Stone");
	case EItemType::Planks: return TEXT("Planks");
	case EItemType::StoneBlock: return TEXT("Block");
	case EItemType::Pickaxe: return TEXT("Pickaxe");
	default: return FString();
	}
}

static const TCHAR* GIconExplosive[12] = {
	TEXT(".....XX....."),
	TEXT("..X..XX..X.."),
	TEXT("...X.XX.X..."),
	TEXT(".X..XXXX..X."),
	TEXT("..XXXXXXXX.."),
	TEXT(".XXXXXXXXXX."),
	TEXT(".XXXXXXXXXX."),
	TEXT("..XXXXXXXX.."),
	TEXT(".X..XXXX..X."),
	TEXT("...X.XX.X..."),
	TEXT("..X..XX..X.."),
	TEXT(".....XX....."),
};

static const TCHAR* GIconCold[12] = {
	TEXT(".....XX....."),
	TEXT(".....XX....."),
	TEXT("..X..XX..X.."),
	TEXT("...X.XX.X..."),
	TEXT("....XXXX...."),
	TEXT("XXX.XXXX.XXX"),
	TEXT("XXX.XXXX.XXX"),
	TEXT("....XXXX...."),
	TEXT("...X.XX.X..."),
	TEXT("..X..XX..X.."),
	TEXT(".....XX....."),
	TEXT(".....XX....."),
};

static const TCHAR* GIconRadiation[12] = {
	TEXT(".....XX....."),
	TEXT("....XXXX...."),
	TEXT("...XX..XX..."),
	TEXT("..XX....XX.."),
	TEXT(".XX..XX..XX."),
	TEXT("XX..XXXX..XX"),
	TEXT("XX.XXXXXX.XX"),
	TEXT("XX..XXXX..XX"),
	TEXT(".XX..XX..XX."),
	TEXT("..XX....XX.."),
	TEXT("...XX..XX..."),
	TEXT("....XXXX...."),
};

static const TCHAR* GIconBite[12] = {
	TEXT("............"),
	TEXT("............"),
	TEXT("XXX.XXX.XXX."),
	TEXT(".X...X...X.."),
	TEXT("............"),
	TEXT(".X...X...X.."),
	TEXT("XXX.XXX.XXX."),
	TEXT("............"),
	TEXT("............"),
	TEXT("............"),
	TEXT("............"),
	TEXT("............"),
};

void ARustFallHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	ARustFallCharacter* Character = Cast<ARustFallCharacter>(GetOwningPawn());
	if (!Character)
	{
		return;
	}

	UHealthComponent* Health = Character->GetHealthComponent();
	UNeedsComponent* Needs = Character->GetNeedsComponent();

	const bool bInventoryOpen = Character->IsInventoryOpen();
	const bool bCraftingOpen = Character->IsCraftingOpen();
	const bool bUIOpen = bInventoryOpen || bCraftingOpen;

	if (bUIOpen)
	{
		DrawBlurBackdrop(Character);
	}

	const float BarW = 190.0f;
	const float BarH = 18.0f;
	const float Gap = 6.0f;
	const float Margin = 28.0f;

	const float BarX = Canvas->SizeX - Margin - BarW;

	const float HungerY = Canvas->SizeY - Margin - BarH;
	const float WaterY = HungerY - Gap - BarH;
	const float HealthY = WaterY - Gap - BarH;

	const bool bDead = Health && Health->IsDead();

	if (!bUIOpen && !bDead)
	{
		if (Health)
		{
			DrawBar(BarX, HealthY, BarW, BarH, Health->Health, Health->MaxHealth, FLinearColor(0.85f, 0.15f, 0.15f),
				FString::Printf(TEXT("Health   %d"), FMath::RoundToInt(Health->Health)));
		}

		if (Needs)
		{
			DrawBar(BarX, WaterY, BarW, BarH, Needs->Thirst, 100.0f, FLinearColor(0.15f, 0.55f, 0.90f),
				FString::Printf(TEXT("Water    %d"), FMath::RoundToInt(Needs->Thirst)));

			DrawBar(BarX, HungerY, BarW, BarH, Needs->Hunger, 100.0f, FLinearColor(0.85f, 0.55f, 0.15f),
				FString::Printf(TEXT("Hunger   %d"), FMath::RoundToInt(Needs->Hunger)));
		}
	}

	if (bDead)
	{
		const FString DeadText = TEXT("YOU DIED");
		const float CenterX = Canvas->SizeX * 0.40f;
		const float CenterY = Canvas->SizeY * 0.45f;

		FCanvasTextItem TextItem(FVector2D(CenterX, CenterY), FText::FromString(DeadText), GEngine->GetLargeFont(), FLinearColor(0.90f, 0.10f, 0.10f));
		TextItem.EnableShadow(FLinearColor::Black);
		TextItem.Scale = FVector2D(2.5f, 2.5f);
		Canvas->DrawItem(TextItem);
	}
	else if (!bUIOpen)
	{
		const float CrossX = Canvas->SizeX * 0.5f;
		const float CrossY = Canvas->SizeY * 0.5f;
		DrawRect(FLinearColor(1.0f, 1.0f, 1.0f, 0.85f), CrossX - 2.0f, CrossY - 2.0f, 4.0f, 4.0f);
	}

	DrawHotbar(Character);

	if (bInventoryOpen)
	{
		DrawInventory(Character);
	}

	if (bCraftingOpen)
	{
		DrawCrafting(Character);
	}
}

void ARustFallHUD::DrawBlurBackdrop(ARustFallCharacter* Character)
{
	if (!Canvas)
	{
		return;
	}

	FTexture* Texture = nullptr;
	if (Character)
	{
		UTextureRenderTarget2D* RenderTarget = Character->GetBlurRenderTarget();
		if (RenderTarget)
		{
			Texture = RenderTarget->GetResource();
		}
	}

	if (Texture)
	{
		FCanvasTileItem TileItem(FVector2D(0.0f, 0.0f), Texture, FVector2D((float)Canvas->SizeX, (float)Canvas->SizeY), FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Opaque;
		Canvas->DrawItem(TileItem);
	}

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.45f), 0.0f, 0.0f, (float)Canvas->SizeX, (float)Canvas->SizeY);
}

void ARustFallHUD::DrawInventory(ARustFallCharacter* Character)
{
	UInventoryComponent* Inventory = Character->GetInventoryComponent();

	TArray<TPair<EItemType, int32>> ItemList;
	if (Inventory)
	{
		for (const TPair<EItemType, int32>& Entry : Inventory->Items)
		{
			ItemList.Add(Entry);
		}
	}

	const float Cell = 56.0f;
	const float Gap = 6.0f;
	const float PreviewW = 240.0f;
	const float PreviewH = 420.0f;

	const int32 Columns = 6;
	const int32 Rows = 4;
	const float GridW = Columns * Cell + (Columns - 1) * Gap;
	const float GridH = Rows * Cell + (Rows - 1) * Gap;

	const float HotbarTop = Canvas->SizeY - 64.0f - 24.0f;
	const float GridTop = HotbarTop - GridH - 12.0f;
	const float CenterX = (Canvas->SizeX - GridW) * 0.5f;

	const float ClothingRowW = 7 * Cell + 6 * Gap;
	const float ClothingX = CenterX - 12.0f - ClothingRowW;

	const float ClothY = GridTop + (Rows - 1) * (Cell + Gap);
	const float BackpackX = ClothingX;
	const float BackpackY = ClothY - Cell - 12.0f;

	const float PreviewX = (ClothingX + Cell + CenterX) * 0.5f - PreviewW * 0.5f;
	const float PreviewY = BackpackY + Cell - PreviewH;

	if (UMaterialInstanceDynamic* PreviewMat = Character->GetPreviewMaterial())
	{
		DrawMaterial(PreviewMat, PreviewX, PreviewY, PreviewW, PreviewH, 0.0f, 0.0f, 1.0f, 1.0f);
	}
	else
	{
		const float CenterPX = PreviewX + PreviewW * 0.5f;
		const FLinearColor FigureColor(0.55f, 0.55f, 0.60f, 0.85f);
		DrawRect(FigureColor, CenterPX - 20.0f, PreviewY + 50.0f, 40.0f, 40.0f);
		DrawRect(FigureColor, CenterPX - 33.0f, PreviewY + 106.0f, 66.0f, 130.0f);
		DrawRect(FigureColor, CenterPX - 53.0f, PreviewY + 110.0f, 20.0f, 104.0f);
		DrawRect(FigureColor, CenterPX + 33.0f, PreviewY + 110.0f, 20.0f, 104.0f);
		DrawRect(FigureColor, CenterPX - 30.0f, PreviewY + 238.0f, 24.0f, 150.0f);
		DrawRect(FigureColor, CenterPX + 6.0f, PreviewY + 238.0f, 24.0f, 150.0f);
	}

	DrawPanelBorder(PreviewX, PreviewY, PreviewW, PreviewH);

	const float StatsX = PreviewX + PreviewW + 14.0f;
	const TCHAR* StatLabels[4] = { TEXT("Explosive"), TEXT("Cold"), TEXT("Radiation"), TEXT("Bite") };
	const FLinearColor StatColors[4] = { FLinearColor(1.0f, 0.5f, 0.1f), FLinearColor(0.5f, 0.8f, 1.0f), FLinearColor(0.6f, 0.9f, 0.2f), FLinearColor(0.9f, 0.3f, 0.3f) };
	const TCHAR* const* StatIcons[4] = { GIconExplosive, GIconCold, GIconRadiation, GIconBite };
	float StatsY = PreviewY + 24.0f;
	for (int32 Index = 0; Index < 4; ++Index)
	{
		DrawIconMask(StatIcons[Index], 12, StatsX, StatsY, 20.0f, StatColors[Index]);
		DrawTextAt(StatsX + 26.0f, StatsY + 2.0f, FString::Printf(TEXT("%s: 0%%"), StatLabels[Index]), FLinearColor(0.9f, 0.9f, 0.9f));
		StatsY += 28.0f;
	}

	const TCHAR* ClothNames[7] = { TEXT("Head"), TEXT("Face"), TEXT("Chest"), TEXT("Hands"), TEXT("Legs"), TEXT("Feet"), TEXT("Misc") };

	DrawCell(BackpackX, BackpackY, Cell, TEXT("Bag"), 0);

	for (int32 Index = 0; Index < 7; ++Index)
	{
		const float X = ClothingX + Index * (Cell + Gap);
		DrawCell(X, ClothY, Cell, ClothNames[Index], 0);
	}

	DrawTextAt(CenterX, GridTop - 22.0f, TEXT("INVENTORY"), FLinearColor(0.95f, 0.90f, 0.60f));
	for (int32 Index = 0; Index < Columns * Rows; ++Index)
	{
		const int32 Col = Index % Columns;
		const int32 Row = Index / Columns;
		const float X = CenterX + Col * (Cell + Gap);
		const float Y = GridTop + Row * (Cell + Gap);

		FString Name;
		int32 Amount = 0;
		if (ItemList.IsValidIndex(Index))
		{
			Name = ItemShortName(ItemList[Index].Key);
			Amount = ItemList[Index].Value;
		}
		DrawCell(X, Y, Cell, Name, Amount);
	}

	const float RightMargin = 24.0f;
	const float RightX = Canvas->SizeX - RightMargin - GridW;
	const float RightStackH = GridH + 16.0f + 2.0f * (Cell + Gap) + 20.0f + 18.0f + Cell;
	const float RightTop = HotbarTop - RightStackH;

	DrawTextAt(RightX, RightTop - 22.0f, TEXT("LOOT"), FLinearColor(0.95f, 0.60f, 0.60f));
	for (int32 Index = 0; Index < Columns * Rows; ++Index)
	{
		const int32 Col = Index % Columns;
		const int32 Row = Index / Columns;
		const float X = RightX + Col * (Cell + Gap);
		const float Y = RightTop + Row * (Cell + Gap);
		DrawCell(X, Y, Cell, FString(), 0);
	}

	const float LootClothY = RightTop + GridH + 16.0f;
	for (int32 Index = 0; Index < 7; ++Index)
	{
		const int32 Col = Index % 4;
		const int32 Row = Index / 4;
		const float X = RightX + Col * (Cell + Gap);
		const float Y = LootClothY + Row * (Cell + Gap);
		DrawCell(X, Y, Cell, ClothNames[Index], 0);
	}

	const float BeltY = LootClothY + 2.0f * (Cell + Gap) + 20.0f;
	DrawTextAt(RightX, BeltY - 18.0f, TEXT("BELT"), FLinearColor(0.85f, 0.85f, 0.85f));
	for (int32 Index = 0; Index < 6; ++Index)
	{
		const float X = RightX + Index * (Cell + Gap);
		DrawCell(X, BeltY, Cell, FString(), 0);
	}
}

void ARustFallHUD::DrawCell(float X, float Y, float Size, const FString& Name, int32 Amount)
{
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.50f), X, Y, Size, Size);

	const FLinearColor BorderColor(1.0f, 1.0f, 1.0f, 0.22f);
	DrawRect(BorderColor, X, Y, Size, 1.0f);
	DrawRect(BorderColor, X, Y + Size - 1.0f, Size, 1.0f);
	DrawRect(BorderColor, X, Y, 1.0f, Size);
	DrawRect(BorderColor, X + Size - 1.0f, Y, 1.0f, Size);

	if (!Name.IsEmpty())
	{
		DrawTextAt(X + 4.0f, Y + Size * 0.5f - 8.0f, Name, FLinearColor::White);
	}

	if (Amount > 0)
	{
		DrawTextAt(X + Size - 20.0f, Y + Size - 16.0f, FString::FromInt(Amount), FLinearColor(0.95f, 0.90f, 0.30f));
	}
}

void ARustFallHUD::DrawTextAt(float X, float Y, const FString& Text, const FLinearColor& Color)
{
	FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), GEngine->GetSmallFont(), Color);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);
}

void ARustFallHUD::DrawPanelBorder(float X, float Y, float Width, float Height)
{
	const FLinearColor BorderColor(1.0f, 1.0f, 1.0f, 0.30f);
	DrawRect(BorderColor, X, Y, Width, 1.0f);
	DrawRect(BorderColor, X, Y + Height - 1.0f, Width, 1.0f);
	DrawRect(BorderColor, X, Y, 1.0f, Height);
	DrawRect(BorderColor, X + Width - 1.0f, Y, 1.0f, Height);
}

void ARustFallHUD::DrawIconMask(const TCHAR* const* Rows, int32 Size, float X, float Y, float BoxSize, const FLinearColor& Color)
{
	const float Pixel = BoxSize / (float)Size;
	for (int32 Row = 0; Row < Size; ++Row)
	{
		const TCHAR* Line = Rows[Row];
		for (int32 Col = 0; Col < Size; ++Col)
		{
			if (Line[Col] == TEXT('X'))
			{
				DrawRect(Color, X + Col * Pixel, Y + Row * Pixel, Pixel + 0.5f, Pixel + 0.5f);
			}
		}
	}
}

void ARustFallHUD::DrawCrafting(ARustFallCharacter* Character)
{
	UCraftingComponent* Crafting = Character->GetCraftingComponent();
	if (!Crafting)
	{
		return;
	}

	const TArray<FRecipe>& Recipes = Crafting->Recipes;
	const float PanelW = 420.0f;
	const float RowH = 52.0f;
	const float PanelX = 80.0f;
	const float PanelY = 140.0f;

	DrawPanelBorder(PanelX - 8.0f, PanelY - 32.0f, PanelW + 16.0f, Recipes.Num() * RowH + 44.0f);
	DrawTextAt(PanelX, PanelY - 26.0f, TEXT("CRAFTING"), FLinearColor(0.95f, 0.90f, 0.60f));

	for (int32 Index = 0; Index < Recipes.Num(); ++Index)
	{
		const FRecipe& Recipe = Recipes[Index];
		const bool bCraftable = Crafting->CanCraft(Index);
		const bool bHovered = (Index == HoveredRecipeIndex);

		const float RowY = PanelY + Index * RowH;
		const float RowBoxH = RowH - 6.0f;

		FLinearColor RowColor(0.05f, 0.05f, 0.05f, 0.55f);
		if (bCraftable)
		{
			RowColor = bHovered ? FLinearColor(0.25f, 0.22f, 0.10f, 0.90f) : FLinearColor(0.12f, 0.12f, 0.10f, 0.75f);
		}

		DrawRect(RowColor, PanelX, RowY, PanelW, RowBoxH);
		DrawPanelBorder(PanelX, RowY, PanelW, RowBoxH);

		const FLinearColor NameColor = bCraftable ? FLinearColor::White : FLinearColor(0.55f, 0.55f, 0.55f);
		DrawTextAt(PanelX + 10.0f, RowY + 6.0f, Recipe.DisplayName.ToString(), NameColor);

		FString CostText;
		for (const TPair<EItemType, int32>& Cost : Recipe.Cost)
		{
			if (!CostText.IsEmpty())
			{
				CostText += TEXT(", ");
			}
			CostText += FString::Printf(TEXT("%d %s"), Cost.Value, *ItemShortName(Cost.Key));
		}
		DrawTextAt(PanelX + 10.0f, RowY + 26.0f, CostText, FLinearColor(0.75f, 0.75f, 0.75f));

		const FString OutputText = FString::Printf(TEXT("-> %d %s"), Recipe.OutputAmount, *ItemShortName(Recipe.OutputItem));
		DrawTextAt(PanelX + PanelW - 130.0f, RowY + 6.0f, OutputText, bCraftable ? FLinearColor(0.60f, 1.0f, 0.60f) : FLinearColor(0.50f, 0.50f, 0.50f));

		const FName BoxName(*FString::Printf(TEXT("Recipe_%d"), Index));
		AddHitBox(FVector2D(PanelX, RowY), FVector2D(PanelW, RowBoxH), BoxName, true, 0);
	}
}

void ARustFallHUD::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);

	const FString Name = BoxName.ToString();
	if (!Name.StartsWith(TEXT("Recipe_")))
	{
		return;
	}

	const int32 Index = FCString::Atoi(*Name.RightChop(7));

	ARustFallCharacter* Character = Cast<ARustFallCharacter>(GetOwningPawn());
	if (Character && Character->GetCraftingComponent())
	{
		Character->GetCraftingComponent()->Craft(Index);
	}
}

void ARustFallHUD::NotifyHitBoxBeginCursorOver(FName BoxName)
{
	Super::NotifyHitBoxBeginCursorOver(BoxName);

	const FString Name = BoxName.ToString();
	if (Name.StartsWith(TEXT("Recipe_")))
	{
		HoveredRecipeIndex = FCString::Atoi(*Name.RightChop(7));
	}
}

void ARustFallHUD::NotifyHitBoxEndCursorOver(FName BoxName)
{
	Super::NotifyHitBoxEndCursorOver(BoxName);

	const FString Name = BoxName.ToString();
	if (Name.StartsWith(TEXT("Recipe_")))
	{
		const int32 Index = FCString::Atoi(*Name.RightChop(7));
		if (HoveredRecipeIndex == Index)
		{
			HoveredRecipeIndex = -1;
		}
	}
}

void ARustFallHUD::DrawHotbar(ARustFallCharacter* Character)
{
	UHotbarComponent* Hotbar = Character->GetHotbarComponent();
	UInventoryComponent* Inventory = Character->GetInventoryComponent();
	if (!Hotbar)
	{
		return;
	}

	const float SlotSize = 64.0f;
	const float SlotGap = 8.0f;
	const int32 Count = UHotbarComponent::SlotCount;
	const float TotalWidth = Count * SlotSize + (Count - 1) * SlotGap;
	const float StartX = (Canvas->SizeX - TotalWidth) * 0.5f;
	const float SlotY = Canvas->SizeY - SlotSize - 24.0f;

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float X = StartX + Index * (SlotSize + SlotGap);
		const bool bSelected = (Index == Hotbar->SelectedSlot);

		DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, bSelected ? 0.70f : 0.45f), X, SlotY, SlotSize, SlotSize);

		const FLinearColor Border = bSelected ? FLinearColor(1.0f, 0.85f, 0.20f) : FLinearColor(1.0f, 1.0f, 1.0f, 0.45f);
		DrawRect(Border, X, SlotY, SlotSize, 2.0f);
		DrawRect(Border, X, SlotY + SlotSize - 2.0f, SlotSize, 2.0f);
		DrawRect(Border, X, SlotY, 2.0f, SlotSize);
		DrawRect(Border, X + SlotSize - 2.0f, SlotY, 2.0f, SlotSize);

		FCanvasTextItem NumberItem(FVector2D(X + 5.0f, SlotY + 3.0f), FText::FromString(FString::FromInt(Index + 1)), GEngine->GetSmallFont(), FLinearColor(0.8f, 0.8f, 0.8f));
		NumberItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(NumberItem);

		const EItemType Item = Hotbar->GetSlotItem(Index);
		const FString Name = ItemShortName(Item);
		if (!Name.IsEmpty())
		{
			FCanvasTextItem NameItem(FVector2D(X + 6.0f, SlotY + SlotSize * 0.5f - 8.0f), FText::FromString(Name), GEngine->GetSmallFont(), FLinearColor::White);
			NameItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(NameItem);

			const int32 Amount = Inventory ? Inventory->GetItemCount(Item) : 0;
			FCanvasTextItem CountItem(FVector2D(X + SlotSize - 22.0f, SlotY + SlotSize - 18.0f), FText::FromString(FString::FromInt(Amount)), GEngine->GetSmallFont(), FLinearColor(0.95f, 0.90f, 0.30f));
			CountItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(CountItem);
		}
	}
}

void ARustFallHUD::DrawBar(float X, float Y, float Width, float Height, float Value, float Max, const FLinearColor& BarColor, const FString& Label)
{
	const float Percent = FMath::Clamp(Max > 0.0f ? Value / Max : 0.0f, 0.0f, 1.0f);

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f), X, Y, Width, Height);
	DrawRect(BarColor, X, Y, Width * Percent, Height);
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.90f), X, Y, Width, 2.0f);
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.90f), X, Y + Height - 2.0f, Width, 2.0f);

	FCanvasTextItem TextItem(FVector2D(X + 8.0f, Y + 1.0f), FText::FromString(Label), GEngine->GetSmallFont(), FLinearColor::White);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);
}
