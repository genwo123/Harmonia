#include "Character/HamoniaCharacter.h"
#include "Gameplay/InventoryComponent.h"
#include "Core/PlayerInteractionComponent.h"
#include "Gameplay/HeldItemDisplayComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Save_Instance/Hamoina_GameInstance.h"
#include "Core/DialogueManagerComponent.h"
#include "Character/Unia.h"

AHamoniaCharacter::AHamoniaCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    CameraSpringArm->SetupAttachment(RootComponent);
    CameraSpringArm->TargetArmLength = 0.0f;
    CameraSpringArm->bUsePawnControlRotation = true;
    CameraSpringArm->bEnableCameraLag = false;
    CameraSpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CameraComponent->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false;

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    InteractionComponent = CreateDefaultSubobject<UPlayerInteractionComponent>(TEXT("InteractionComponent"));

    HeldObjectAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HeldObjectAttachPoint"));
    HeldObjectAttachPoint->SetupAttachment(CameraComponent);
    HeldObjectAttachPoint->SetRelativeLocation(FVector(50.0f, 0.0f, -20.0f));

    HeldItemDisplay = CreateDefaultSubobject<UHeldItemDisplayComponent>(TEXT("HeldItemDisplay"));

    HeldItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeldItemMesh"));
    HeldItemMeshComponent->SetupAttachment(CameraComponent);
    HeldItemMeshComponent->SetRelativeLocation(FVector(30.0f, 15.0f, -10.0f));
    HeldItemMeshComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
    HeldItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HeldItemMeshComponent->SetVisibility(false);
    HeldItemMeshComponent->CastShadow = false;

    DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));
}

void AHamoniaCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        GetCharacterMovement()->JumpZVelocity = JumpHeight;
    }

    if (InteractionComponent)
    {
        InteractionComponent->SetupReferences(CameraComponent, InventoryComponent, HeldObjectAttachPoint);
        InteractionComponent->bShowDebugLines = true;
    }

    if (InteractionComponent)
    {
        GetWorld()->GetTimerManager().SetTimer(
            LegacySyncTimer,
            [this]()
            {
                bIsLookingAtInteractable = InteractionComponent->bIsLookingAtInteractable;
                CurrentInteractableActor = InteractionComponent->CurrentInteractableActor;
                CurrentInteractionText = InteractionComponent->CurrentInteractionText;
                CurrentInteractionType = InteractionComponent->CurrentInteractionType;
            },
            0.1f,
            true
        );
    }
}
void AHamoniaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Move);
        }

        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Look);
        }

        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHamoniaCharacter::StartSprint);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHamoniaCharacter::StopSprint);
        }

        if (CrouchAction)
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AHamoniaCharacter::ToggleCrouch);
        }

        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AHamoniaCharacter::Interact);
        }

        if (RotateAction)
        {
            EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &AHamoniaCharacter::RotateObject);
        }

        if (PushAction)
        {
            EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Started, this, &AHamoniaCharacter::PushObject);
        }

        if (DropAction)
        {
            EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &AHamoniaCharacter::DropHeldObject);
        }

        if (InventoryToggleAction)
        {
            EnhancedInputComponent->BindAction(InventoryToggleAction, ETriggerEvent::Started, this, &AHamoniaCharacter::OnInventoryToggle);
        }

        if (InventoryLeftAction)
        {
            EnhancedInputComponent->BindAction(InventoryLeftAction, ETriggerEvent::Started, this, &AHamoniaCharacter::OnInventoryMoveLeft);
        }

        if (InventoryRightAction)
        {
            EnhancedInputComponent->BindAction(InventoryRightAction, ETriggerEvent::Started, this, &AHamoniaCharacter::OnInventoryMoveRight);
        }

        if (InventoryUseAction)
        {
            EnhancedInputComponent->BindAction(InventoryUseAction, ETriggerEvent::Started, this, &AHamoniaCharacter::OnInventoryUse);
        }
    }
}

void AHamoniaCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AHamoniaCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X * LookSensitivity);
        AddControllerPitchInput(LookAxisVector.Y * LookSensitivity);
    }
}

void AHamoniaCharacter::StartSprint()
{
    bIsSprinting = true;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }
}

void AHamoniaCharacter::StopSprint()
{
    bIsSprinting = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void AHamoniaCharacter::ToggleCrouch()
{
    if (bIsCrouched)
    {
        UnCrouch();
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        }
    }
    else
    {
        Crouch();
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
        }
    }
}

void AHamoniaCharacter::Interact()
{
    if (InteractionComponent)
    {
        InteractionComponent->PerformInteraction();
    }
}

void AHamoniaCharacter::RotateObject()
{
    if (InteractionComponent)
    {
        InteractionComponent->RotatePedestal();
    }
}

void AHamoniaCharacter::PushObject()
{
    if (InteractionComponent)
    {
        InteractionComponent->PushPedestal();
    }
}

void AHamoniaCharacter::DropHeldObject()
{
    if (InteractionComponent)
    {
        InteractionComponent->DropHeldObject();
    }
}

void AHamoniaCharacter::OnInventoryToggle()
{
    if (InventoryComponent)
    {
        InventoryComponent->ToggleInventory();
    }
}

void AHamoniaCharacter::OnInventoryMoveLeft()
{
    if (InventoryComponent)
    {
        InventoryComponent->MoveSelection(-1);

        if (HeldItemDisplay)
        {
            UItem* CurrentItem = InventoryComponent->GetSelectedItem();
            if (CurrentItem)
            {
                HeldItemDisplay->UpdateDisplay(CurrentItem, InventoryComponent->bIsInventoryOpen);
            }
        }
    }
}

void AHamoniaCharacter::OnInventoryMoveRight()
{
    if (InventoryComponent)
    {
        InventoryComponent->MoveSelection(1);

        if (HeldItemDisplay)
        {
            UItem* CurrentItem = InventoryComponent->GetSelectedItem();
            if (CurrentItem)
            {
                HeldItemDisplay->UpdateDisplay(CurrentItem, InventoryComponent->bIsInventoryOpen);
            }
        }
    }
}

void AHamoniaCharacter::OnInventoryUse()
{
    if (InventoryComponent)
    {
        InventoryComponent->UseSelectedItem();
    }
}

void AHamoniaCharacter::InitializeDialogueSystem()
{
    if (DialogueManager && DefaultDialogueDataTable)
    {
        DialogueManager->DialogueDataTable = DefaultDialogueDataTable;
    }
}

bool AHamoniaCharacter::IsDialogueSystemReady() const
{
    return DialogueManager && DialogueManager->DialogueDataTable != nullptr;
}

void AHamoniaCharacter::SaveBeforeLevelTransition()
{
    if (UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        FString CurrentLevel = GetWorld()->GetMapName();
        CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

        GameInstance->UpdatePlayerLocation(CurrentLevel, GetActorLocation(), GetActorRotation());
        GameInstance->SaveContinueGame();
    }
}

void AHamoniaCharacter::SetCurrentInteractableNPC(AUnia* NPC)
{
}

void AHamoniaCharacter::RemoveInteractableNPC(AUnia* NPC)
{
}

void AHamoniaCharacter::ShowHeldItemMesh()
{
    if (HeldItemMeshComponent)
    {
        HeldItemMeshComponent->SetVisibility(true);
    }
}

void AHamoniaCharacter::HideHeldItemMesh()
{
    if (HeldItemMeshComponent)
    {
        HeldItemMeshComponent->SetVisibility(false);
    }
}