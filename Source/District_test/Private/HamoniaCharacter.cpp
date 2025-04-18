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

	// 상호작용 관련 변수 초기화
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

	// Enhanced Input 설정을 BeginPlay에서 (컨트롤러가 완전히 설정된 후) 진행
	// 지연된 호출을 통해 입력 설정이 확실히 적용되도록 함
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);

	// 디버그: 레벨에서 PickupActor 찾기
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

	// 상호작용 중인 액터가 유효한지 확인 (이미 제거되었을 수 있음)
	if (CurrentInteractableActor && !IsValid(CurrentInteractableActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Interactable actor was destroyed, resetting state"));
		bIsLookingAtInteractable = false;
		CurrentInteractableActor = nullptr;
		CurrentInteractionText.Empty();
	}

	// 기본 레이캐스트 방식으로 검사
	CheckForInteractables();

	// 레이캐스트로 상호작용 가능한 객체를 찾지 못한 경우에만 근접 검사 실행
	if (!bIsLookingAtInteractable)
	{
		// PickupActor 검사
		for (TActorIterator<APickupActor> It(GetWorld()); It; ++It)
		{
			APickupActor* PickupActor = *It;
			if (IsValid(PickupActor)) // 유효한 액터인지 확인
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

		// 여전히 상호작용 가능한 객체를 찾지 못했다면, 일반 액터 중 인터페이스 구현체 검사
		if (!bIsLookingAtInteractable)
		{
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				AActor* Actor = *It;
				// A_Reflect나 관련 액터인지 확인 (이름으로 확인하거나 클래스로 확인)
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

	// 디버그 라인 그리기
	DrawDebugInteractionLine();
}

void AHamoniaCharacter::SetupEnhancedInput()
{
	// 지연된 호출을 통해 컨트롤러가 확실히 설정된 후 입력 시스템을 설정
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
		// 실패한 경우 다시 시도
		UE_LOG(LogTemp, Warning, TEXT("SetupEnhancedInput delayed - Controller or MappingContext not ready"));
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
	}
}


void AHamoniaCharacter::DrawDebugInteractionLine()
{
	FVector Start = CameraComponent->GetComponentLocation();
	FVector End = Start + (CameraComponent->GetForwardVector() * InteractionDistance);

	// 상호작용 가능 여부에 따라 색상 변경
	FColor LineColor = bIsLookingAtInteractable ? FColor::Green : FColor::Red;

	// 디버그 라인 그리기
	DrawDebugLine(GetWorld(), Start, End, LineColor, false, -1.0f, 0, 1.0f);

	if (bIsLookingAtInteractable && CurrentInteractableActor)
	{
		// 상호작용 가능한 오브젝트에 구체 표시
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
	// 입력값 추출
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

		// 명확한 움직임을 위해 벡터를 정규화하지 않고 직접 사용
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHamoniaCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 시선 이동 속도를 조절하기 위한 감도 적용
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

	// 이전 상태 저장
	bool bWasLookingAtInteractable = bIsLookingAtInteractable;
	AActor* PreviousInteractableActor = CurrentInteractableActor;

	// 레이캐스트 검사만 초기화 (근접 검사 결과 유지를 위해)
	bool bFoundWithRaycast = false;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		// 인터페이스 구현 여부 확인
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			// 상호작용 가능 여부 확인
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

	// 레이캐스트 검사 실패 시에만 상태 초기화 (근접 검사를 위해)
	if (!bFoundWithRaycast && bWasLookingAtInteractable && PreviousInteractableActor == CurrentInteractableActor)
	{
		bIsLookingAtInteractable = false;
		CurrentInteractionText = FString();
		CurrentInteractableActor = nullptr;
	}

	// 상태가 변경되었을 때만 로그 출력
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