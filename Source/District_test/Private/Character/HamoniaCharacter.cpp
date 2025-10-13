#include "Character/HamoniaCharacter.h"
#include "Gameplay/InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Interaction/InteractableInterface.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h" 
#include "Gameplay/PuzzleInteractionComponent.h" 
#include "Gameplay/Pedestal.h"
#include "Interaction/InteractableMechanism.h"
#include "Gameplay/PickupActor.h"
#include "TimerManager.h"
#include <Core/EnhancedQuestComponent.h>

AHamoniaCharacter::AHamoniaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->bUsePawnControlRotation = true;

	CurrentInteractableNPC = nullptr;

	GetCapsuleComponent()->InitCapsuleSize(36.0f, 88.0f);

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	bIsLookingAtInteractable = false;
	CurrentInteractableActor = nullptr;

	HeldItemDisplay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeldItemDisplay"));
	HeldItemDisplay->SetupAttachment(CameraComponent);
	HeldItemDisplay->SetRelativeLocation(FVector(50.0f, 20.0f, -10.0f));
	HeldItemDisplay->SetVisibility(false);

	
	CurrentDisplayedItem = nullptr;

	DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));

	bShowDebugLines = true;
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

	if (CurrentInteractableActor && !IsValid(CurrentInteractableActor))
	{
		bIsLookingAtInteractable = false;
		CurrentInteractableActor = nullptr;
		CurrentInteractionText.Empty();
	}

	CheckForInteractables();

	if (bShowDebugLines)
	{
		DrawDebugInteractionLine();
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
			EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &AHamoniaCharacter::PushObject);
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
	}
}

void AHamoniaCharacter::DrawDebugInteractionLine()
{
	if (!bShowDebugLines) return;

	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * InteractionDistance);

	FColor LineColor = bIsLookingAtInteractable ? FColor::Green : FColor::Red;

	DrawDebugLine(GetWorld(), Start, End, LineColor, false, -1.0f, 0, 1.0f);

	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		DrawDebugSphere(
			GetWorld(),
			CurrentInteractableActor->GetActorLocation(),
			30.0f,
			12,
			FColor::Green,
			false,
			-1.0f,
			0,
			1.0f
		);
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

void AHamoniaCharacter::Interact()
{
	if (DialogueManager)
	{
		if (DialogueManager->bIsInDialogue)
		{
			OnDialogueProgressRequested.Broadcast();
			return;
		}
	}

	if (CurrentInteractableNPC)
	{
		CurrentInteractableNPC->HandlePlayerInteraction();
		return;
	}

	AActor* HeldObject = GetHeldObject();

	if (HeldObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interact] Currently holding: %s"), *HeldObject->GetName());
		UE_LOG(LogTemp, Warning, TEXT("[Interact] bIsLookingAtInteractable: %d"), bIsLookingAtInteractable);

		if (CurrentInteractableActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Interact] CurrentInteractableActor: %s"), *CurrentInteractableActor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Interact] CurrentInteractableActor is NULL"));
		}

		UPuzzleInteractionComponent* HeldItemComp =
			HeldObject->FindComponentByClass<UPuzzleInteractionComponent>();

		if (HeldItemComp)
		{
			if (bIsLookingAtInteractable && CurrentInteractableActor)
			{
				APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
				if (Pedestal)
				{
					UE_LOG(LogTemp, Warning, TEXT("[Interact] Placing on Pedestal"));
					HeldItemComp->PlaceOnPedestal(Pedestal);
					if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
					{
						IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
					}
					return;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[Interact] Not a Pedestal - Type: %s"),
						*CurrentInteractableActor->GetClass()->GetName());
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("[Interact] Dropping object"));
			FVector DropLocation = GetActorLocation() + (GetActorForwardVector() * 150.0f);
			DropLocation.Z = GetActorLocation().Z;
			HeldItemComp->PutDown(DropLocation, GetActorRotation());
			return;
		}
	}

	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		UItem* HeldInventoryItem = GetCurrentHeldInventoryItem();
		if (HeldInventoryItem && HandleInventoryItemInteraction(HeldInventoryItem, CurrentInteractableActor))
		{
			return;
		}

		APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
		if (Pedestal)
		{
			AActor* ObjectOnPedestal = Pedestal->GetPlacedObject();
			if (ObjectOnPedestal)
			{
				UPuzzleInteractionComponent* InteractionComp =
					ObjectOnPedestal->FindComponentByClass<UPuzzleInteractionComponent>();
				if (InteractionComp)
				{
					InteractionComp->PickUp(this);
					if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
					{
						IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
					}
					return;
				}
			}
			IInteractableInterface::Execute_Interact(Pedestal, this);
			if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
			{
				IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
			}
			return;
		}

		UPuzzleInteractionComponent* InteractionComp =
			CurrentInteractableActor->FindComponentByClass<UPuzzleInteractionComponent>();
		if (InteractionComp && InteractionComp->bCanBePickedUp)
		{
			InteractionComp->PickUp(this);
			if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
			{
				IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
			}
			return;
		}

		IInteractableInterface::Execute_Interact(CurrentInteractableActor, this);
		if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
		}
	}
}


AActor* AHamoniaCharacter::GetHeldObject()
{
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		UPuzzleInteractionComponent* InteractionComp =
			Actor->FindComponentByClass<UPuzzleInteractionComponent>();

		if (InteractionComp && InteractionComp->HoldingActor == this)
		{
			return Actor;
		}
	}

	return nullptr;
}

