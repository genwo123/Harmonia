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
#include "TimerManager.h"

AHamoniaCharacter::AHamoniaCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    InteractionComponent = CreateDefaultSubobject<UPlayerInteractionComponent>(TEXT("InteractionComponent"));
    HeldItemDisplay = CreateDefaultSubobject<UHeldItemDisplayComponent>(TEXT("HeldItemDisplay"));

    
    CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    CameraSpringArm->SetupAttachment(GetCapsuleComponent());
    CameraSpringArm->TargetArmLength = 0.0f;
    CameraSpringArm->bDoCollisionTest = true;
    CameraSpringArm->bUsePawnControlRotation = true;
    CameraSpringArm->ProbeSize = 12.0f;
    CameraSpringArm->ProbeChannel = ECC_Camera;


    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CameraComponent->SetupAttachment(CameraSpringArm);  
    CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    CameraComponent->bUsePawnControlRotation = false; 

    HeldItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeldItemMesh"));
    HeldItemMeshComponent->SetupAttachment(CameraComponent);
    HeldItemMeshComponent->SetRelativeLocation(FVector(50.0f, 20.0f, -10.0f));
    HeldItemMeshComponent->SetVisibility(false);
    HeldItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    HeldObjectAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HeldObjectAttachPoint"));
    HeldObjectAttachPoint->SetupAttachment(CameraComponent);
    HeldObjectAttachPoint->SetRelativeLocation(FVector(100.0f, 0.0f, -20.0f));

    GetCapsuleComponent()->InitCapsuleSize(36.0f, 88.0f);
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    bIsLookingAtInteractable = false;
    CurrentInteractableActor = nullptr;

    DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));
}


void AHamoniaCharacter::BeginPlay()
{
    Super::BeginPlay();

    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (MovementComponent)
    {
        MovementComponent->MaxWalkSpeed = WalkSpeed;
        MovementComponent->MaxWalkSpeedCrouched = CrouchSpeed;
        MovementComponent->JumpZVelocity = JumpHeight;
        MovementComponent->AirControl = 0.2f;
        MovementComponent->SetCrouchedHalfHeight(44.0f);
    }

    if (InteractionComponent)
    {
        InteractionComponent->SetupReferences(CameraComponent, InventoryComponent, HeldObjectAttachPoint);
    }

    if (HeldItemDisplay)
    {
        HeldItemDisplay->SetupMeshComponent(HeldItemMeshComponent);
    }

    FTimerHandle InputTimerHandle;
    GetWorldTimerManager().SetTimer(InputTimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetInputMode(FInputModeGameOnly());
    }

    FTimerHandle ForceInputTimerHandle;
    GetWorldTimerManager().SetTimer(ForceInputTimerHandle, [this]()
        {
            if (APlayerController* PC = Cast<APlayerController>(GetController()))
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
                {
                    if (DefaultMappingContext)
                    {
                        Subsystem->RemoveMappingContext(DefaultMappingContext);
                        Subsystem->AddMappingContext(DefaultMappingContext, 0);
                    }
                }
            }
        }, 1.0f, false);

    if (DialogueManager)
    {
        DialogueManager->OnDialogueStarted.AddDynamic(this, &AHamoniaCharacter::OnDialogueStarted);
        DialogueManager->OnDialogueEnded.AddDynamic(this, &AHamoniaCharacter::OnDialogueEnded);

        FTimerHandle DialogueInitTimerHandle;
        GetWorldTimerManager().SetTimer(DialogueInitTimerHandle, this, &AHamoniaCharacter::InitializeDialogueSystem, 0.1f, false);
    }

    if (bAutoStartDialogue && !DefaultDialogueID.IsEmpty())
    {
        FTimerHandle DialogueTimerHandle;
        GetWorldTimerManager().SetTimer(DialogueTimerHandle, [this]()
            {
                if (DialogueManager && !DialogueManager->bIsInDialogue)
                {
                    DialogueManager->StartDialogue(DefaultDialogueID);
                }
            }, DelayBeforeDialogue, false);
    }
}

void AHamoniaCharacter::SaveBeforeLevelTransition()
{
    if (InventoryComponent)
    {
        InventoryComponent->SaveInventoryToGameInstance();
    }

    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetGameInstance());
    if (GameInstance)
    {
        GameInstance->SaveContinueGame();
    }
}

void AHamoniaCharacter::SetupEnhancedInput()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController && DefaultMappingContext)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
        else
        {
            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
        }
    }
    else
    {
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
    }
}

void AHamoniaCharacter::InitializeDialogueSystem()
{
    if (DialogueManager && DefaultDialogueDataTable)
    {
        DialogueManager->DialogueDataTable = DefaultDialogueDataTable;
    }
}

bool AHamoniaCharacter::IsDialogueSystemReady()
{
    return DialogueManager && DialogueManager->DialogueDataTable != nullptr;
}

void AHamoniaCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (InteractionComponent)
    {
        bIsLookingAtInteractable = InteractionComponent->IsLookingAtInteractable();
        CurrentInteractableActor = InteractionComponent->GetCurrentInteractableActor();
        CurrentInteractionText = InteractionComponent->GetCurrentInteractionText();
        CurrentInteractionType = InteractionComponent->GetCurrentInteractionType();
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

void AHamoniaCharacter::StartSprint(const FInputActionValue& Value)
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (MovementComponent)
    {
        MovementComponent->MaxWalkSpeed = SprintSpeed;
        bIsSprinting = true;
    }
}

void AHamoniaCharacter::StopSprint(const FInputActionValue& Value)
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (MovementComponent)
    {
        MovementComponent->MaxWalkSpeed = WalkSpeed;
        bIsSprinting = false;
    }
}

