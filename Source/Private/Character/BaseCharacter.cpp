// BaseCharacter.cpp
#include "Character/BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Character movement 설정
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
    GetCharacterMovement()->SetCrouchedHalfHeight(60.0f);

    // Create camera component
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CameraComponent->SetupAttachment(GetCapsuleComponent());
    CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f + BaseEyeHeight));
    CameraComponent->bUsePawnControlRotation = true;
}

void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Set up enhanced input
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            //Subsystem->ClearMappingContext(DefaultMappingContext);
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // Set default walk speed
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);

        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Sprinting
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ABaseCharacter::StartSprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopSprint);

        // Crouching
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABaseCharacter::ToggleCrouch);

        // Interaction
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Interact);
    }
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get forward and right vectors
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Add movement
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Add yaw and pitch input
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABaseCharacter::StartSprint()
{
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ABaseCharacter::StopSprint()
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ABaseCharacter::ToggleCrouch()
{
    if (GetCharacterMovement()->IsCrouching())
    {
        UnCrouch();
        GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
    }
    else
    {
        Crouch();
        GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
    }
}

void ABaseCharacter::Interact()
{
    // 상호작용 실행 (아직 구현되지 않음)
    UE_LOG(LogTemp, Display, TEXT("Interact button pressed"));

    // 상호작용 로직은 후에 구현
    FVector Start = CameraComponent->GetComponentLocation();
    FVector ForwardVector = CameraComponent->GetForwardVector();
    FVector End = Start + (ForwardVector * 200.0f); // 200cm 레이 캐스팅

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    // 레이 트레이스 수행
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        AActor* HitActor = HitResult.GetActor();
        
        if (HitActor)
        {
            UE_LOG(LogTemp, Display, TEXT("Hit Actor: %s"), *HitActor->GetName());
            // 후에 상호작용 인터페이스 구현 예정
        }
    }
}