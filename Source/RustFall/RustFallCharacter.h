// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "RustFallCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UNeedsComponent;
class UHealthComponent;
class UInventoryComponent;
class UCraftingComponent;
class UHotbarComponent;
class AResourceNode;
class ACharacterPreview;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;
class USceneComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ARustFallCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** First-person tool viewmodel root (visible only to the owner) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tool, meta = (AllowPrivateAccess = "true"))
	USceneComponent* ToolRoot;

	/** First-person tool viewmodel: handle (visible only to the owner) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tool, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ToolHandle;

	/** First-person tool viewmodel: head (visible only to the owner) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tool, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ToolHead;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Sprint Input Action (hold Shift) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Interact Input Action (hold E) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	/** Craft Input Action (press C) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CraftAction;

	/** Hotbar scroll Input Action (mouse wheel) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HotbarAction;

	/** Inventory toggle Input Action (Tab) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InventoryAction;

	/** Survival needs: hunger, thirst, stamina, temperature */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Needs, meta = (AllowPrivateAccess = "true"))
	UNeedsComponent* NeedsComponent;

	/** Health, damage and death */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	/** Inventory (gathered resources) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	/** Crafting (recipes and production) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crafting, meta = (AllowPrivateAccess = "true"))
	UCraftingComponent* CraftingComponent;

	/** Hotbar (6 quick slots) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Hotbar, meta = (AllowPrivateAccess = "true"))
	UHotbarComponent* HotbarComponent;

	/** Actor hosting the rotating 3D character preview */
	UPROPERTY()
	ACharacterPreview* PreviewActor;

	/** Low-resolution capture of the player view, used to blur the inventory background */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* BlurCapture;

	UPROPERTY()
	UTextureRenderTarget2D* BlurRenderTarget;

	/** Distance the character can reach resource nodes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	float InteractRange = 350.0f;

	/** Time between attacks in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	float AttackInterval = 0.5f;

	/** Health lost per second while starving or dehydrated */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	float StarvationDamagePerSecond = 1.0f;

	/** Health lost per second while freezing or overheating */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	float TemperatureDamagePerSecond = 1.0f;

	/** Normal walking speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 250.0f;

	/** Sprinting speed while holding the sprint key */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 650.0f;

public:
	ARustFallCharacter();

	virtual void Tick(float DeltaSeconds) override;

	/** Debug: instantly kill the character (console command "KillSelf") */
	UFUNCTION(Exec)
	void KillSelf();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called when the jump key is pressed */
	void OnJumpPressed();

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called when sprint input starts */
	void StartSprint();

	/** Called when sprint input ends */
	void StopSprint();

	/** Called when interact input starts */
	void StartInteract();

	/** Called when interact input ends */
	void StopInteract();

	/** Called when the craft key is pressed */
	void OnCraftPressed();

	/** Called when the hotbar scroll input is triggered */
	void OnHotbarScroll(const FInputActionValue& Value);

	/** Called when the inventory key is pressed */
	void OnInventoryToggle();

	/** Called when the attack input starts (LMB) */
	void OnAttackPressed();

	/** Called when the attack input ends (LMB) */
	void OnAttackReleased();

	/** Performs attack trace and gathers resources from the targeted node */
	void AttackTick(float DeltaSeconds);

	/** Updates mouse cursor / input mode based on whether any UI screen is open */
	void UpdateInputMode();

	/** Shows/hides the first-person tool viewmodel based on the equipped hotbar item */
	void UpdateToolVisual();

	/** Called when health reaches zero */
	UFUNCTION()
	void HandleDeath();

protected:

	virtual void BeginPlay() override;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns NeedsComponent subobject **/
	FORCEINLINE class UNeedsComponent* GetNeedsComponent() const { return NeedsComponent; }
	/** Returns HealthComponent subobject **/
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	/** Returns InventoryComponent subobject **/
	FORCEINLINE class UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	/** Returns CraftingComponent subobject **/
	FORCEINLINE class UCraftingComponent* GetCraftingComponent() const { return CraftingComponent; }
	/** Returns HotbarComponent subobject **/
	FORCEINLINE class UHotbarComponent* GetHotbarComponent() const { return HotbarComponent; }

	/** Render target holding the 3D character preview (may be null) */
	class UTextureRenderTarget2D* GetPreviewRenderTarget() const;

	/** Material that composites the preview with inverted alpha (may be null) */
	class UMaterialInstanceDynamic* GetPreviewMaterial() const;

	/** Render target holding the blurred background of the player view (may be null) */
	class UTextureRenderTarget2D* GetBlurRenderTarget() const { return BlurRenderTarget; }

	/** True while the inventory screen is open */
	UFUNCTION(BlueprintPure, Category = Inventory)
	bool IsInventoryOpen() const { return bInventoryOpen; }

	/** True while the crafting menu is open */
	UFUNCTION(BlueprintPure, Category = Crafting)
	bool IsCraftingOpen() const { return bCraftingOpen; }

	/** True while any UI screen is open */
	bool IsUIOpen() const { return bInventoryOpen || bCraftingOpen; }

private:
	/** True while attacking */
	bool bIsAttacking = false;

	/** Accumulator driving attack rate */
	float AttackAccumulator = 0.0f;

	/** True while the inventory screen is open */
	bool bInventoryOpen = false;

	/** True while the crafting menu is open */
	bool bCraftingOpen = false;

	/** Last hotbar slot used to refresh the tool viewmodel */
	int32 CachedToolSlot = -1;
};