void AHamoniaCharacter::ToggleCrouch(const FInputActionValue& Value)
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (MovementComponent)
    {
        if (MovementComponent->IsCrouching())
        {
            UnCrouch();
            MovementComponent->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
        }
        else
        {
            Crouch();
            MovementComponent->MaxWalkSpeed = CrouchSpeed;
        }
    }
}

void AHamoniaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

    if (EnhancedInputComponent)
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
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        if (SprintAction)
        {
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::StartSprint);
            EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHamoniaCharacter::StopSprint);
        }

        if (CrouchAction)
        {
            EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::ToggleCrouch);
        }

        if (RotateAction)
        {
            EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &AHamoniaCharacter::RotateObject);
        }

        if (PushAction)
        {
            EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Started, this, &AHamoniaCharacter::OnEKeyPressed);
        }

        if (InteractAction)
        {
            EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AHamoniaCharacter::Interact);
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

void AHamoniaCharacter::RotateObject()
{
    if (InteractionComponent)
    {
        InteractionComponent->RotateObject();
    }
}

void AHamoniaCharacter::PushObject()
{
    if (InteractionComponent)
    {
        InteractionComponent->PushObject();
    }
}

void AHamoniaCharacter::OnEKeyPressed()
{
    if (InteractionComponent)
    {
        InteractionComponent->OnEKeyPressed();
    }
}

void AHamoniaCharacter::Interact()
{
    if (InteractionComponent)
    {
        InteractionComponent->PerformInteraction();
    }
}

AActor* AHamoniaCharacter::GetHeldObject()
{
    if (InteractionComponent)
    {
        return InteractionComponent->GetHeldObject();
    }
    return nullptr;
}

void AHamoniaCharacter::CheckForInteractables()
{
    if (InteractionComponent)
    {
        InteractionComponent->CheckForInteractables();
    }
}

void AHamoniaCharacter::DrawDebugInteractionLine()
{
}

EInteractionType AHamoniaCharacter::GetCurrentInteractionType() const
{
    return InteractionComponent ? InteractionComponent->GetCurrentInteractionType() : EInteractionType::Default;
}

bool AHamoniaCharacter::IsLookingAtInteractable() const
{
    return InteractionComponent ? InteractionComponent->IsLookingAtInteractable() : false;
}

FString AHamoniaCharacter::GetCurrentInteractionText() const
{
    return InteractionComponent ? InteractionComponent->GetCurrentInteractionText() : FString();
}

void AHamoniaCharacter::OnDialogueStarted(ESpeakerType Speaker, FText DialogueText, EDialogueType Type, float Duration)
{
}

void AHamoniaCharacter::OnDialogueEnded()
{
}

UDialogueManagerComponent* AHamoniaCharacter::GetDialogueManager()
{
    return DialogueManager;
}

void AHamoniaCharacter::OnInventoryToggle()
{
    if (InventoryComponent)
    {
        InventoryComponent->ToggleInventory();

        if (HeldItemDisplay)
        {
            UItem* SelectedItem = InventoryComponent->GetSelectedItem();
            HeldItemDisplay->UpdateDisplay(SelectedItem, InventoryComponent->bIsInventoryOpen);
        }
    }
}

void AHamoniaCharacter::OnInventoryMoveLeft()
{
    if (InventoryComponent && InventoryComponent->bIsInventoryOpen)
    {
        InventoryComponent->MoveSelection(-1);
        OnInventorySelectionChanged(InventoryComponent->CurrentSelectedSlot);
    }
}

void AHamoniaCharacter::OnInventoryMoveRight()
{
    if (InventoryComponent && InventoryComponent->bIsInventoryOpen)
    {
        InventoryComponent->MoveSelection(1);
        OnInventorySelectionChanged(InventoryComponent->CurrentSelectedSlot);
    }
}

void AHamoniaCharacter::OnInventoryUse()
{
    if (InventoryComponent && InventoryComponent->bIsInventoryOpen)
    {
        InventoryComponent->UseSelectedItem();
    }
}

void AHamoniaCharacter::OnInventorySelectionChanged(int32 NewSlotIndex)
{
    if (InventoryComponent && HeldItemDisplay)
    {
        UItem* NewSelectedItem = InventoryComponent->GetItemAtSlot(NewSlotIndex);
        HeldItemDisplay->UpdateDisplay(NewSelectedItem, InventoryComponent->bIsInventoryOpen);
    }
}

void AHamoniaCharacter::SetCurrentInteractableNPC(AUnia* NPC)
{
    if (InteractionComponent)
    {
        InteractionComponent->SetCurrentInteractableNPC(NPC);
    }
}

void AHamoniaCharacter::RemoveInteractableNPC(AUnia* NPC)
{
    if (InteractionComponent)
    {
        InteractionComponent->RemoveInteractableNPC(NPC);
    }
}

AUnia* AHamoniaCharacter::GetCurrentInteractableNPC() const
{
    return InteractionComponent ? InteractionComponent->GetCurrentInteractableNPC() : nullptr;
}

bool AHamoniaCharacter::HasInteractableNPC() const
{
    return InteractionComponent ? InteractionComponent->HasInteractableNPC() : false;
}

void AHamoniaCharacter::ShowHeldItemMesh(UItem* Item)
{
    if (HeldItemDisplay)
    {
        HeldItemDisplay->ShowItemMesh(Item);
    }
}

void AHamoniaCharacter::HideHeldItemMesh()
{
    if (HeldItemDisplay)
    {
        HeldItemDisplay->HideItemMesh();
    }
}