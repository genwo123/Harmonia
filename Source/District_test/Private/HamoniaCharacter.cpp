#include "HamoniaCharacter.h"
#include "InventoryComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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
		}
	}
	else
	{
		// ������ ��� �ٽ� �õ�
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
	}
}

void AHamoniaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHamoniaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setup Enhanced Input
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Look);

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHamoniaCharacter::StopSprint);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::ToggleCrouch);

		// Interaction
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AHamoniaCharacter::Interact);
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
	CheckForInteractables();
}

void AHamoniaCharacter::CheckForInteractables()
{
	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		// �������Ʈ���� ó���� �� �ֵ��� �⺻ ������ ����
	}
}