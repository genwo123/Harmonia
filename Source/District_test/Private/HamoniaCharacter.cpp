// HamoniaCharacter.cpp
#include "HamoniaCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AHamoniaCharacter::AHamoniaCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Character movement 설정
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
    GetCharacterMovement()->CrouchedHalfHeight = CrouchedHalfHeight;
    GetCharacterMovement()->JumpZVelocity = JumpZVelocity;

    // Create camera component
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CameraComponent->SetupAttachment(GetCapsuleComponent());
    CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f + BaseEyeHeight));
    CameraComponent->bUsePawnControlRotation = true;
}

void AHamoniaCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 캐릭터 이동 속성 초기화
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->CrouchedHalfHeight = CrouchedHalfHeight;
    GetCharacterMovement()->JumpZVelocity = JumpZVelocity;

    // Set up enhanced input
    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    if (PlayerController != nullptr)
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
        if (Subsystem != nullptr)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AHamoniaCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AHamoniaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (EnhancedInputComponent != nullptr)
    {
        // Moving
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

void AHamoniaCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // 감도 적용 및 Y축 반전 옵션 처리
        float SensitivityX = LookSensitivity;
        float SensitivityY = bInvertYAxis ? -LookSensitivity : LookSensitivity;

        // Add yaw and pitch input
        AddControllerYawInput(LookAxisVector.X * SensitivityX);
        AddControllerPitchInput(LookAxisVector.Y * SensitivityY);
    }
}

void AHamoniaCharacter::StartSprint(const FInputActionValue& Value)
{
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AHamoniaCharacter::StopSprint(const FInputActionValue& Value)
{
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AHamoniaCharacter::ToggleCrouch(const FInputActionValue& Value)
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

void AHamoniaCharacter::Interact(const FInputActionValue& Value)
{
    // 상호작용 실행
    UE_LOG(LogTemp, Display, TEXT("Interact button pressed"));

    // 레이 트레이스를 통한 상호작용 구현
    FVector Start = CameraComponent->GetComponentLocation();
    FVector ForwardVector = CameraComponent->GetForwardVector();
    FVector End = Start + (ForwardVector * InteractionDistance); // 사용자 정의 거리 사용

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
            // 상호작용 인터페이스 구현 대상
            // 나중에 IInteractableInterface를 구현하면 해당 인터페이스로 상호작용 처리
        }
    }
}