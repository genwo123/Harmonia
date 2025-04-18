#include "HamoniaCharacter.h"
#include "InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InteractableInterface.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "PickupActor.h"

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
	GetCharacterMovement()->CrouchedHalfHeight = 44.0f;

	// ��ȣ�ۿ� ���� ���� �ʱ�ȭ
	bIsLookingAtInteractable = false;
	CurrentInteractableActor = nullptr;
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
	}

	// Enhanced Input ������ BeginPlay���� (��Ʈ�ѷ��� ������ ������ ��) ����
	// ������ ȣ���� ���� �Է� ������ Ȯ���� ����ǵ��� ��
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);

	// �����: �������� PickupActor ã��
	UE_LOG(LogTemp, Warning, TEXT("Searching for PickupActors in level..."));
	for (TActorIterator<APickupActor> It(GetWorld()); It; ++It)
	{
		APickupActor* PickupActor = *It;
		UE_LOG(LogTemp, Warning, TEXT("Found PickupActor: %s at location %s"),
			*PickupActor->GetName(),
			*PickupActor->GetActorLocation().ToString());
	}
}

void AHamoniaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ��ȣ�ۿ� ���� ���Ͱ� ��ȿ���� Ȯ�� (�̹� ���ŵǾ��� �� ����)
	if (CurrentInteractableActor && !IsValid(CurrentInteractableActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Interactable actor was destroyed, resetting state"));
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
			if (IsValid(PickupActor)) // ��ȿ�� �������� Ȯ��
			{
				float Distance = FVector::Distance(GetActorLocation(), PickupActor->GetActorLocation());
				if (Distance < 400.0f)
				{
					bIsLookingAtInteractable = true;
					CurrentInteractableActor = PickupActor;
					CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(PickupActor);
					UE_LOG(LogTemp, Display, TEXT("Auto-detected PickupActor at distance %f"), Distance);
					break;
				}
			}
		}

		// ������ ��ȣ�ۿ� ������ ��ü�� ã�� ���ߴٸ�, �Ϲ� ���� �� �������̽� ����ü �˻�
		if (!bIsLookingAtInteractable)
		{
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				AActor* Actor = *It;
				// A_Reflect�� ���� �������� Ȯ�� (�̸����� Ȯ���ϰų� Ŭ������ Ȯ��)
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
						UE_LOG(LogTemp, Warning, TEXT("Auto-detected Reflect actor: %s at distance %f"),
							*Actor->GetName(), Distance);
						break;
					}
				}
			}
		}
	}

	// ����� ���� �׸���
	DrawDebugInteractionLine();
}

void AHamoniaCharacter::SetupEnhancedInput()
{
	// ������ ȣ���� ���� ��Ʈ�ѷ��� Ȯ���� ������ �� �Է� �ý����� ����
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController && DefaultMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogTemp, Warning, TEXT("Enhanced input setup successful"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get Enhanced Input subsystem"));
		}
	}
	else
	{
		// ������ ��� �ٽ� �õ�
		UE_LOG(LogTemp, Warning, TEXT("SetupEnhancedInput delayed - Controller or MappingContext not ready"));
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
	}
}


void AHamoniaCharacter::DrawDebugInteractionLine()
{
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

	// Setup Enhanced Input
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent)
	{
		// Movement
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Move);
			UE_LOG(LogTemp, Display, TEXT("Move action bound"));
		}

		// Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Look);
			UE_LOG(LogTemp, Display, TEXT("Look action bound"));
		}

		// Jumping
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
			UE_LOG(LogTemp, Display, TEXT("Jump action bound"));
		}

		// Sprinting
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHamoniaCharacter::StopSprint);
			UE_LOG(LogTemp, Display, TEXT("Sprint action bound"));
		}

		// Crouching
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::ToggleCrouch);
			UE_LOG(LogTemp, Display, TEXT("Crouch action bound"));
		}

		// Interaction
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Interact);
			UE_LOG(LogTemp, Display, TEXT("Interact action bound"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("InteractAction is null - Cannot bind interaction"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to setup Enhanced Input - No EnhancedInputComponent found"));
	}
}

void AHamoniaCharacter::Move(const FInputActionValue& Value)
{
	// �Է°� ����
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Find forward and right directions
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Forward/backward direction
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// Right/left direction
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// ��Ȯ�� �������� ���� ���͸� ����ȭ���� �ʰ� ���� ���
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHamoniaCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// �ü� �̵� �ӵ��� �����ϱ� ���� ���� ����
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
	UE_LOG(LogTemp, Warning, TEXT("Interact called"));

	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interacting with: %s"), *CurrentInteractableActor->GetName());
		IInteractableInterface::Execute_Interact(CurrentInteractableActor, this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No interactable actor found"));
	}
}

void AHamoniaCharacter::CheckForInteractables()
{
	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// ���� ���� ����
	bool bWasLookingAtInteractable = bIsLookingAtInteractable;
	AActor* PreviousInteractableActor = CurrentInteractableActor;

	// ����ĳ��Ʈ �˻縸 �ʱ�ȭ (���� �˻� ��� ������ ����)
	bool bFoundWithRaycast = false;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		// �������̽� ���� ���� Ȯ��
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			// ��ȣ�ۿ� ���� ���� Ȯ��
			if (IInteractableInterface::Execute_CanInteract(HitActor, this))
			{
				bIsLookingAtInteractable = true;
				bFoundWithRaycast = true;
				CurrentInteractableActor = HitActor;
				CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(HitActor);
				UE_LOG(LogTemp, Verbose, TEXT("Can interact with: %s, Text: %s"),
					*HitActor->GetName(), *CurrentInteractionText);
			}
		}
	}

	// ����ĳ��Ʈ �˻� ���� �ÿ��� ���� �ʱ�ȭ (���� �˻縦 ����)
	if (!bFoundWithRaycast && bWasLookingAtInteractable && PreviousInteractableActor == CurrentInteractableActor)
	{
		bIsLookingAtInteractable = false;
		CurrentInteractionText = FString();
		CurrentInteractableActor = nullptr;
	}

	// ���°� ����Ǿ��� ���� �α� ���
	if (bWasLookingAtInteractable != bIsLookingAtInteractable ||
		(bIsLookingAtInteractable && PreviousInteractableActor != CurrentInteractableActor))
	{
		if (bIsLookingAtInteractable)
		{
			UE_LOG(LogTemp, Display, TEXT("Looking at interactable: %s"), *CurrentInteractableActor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("No longer looking at interactable"));
		}
	}
}