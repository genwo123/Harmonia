// HamoniaCharacter.cpp - �α� ���� �� ����� ��� ����
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
#include <Core/EnhancedQuestComponent.h>

AHamoniaCharacter::AHamoniaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create inventory component
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	// Setup first person camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	CameraComponent->bUsePawnControlRotation = true;

	// Set capsule size
	GetCapsuleComponent()->InitCapsuleSize(36.0f, 88.0f);

	// Enable crouching
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// ��ȣ�ۿ� ���� ���� �ʱ�ȭ
	bIsLookingAtInteractable = false;
	CurrentInteractableActor = nullptr;

	DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));

	// ����� ǥ�� �⺻��
	bShowDebugLines = true;
}

void AHamoniaCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply movement settings
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->MaxWalkSpeed = WalkSpeed;
		MovementComponent->MaxWalkSpeedCrouched = CrouchSpeed;
		MovementComponent->JumpZVelocity = JumpHeight;
		MovementComponent->AirControl = 0.2f;
		MovementComponent->SetCrouchedHalfHeight(44.0f);
	}

	// Enhanced Input ����
	FTimerHandle InputTimerHandle;
	GetWorldTimerManager().SetTimer(InputTimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);

	// DialogueManager �̺�Ʈ ���ε�
	if (DialogueManager)
	{
		DialogueManager->OnDialogueStarted.AddDynamic(this, &AHamoniaCharacter::OnDialogueStarted);
		DialogueManager->OnDialogueEnded.AddDynamic(this, &AHamoniaCharacter::OnDialogueEnded);

		// DataTable ���� (Ŭ���� ����Ʈ���� ������ ���)
		if (DefaultDialogueDataTable)
		{
			DialogueManager->DialogueDataTable = DefaultDialogueDataTable;
		}
	}

	// �⺻ ��ȭ �ڵ� ���� ����
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

void AHamoniaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ��ȣ�ۿ� ���� ���Ͱ� ��ȿ���� Ȯ��
	if (CurrentInteractableActor && !IsValid(CurrentInteractableActor))
	{
		bIsLookingAtInteractable = false;
		CurrentInteractableActor = nullptr;
		CurrentInteractionText.Empty();
	}

	// �⺻ ����ĳ��Ʈ ������� �˻�
	CheckForInteractables();

	// ����ĳ��Ʈ�� ��ȣ�ۿ� ������ ��ü�� ã�� ���� ��쿡�� ���� �˻� ����
	if (!bIsLookingAtInteractable)
	{
		// PickupActor �˻�
		for (TActorIterator<APickupActor> It(GetWorld()); It; ++It)
		{
			APickupActor* PickupActor = *It;
			if (IsValid(PickupActor))
			{
				float Distance = FVector::Distance(GetActorLocation(), PickupActor->GetActorLocation());
				if (Distance < 400.0f)
				{
					bIsLookingAtInteractable = true;
					CurrentInteractableActor = PickupActor;
					CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(PickupActor);
					break;
				}
			}
		}

		// �Ϲ� ���� �� �������̽� ����ü �˻�
		if (!bIsLookingAtInteractable)
		{
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				AActor* Actor = *It;
				if (IsValid(Actor) &&
					Actor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()) &&
					(Actor->GetName().Contains("Reflect") || Actor->GetName().Contains("Mirror")))
				{
					float Distance = FVector::Distance(GetActorLocation(), Actor->GetActorLocation());
					if (Distance < 400.0f)
					{
						bIsLookingAtInteractable = true;
						CurrentInteractableActor = Actor;
						CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(Actor);
						break;
					}
				}
			}
		}
	}

	// ����� ���� �׸��� (������ ����)
	if (bShowDebugLines)
	{
		DrawDebugInteractionLine();
	}

	// Ű �Է� ���� Ȯ��
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		// R Ű (ȸ��)
		if (PC->WasInputKeyJustPressed(EKeys::R))
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

		// E Ű (�б�)
		if (PC->WasInputKeyJustPressed(EKeys::E))
		{
			if (bIsLookingAtInteractable && CurrentInteractableActor)
			{
				APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
				if (Pedestal)
				{
					FVector Direction = Pedestal->GetActorLocation() - GetActorLocation();
					Direction.Z = 0;
					Direction.Normalize();
					Pedestal->Push(Direction);
				}
			}
		}

		// F Ű�� ����� ��� ��� ����
		// ���� �����Ϳ����� bShowDebugLines üũ�ڽ��� ����
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
			// ������ ��� �ٽ� �õ�
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
		}
	}
	else
	{
		// ������ ��� �ٽ� �õ�
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
	}
}

void AHamoniaCharacter::DrawDebugInteractionLine()
{
	// ����� ǥ�ð� ���������� ����
	if (!bShowDebugLines) return;

	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * InteractionDistance);

	// ��ȣ�ۿ� ���� ���ο� ���� ���� ����
	FColor LineColor = bIsLookingAtInteractable ? FColor::Green : FColor::Red;

	// ����� ���� �׸���
	DrawDebugLine(GetWorld(), Start, End, LineColor, false, -1.0f, 0, 1.0f);

	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		// ��ȣ�ۿ� ������ ������Ʈ�� ��ü ǥ��
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

void AHamoniaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		// Movement
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Move);
		}

		// Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Look);
		}

		// Jumping
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// Sprinting
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHamoniaCharacter::StopSprint);
		}

		// Crouching
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::ToggleCrouch);
		}

		// ȸ�� �׼� (R Ű �� �б� ���)
		if (RotateAction)
		{
			EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &AHamoniaCharacter::PushObject);
		}

		// �б� �׼� (E Ű �� ȸ�� ���)
		if (PushAction)
		{
			EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Started, this, &AHamoniaCharacter::RotateObject);
		}

		// Interaction
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AHamoniaCharacter::Interact);
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
	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		// ���� ��� �ִ� ������Ʈ�� �ִ��� Ȯ��
		AActor* HeldObject = GetHeldObject();

		// 1. ������Ʈ�� ��� �ִ� ���
		if (HeldObject)
		{
			UPuzzleInteractionComponent* HeldItemComp =
				HeldObject->FindComponentByClass<UPuzzleInteractionComponent>();

			if (HeldItemComp)
			{
				// ��ħ������ Ȯ��
				APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);

				if (Pedestal)
				{
					// ��ħ�뿡 ������Ʈ ��ġ
					HeldItemComp->PlaceOnPedestal(Pedestal);
				}
				else
				{
					// �ٴڿ� ��������
					FVector DropLocation = GetActorLocation() + (GetActorForwardVector() * 100.0f);
					HeldItemComp->PutDown(DropLocation, GetActorRotation());
				}

				// ����Ʈ �̺�Ʈ ȣ�� (��ġ/�������� ��)
				if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
				{
					IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
				}
				return;
			}
		}
		// 2. ������Ʈ�� ��� ���� ���� ���
		else
		{
			// 2.1 ���� �ٶ󺸴� ����� ��ħ���� ���
			APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
			if (Pedestal)
			{
				// ��ħ�� ���� ������Ʈ�� �ִ��� Ȯ��
				AActor* ObjectOnPedestal = Pedestal->GetPlacedObject();
				if (ObjectOnPedestal)
				{
					// ������Ʈ�� ��ȣ�ۿ� ������Ʈ Ȯ��
					UPuzzleInteractionComponent* InteractionComp =
						ObjectOnPedestal->FindComponentByClass<UPuzzleInteractionComponent>();

					if (InteractionComp)
					{
						// ������Ʈ ���� �õ�
						InteractionComp->PickUp(this);

						// ����Ʈ �̺�Ʈ ȣ�� (���� ��)
						if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
						{
							IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
						}
						return;
					}
				}

				// ��ħ�� ��ü�� ��ȣ�ۿ�
				IInteractableInterface::Execute_Interact(Pedestal, this);

				// ����Ʈ �̺�Ʈ ȣ�� (��ħ�� ��ȣ�ۿ� ��)
				if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
				{
					IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
				}
				return;
			}

			// 2.2 ���� �ٶ󺸴� ����� PuzzleInteractionComponent�� ���� ������Ʈ�� ���
			UPuzzleInteractionComponent* InteractionComp =
				CurrentInteractableActor->FindComponentByClass<UPuzzleInteractionComponent>();

			if (InteractionComp && InteractionComp->bCanBePickedUp)
			{
				// ������Ʈ ���� �õ�
				InteractionComp->PickUp(this);

				// ����Ʈ �̺�Ʈ ȣ�� (�Ⱦ� ��)
				if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
				{
					IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
				}
				return;
			}
		}

		// 3. ��Ÿ �Ϲ� ��ȣ�ۿ�
		IInteractableInterface::Execute_Interact(CurrentInteractableActor, this);

		// 4. ����Ʈ �̺�Ʈ ȣ�� (�Ϲ� ��ȣ�ۿ� ��)
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
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;

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
			// �÷��̾ �ٶ󺸴� �������� �о��
			FVector Direction = CameraComponent->GetForwardVector();
			Direction.Z = 0; // ���� ���⸸ ���
			Direction.Normalize();
			Pedestal->Push(Direction);
		}
	}
}

void AHamoniaCharacter::OnDialogueStarted(ESpeakerType Speaker, FText DialogueText, EDialogueType Type, float Duration)
{
	// WBP���� ó���� �� �ֵ��� �� ���� �Ǵ�
	// �⺻���� �α� ���
	UE_LOG(LogTemp, Log, TEXT("Dialogue Started: %s"), *DialogueText.ToString());
}

void AHamoniaCharacter::OnDialogueEnded()
{
	// ��ȭ ���� �� ó�� (�Է� ��� ���� ��)
	UE_LOG(LogTemp, Log, TEXT("Dialogue Ended"));
}

UDialogueManagerComponent* AHamoniaCharacter::GetDialogueManager()
{
	return DialogueManager;
}