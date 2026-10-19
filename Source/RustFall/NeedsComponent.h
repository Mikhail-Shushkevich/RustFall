#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NeedsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNeedChanged, FName, NeedName, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNeedDepleted, FName, NeedName);

UCLASS(ClassGroup = (RustFall), meta = (BlueprintSpawnableComponent))
class RUSTFALL_API UNeedsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNeedsComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Needs|Events")
	FOnNeedChanged OnNeedChanged;

	UPROPERTY(BlueprintAssignable, Category = "Needs|Events")
	FOnNeedDepleted OnNeedDepleted;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs|State", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs|State", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs|State", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs|State", meta = (ToolTip = "Internal body temperature in Celsius. 37 is healthy."))
	float Temperature = 37.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Rates", meta = (ToolTip = "Hunger lost per second while idle."))
	float HungerDrainPerSecond = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Rates", meta = (ToolTip = "Thirst lost per second while idle."))
	float ThirstDrainPerSecond = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Rates", meta = (ToolTip = "Stamina lost per second while sprinting."))
	float StaminaDrainPerSecond = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Rates", meta = (ToolTip = "Stamina regained per second while not sprinting."))
	float StaminaRegenPerSecond = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Temperature", meta = (ToolTip = "Ambient temperature in Celsius. Change this from weather/zone systems."))
	float AmbientTemperature = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Temperature", meta = (ToolTip = "Body temperature the character returns to in a comfortable climate."))
	float NominalBodyTemperature = 37.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Temperature", meta = (ToolTip = "Ambient temperatures in this range do not affect body temperature."))
	float NeutralTemperatureMin = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Temperature", meta = (ToolTip = "Ambient temperatures in this range do not affect body temperature."))
	float NeutralTemperatureMax = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Temperature", meta = (ToolTip = "How fast body temperature moves toward ambient (degrees per second)."))
	float TemperatureAdjustSpeed = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Temperature")
	float ComfortTemperatureMin = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Temperature")
	float ComfortTemperatureMax = 39.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Needs|State", meta = (ToolTip = "Set by the character when sprinting."))
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadWrite, Category = "Needs|State", meta = (ToolTip = "True while the sprint key is held. While true, stamina does not regenerate."))
	bool bSprintInputHeld = false;

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void SetSprintInputHeld(bool bHeld);

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void AddHunger(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void AddThirst(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void AddStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void Eat(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void Drink(float Amount);

	UFUNCTION(BlueprintPure, Category = "Needs")
	bool IsStarving() const { return Hunger <= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Needs")
	bool IsDehydrated() const { return Thirst <= 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Needs")
	bool IsFreezing() const { return Temperature < ComfortTemperatureMin; }

	UFUNCTION(BlueprintPure, Category = "Needs")
	bool IsOverheating() const { return Temperature > ComfortTemperatureMax; }

protected:
	void BroadcastNeed(FName NeedName, float Value, float Max);

private:
	bool bHungerDepleted = false;
	bool bThirstDepleted = false;
};