void AHamoniaCharacter::CheckForInteractables()
{
	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// 들고 있는 오브젝트 무시
	AActor* HeldObject = GetHeldObject();
	if (HeldObject)
	{
		QueryParams.AddIgnoredActor(HeldObject);
		UE_LOG(LogTemp, Warning, TEXT("[CheckInteractables] Ignoring held object: %s"), *HeldObject->GetName());
	}

	ECollisionChannel TraceChannel = ECC_Visibility;

	bIsLookingAtInteractable = false;
	CurrentInteractableActor = nullptr;
	CurrentInteractionText.Empty();
	CurrentInteractionType = EInteractionType::Default;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, QueryParams);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			bool bCanInteractResult = IInteractableInterface::Execute_CanInteract(HitActor, this);

			if (bCanInteractResult)
			{
				bIsLookingAtInteractable = true;
				CurrentInteractableActor = HitActor;
				CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(HitActor);
				CurrentInteractionType = IInteractableInterface::Execute_GetInteractionType(HitActor);

				UE_LOG(LogTemp, Warning, TEXT("[CheckInteractables] Found: %s (Type: %s)"),
					*HitActor->GetName(), *HitActor->GetClass()->GetName());
			}
		}
	}
}

EInteractionType AHamoniaCharacter::GetCurrentInteractionType() const
{
	return CurrentInteractionType;
}

bool AHamoniaCharacter::IsLookingAtInteractable() const
{
	return bIsLookingAtInteractable;
}

FString AHamoniaCharacter::GetCurrentInteractionText() const
{
	return CurrentInteractionText;
}

void AHamoniaCharacter::RotateObject()
{
	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
		if (Pedestal)
		{
			Pedestal->Rotate();
		}
	}
}

void AHamoniaCharacter::PushObject()
{
	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
		if (Pedestal)
		{
			FVector Direction = CameraComponent->GetForwardVector();
			Direction.Z = 0;
			Direction.Normalize();
			Pedestal->Push(Direction);
		}
	}
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

		if (InventoryComponent->bIsInventoryOpen)
		{
			UItem* SelectedItem = InventoryComponent->GetSelectedItem();
			UpdateHeldItemDisplay(SelectedItem);
		}
		else
		{
			HideHeldItemMesh();
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

void AHamoniaCharacter::OnEKeyPressed()
{
	if (InventoryComponent && InventoryComponent->bIsInventoryOpen)
	{
		OnInventoryUse();
	}
	else
	{
		PushObject();
	}
}

void AHamoniaCharacter::OnInventoryUse()
{
	if (InventoryComponent)
	{
		InventoryComponent->UseSelectedItem();
	}
}

UItem* AHamoniaCharacter::GetCurrentHeldInventoryItem()
{
	if (InventoryComponent && InventoryComponent->bIsInventoryOpen)
	{
		return InventoryComponent->GetSelectedItem();
	}
	return nullptr;
}

bool AHamoniaCharacter::HandleInventoryItemInteraction(UItem* Item, AActor* TargetActor)
{
	if (!Item || !TargetActor)
	{
		return false;
	}

	if (Item->Name.Contains("Key"))
	{
		AInteractableMechanism* Door = Cast<AInteractableMechanism>(TargetActor);
		if (Door && Door->MechanismType == EMechanismType::Door)
		{
			if (Door->RequiredKeyName.Equals(Item->Name, ESearchCase::IgnoreCase))
			{
				IInteractableInterface::Execute_Interact(Door, this);
				return true;
			}
		}
	}

	if (Item->Name.Contains("Tool"))
	{
		APedestal* Pedestal = Cast<APedestal>(TargetActor);
		if (Pedestal)
		{
			Item->Use(this);
			return true;
		}
	}

	if (Item->bCanBeUsed)
	{
		Item->Use(this);
		return true;
	}

	return false;
}

void AHamoniaCharacter::UpdateHeldItemDisplay(UItem* NewItem)
{
	CurrentDisplayedItem = NewItem;

	if (NewItem && InventoryComponent && InventoryComponent->bIsInventoryOpen)
	{
		ShowHeldItemMesh(NewItem);
	}
	else
	{
		HideHeldItemMesh();
	}
}

void AHamoniaCharacter::ShowHeldItemMesh(UItem* Item)
{
	if (Item && HeldItemDisplay)
	{
		ShowHeldItemMeshBP(Item);
	}
	else
	{
		HideHeldItemMeshBP();
	}
}

void AHamoniaCharacter::HideHeldItemMesh()
{
	HideHeldItemMeshBP();
}

void AHamoniaCharacter::OnInventorySelectionChanged(int32 NewSlotIndex)
{
	if (InventoryComponent)
	{
		UItem* NewSelectedItem = InventoryComponent->GetItemAtSlot(NewSlotIndex);
		UpdateHeldItemDisplay(NewSelectedItem);
	}
}

void AHamoniaCharacter::SetCurrentInteractableNPC(AUnia* NPC)
{
	if (NPC && !InteractableNPCs.Contains(NPC))
	{
		InteractableNPCs.Add(NPC);
	}
	CurrentInteractableNPC = NPC;
}

void AHamoniaCharacter::RemoveInteractableNPC(AUnia* NPC)
{
	InteractableNPCs.Remove(NPC);

	if (CurrentInteractableNPC == NPC)
	{
		CurrentInteractableNPC = InteractableNPCs.Num() > 0 ? InteractableNPCs[0] : nullptr;
	}
}