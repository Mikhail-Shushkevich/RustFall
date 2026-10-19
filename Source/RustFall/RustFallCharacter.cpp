// Copyright Epic Games, Inc. All Rights Reserved.

#include "RustFallCharacter.h"
#include "NeedsComponent.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "CraftingComponent.h"
#include "HotbarComponent.h"
#include "ResourceNode.h"
#include "CharacterPreview.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ARustFallCharacter

ARustFallCharacter::ARustFallCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// First-person: the character body turns with the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (first-person: zero arm length, positioned at eye height)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// First-person: show the owner's own body so the arm/hand is visible
	GetMesh()->SetOwnerNoSee(false);

	// First-person tool viewmodel (visible only to the owner), attached to the right hand bone
	ToolRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ToolRoot"));
	ToolRoot->SetupAttachment(GetMesh(), TEXT("hand_r"));
	ToolRoot->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	ToolRoot->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	ToolHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolHandle"));
	ToolHandle->SetupAttachment(ToolRoot);
	ToolHandle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ToolHandle->SetCastShadow(false);
	ToolHandle->SetOnlyOwnerSee(true);
	ToolHandle->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	ToolHandle->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ToolHandle->SetRelativeScale3D(FVector(0.03f, 0.03f, 0.5f));

	ToolHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolHead"));
	ToolHead->SetupAttachment(ToolRoot);
	ToolHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ToolHead->SetCastShadow(false);
	ToolHead->SetOnlyOwnerSee(true);
	ToolHead->SetRelativeLocation(FVector(0.0f, 0.0f, 22.0f));
	ToolHead->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	ToolHead->SetRelativeScale3D(FVector(0.2f, 0.04f, 0.04f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ToolHandleMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (ToolHandleMesh.Succeeded())
	{
		ToolHandle->SetStaticMesh(ToolHandleMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ToolHeadMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (ToolHeadMesh.Succeeded())
	{
		ToolHead->SetStaticMesh(ToolHeadMesh.Object);
	}

	// Low-resolution capture of the player view, used to blur the inventory background
	BlurCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("BlurCapture"));
	BlurCapture->SetupAttachment(FollowCamera);
	BlurCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	BlurCapture->FOVAngle = 90.0f;
	BlurCapture->bCaptureEveryFrame = false;
	BlurCapture->bCaptureOnMovement = false;
	BlurCapture->HiddenActors.Add(this);

	// Survival needs component (hunger, thirst, stamina, temperature)
	NeedsComponent = CreateDefaultSubobject<UNeedsComponent>(TEXT("NeedsComponent"));

	// Health, damage and death
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// Inventory of gathered resources
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	// Crafting (recipes)
	CraftingComponent = CreateDefaultSubobject<UCraftingComponent>(TEXT("CraftingComponent"));

	// Hotbar (6 quick slots)
	HotbarComponent = CreateDefaultSubobject<UHotbarComponent>(TEXT("HotbarComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ARustFallCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ARustFallCharacter::HandleDeath);
	}

	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		PreviewActor = World->SpawnActor<ACharacterPreview>(ACharacterPreview::StaticClass(), FVector(0.0f, 0.0f, 100000.0f), FRotator::ZeroRotator, SpawnParams);

		if (PreviewActor)
		{
			PreviewActor->SetPreviewMesh(GetMesh() ? GetMesh()->GetSkeletalMeshAsset() : nullptr);
			PreviewActor->FollowPose(GetMesh());
		}
	}

	if (BlurCapture)
	{
		BlurRenderTarget = NewObject<UTextureRenderTarget2D>(this);
		BlurRenderTarget->RenderTargetFormat = RTF_RGBA8;
		BlurRenderTarget->ClearColor = FLinearColor(0.02f, 0.02f, 0.03f, 1.0f);
		BlurRenderTarget->bAutoGenerateMips = false;
		BlurRenderTarget->InitAutoFormat(240, 135);
		BlurCapture->TextureTarget = BlurRenderTarget;
	}

	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
	}

	UpdateToolVisual();
}

UTextureRenderTarget2D* ARustFallCharacter::GetPreviewRenderTarget() const
{
	return PreviewActor ? PreviewActor->GetRenderTarget() : nullptr;
}

UMaterialInstanceDynamic* ARustFallCharacter::GetPreviewMaterial() const
{
	return PreviewActor ? PreviewActor->GetPreviewMaterial() : nullptr;
}

void ARustFallCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsAttacking && HealthComponent && !HealthComponent->IsDead())
	{
		AttackTick(DeltaSeconds);
	}

	if (HotbarComponent && HotbarComponent->SelectedSlot != CachedToolSlot)
	{
		CachedToolSlot = HotbarComponent->SelectedSlot;
		UpdateToolVisual();
	}

	if (!NeedsComponent)
	{
		return;
	}

	if (NeedsComponent->bIsSprinting && NeedsComponent->Stamina <= 0.0f)
	{
		NeedsComponent->bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	if (HealthComponent && !HealthComponent->IsDead())
	{
		float DamageThisFrame = 0.0f;

		if (NeedsComponent->IsStarving() || NeedsComponent->IsDehydrated())
		{
			DamageThisFrame += StarvationDamagePerSecond;
		}

		if (NeedsComponent->IsFreezing() || NeedsComponent->IsOverheating())
		{
			DamageThisFrame += TemperatureDamagePerSecond;
		}

		if (DamageThisFrame > 0.0f)
		{
			HealthComponent->ApplyDamage(DamageThisFrame * DeltaSeconds);
		}
	}
}

void ARustFallCharacter::KillSelf()
{
	if (HealthComponent)
	{
		HealthComponent->Kill();
	}
}

