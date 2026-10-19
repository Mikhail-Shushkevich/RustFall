#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RustFallTypes.h"
#include "HotbarComponent.generated.h"

UCLASS(ClassGroup = (RustFall), meta = (BlueprintSpawnableComponent))
class RUSTFALL_API UHotbarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHotbarComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hotbar")
	TArray<EItemType> Slots;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	int32 SelectedSlot = 0;

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void SelectSlot(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Hotbar")
	void CycleSlot(int32 Direction);

	UFUNCTION(BlueprintPure, Category = "Hotbar")
	EItemType GetSlotItem(int32 Index) const;

	static constexpr int32 SlotCount = 6;

protected:
	virtual void BeginPlay() override;
};
