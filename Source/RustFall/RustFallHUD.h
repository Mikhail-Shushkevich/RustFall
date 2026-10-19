#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RustFallHUD.generated.h"

class ARustFallCharacter;

UCLASS()
class RUSTFALL_API ARustFallHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	virtual void NotifyHitBoxClick(FName BoxName) override;

	virtual void NotifyHitBoxBeginCursorOver(FName BoxName) override;

	virtual void NotifyHitBoxEndCursorOver(FName BoxName) override;

protected:
	void DrawBar(float X, float Y, float Width, float Height, float Value, float Max, const FLinearColor& BarColor, const FString& Label);

	void DrawHotbar(ARustFallCharacter* Character);

	void DrawInventory(ARustFallCharacter* Character);

	void DrawCrafting(ARustFallCharacter* Character);

	void DrawBlurBackdrop(ARustFallCharacter* Character);

	void DrawCell(float X, float Y, float Size, const FString& Name, int32 Amount);

	void DrawTextAt(float X, float Y, const FString& Text, const FLinearColor& Color);

	void DrawPanelBorder(float X, float Y, float Width, float Height);

	void DrawIconMask(const TCHAR* const* Rows, int32 Size, float X, float Y, float BoxSize, const FLinearColor& Color);

private:
	int32 HoveredRecipeIndex = -1;
};