void ARustFallCharacter::HandleDeath()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		DisableInput(PlayerController);
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	StopSprint();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		SkelMesh->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, true);

		if (FollowCamera)
		{
			FollowCamera->bUsePawnControlRotation = false;
			FollowCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			FollowCamera->AttachToComponent(SkelMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("head"));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARustFallCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ARustFallCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARustFallCharacter::OnJumpPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARustFallCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARustFallCharacter::Look);

		// Sprinting (hold)
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ARustFallCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARustFallCharacter::StopSprint);
		}

		// Interacting (hold E)
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ARustFallCharacter::StartInteract);
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ARustFallCharacter::StopInteract);
		}

		// Crafting (press C)
		if (CraftAction)
		{
			EnhancedInputComponent->BindAction(CraftAction, ETriggerEvent::Started, this, &ARustFallCharacter::OnCraftPressed);
		}

		// Hotbar selection (mouse wheel)
		if (HotbarAction)
		{
			EnhancedInputComponent->BindAction(HotbarAction, ETriggerEvent::Triggered, this, &ARustFallCharacter::OnHotbarScroll);
		}

		// Inventory screen (Tab)
		if (InventoryAction)
		{
			EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ARustFallCharacter::OnInventoryToggle);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Attack (hold LMB)
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ARustFallCharacter::OnAttackPressed);
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ARustFallCharacter::OnAttackReleased);
}

void ARustFallCharacter::Move(const FInputActionValue& Value)
{
	if (IsUIOpen())
	{
		return;
	}

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARustFallCharacter::Look(const FInputActionValue& Value)
{
	if (IsUIOpen())
	{
		return;
	}

	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARustFallCharacter::OnJumpPressed()
{
	if (IsUIOpen())
	{
		return;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	Jump();
}

void ARustFallCharacter::StartSprint()
{
	if (IsUIOpen())
	{
		return;
	}

	if (!NeedsComponent)
	{
		return;
	}

	NeedsComponent->SetSprintInputHeld(true);

	if (NeedsComponent->Stamina > 0.0f)
	{
		NeedsComponent->bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void ARustFallCharacter::StopSprint()
{
	if (NeedsComponent)
	{
		NeedsComponent->SetSprintInputHeld(false);
		NeedsComponent->bIsSprinting = false;
	}
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ARustFallCharacter::StartInteract()
{
}

void ARustFallCharacter::StopInteract()
{
}

void ARustFallCharacter::OnAttackPressed()
{
	if (IsUIOpen())
	{
		return;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	bIsAttacking = true;
	AttackAccumulator = AttackInterval;
}

void ARustFallCharacter::OnAttackReleased()
{
	bIsAttacking = false;
}

void ARustFallCharacter::OnCraftPressed()
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	bCraftingOpen = !bCraftingOpen;

	if (bCraftingOpen)
	{
		StopSprint();

		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
	}

	UpdateInputMode();
}

void ARustFallCharacter::UpdateInputMode()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	const bool bUIOpen = IsUIOpen();
	if (bUIOpen)
	{
		bIsAttacking = false;
	}

	PlayerController->bShowMouseCursor = bUIOpen;
	PlayerController->bEnableClickEvents = bUIOpen;
	PlayerController->bEnableMouseOverEvents = bUIOpen;

	if (bUIOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	if (BlurCapture)
	{
		BlurCapture->bCaptureEveryFrame = bUIOpen;
		if (bUIOpen)
		{
			BlurCapture->CaptureScene();
		}
	}
}

void ARustFallCharacter::OnHotbarScroll(const FInputActionValue& Value)
{
	if (IsUIOpen())
	{
		return;
	}

	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	if (!HotbarComponent)
	{
		return;
	}

	const float Axis = Value.Get<float>();
	if (Axis > 0.5f)
	{
		HotbarComponent->CycleSlot(1);
	}
	else if (Axis < -0.5f)
	{
		HotbarComponent->CycleSlot(-1);
	}
}

void ARustFallCharacter::OnInventoryToggle()
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return;
	}

	bInventoryOpen = !bInventoryOpen;

	if (bInventoryOpen)
	{
		StopSprint();

		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
	}

	UpdateInputMode();
}

void ARustFallCharacter::AttackTick(float DeltaSeconds)
{
	if (!FollowCamera || !InventoryComponent)
	{
		return;
	}

	if (AttackAccumulator < AttackInterval)
	{
		AttackAccumulator += DeltaSeconds;
		return;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const float ArmLength = CameraBoom ? CameraBoom->TargetArmLength : 0.0f;
	const FVector End = Start + FollowCamera->GetForwardVector() * (ArmLength + InteractRange);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const FCollisionShape AttackSphere = FCollisionShape::MakeSphere(30.0f);
	if (!GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, AttackSphere, Params))
	{
		return;
	}

	AResourceNode* Node = Cast<AResourceNode>(Hit.GetActor());
	if (!Node)
	{
		return;
	}

	AttackAccumulator = 0.0f;

	int32 Yield = 1;
	if (HotbarComponent && HotbarComponent->GetSlotItem(HotbarComponent->SelectedSlot) == EItemType::Pickaxe)
	{
		Yield = 3;
	}

	const int32 Gathered = Node->Gather(Yield);
	if (Gathered > 0)
	{
		InventoryComponent->AddItem(Node->ResourceType, Gathered);
	}
}

void ARustFallCharacter::UpdateToolVisual()
{
	const bool bHasTool = HotbarComponent && HotbarComponent->GetSlotItem(HotbarComponent->SelectedSlot) == EItemType::Pickaxe;

	if (ToolHandle)
	{
		ToolHandle->SetVisibility(bHasTool);
	}

	if (ToolHead)
	{
		ToolHead->SetVisibility(bHasTool);
	}
}
