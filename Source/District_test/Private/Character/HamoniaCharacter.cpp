// HamoniaCharacter.cpp - 로그 정리 및 디버그 토글 적용
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

	// 상호작용 관련 변수 초기화
	bIsLookingAtInteractable = false;
	CurrentInteractableActor = nullptr;

	DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));

	// 디버그 표시 기본값
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

	// Enhanced Input 설정
	FTimerHandle InputTimerHandle;
	GetWorldTimerManager().SetTimer(InputTimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);

	// DialogueManager 이벤트 바인딩
	if (DialogueManager)
	{
		DialogueManager->OnDialogueStarted.AddDynamic(this, &AHamoniaCharacter::OnDialogueStarted);
		DialogueManager->OnDialogueEnded.AddDynamic(this, &AHamoniaCharacter::OnDialogueEnded);

		// DataTable 설정 (클래스 디폴트에서 설정된 경우)
		if (DefaultDialogueDataTable)
		{
			DialogueManager->DialogueDataTable = DefaultDialogueDataTable;
		}
	}

	// 기본 대화 자동 시작 설정
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

	// 상호작용 중인 액터가 유효한지 확인
	if (CurrentInteractableActor && !IsValid(CurrentInteractableActor))
	{
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

		// 일반 액터 중 인터페이스 구현체 검사
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

	// 디버그 라인 그리기 (설정에 따라)
	if (bShowDebugLines)
	{
		DrawDebugInteractionLine();
	}

	// 키 입력 직접 확인
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		// R 키 (회전)
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

		// E 키 (밀기)
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

		// F 키로 디버그 토글 기능 제거
		// 이제 에디터에서만 bShowDebugLines 체크박스로 제어
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
			// 실패한 경우 다시 시도
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
		}
	}
	else
	{
		// 실패한 경우 다시 시도
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AHamoniaCharacter::SetupEnhancedInput, 0.1f, false);
	}
}

void AHamoniaCharacter::DrawDebugInteractionLine()
{
	// 디버그 표시가 꺼져있으면 리턴
	if (!bShowDebugLines) return;

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

		// 회전 액션 (R 키 → 밀기 기능)
		if (RotateAction)
		{
			EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &AHamoniaCharacter::PushObject);
		}

		// 밀기 액션 (E 키 → 회전 기능)
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
		// 현재 들고 있는 오브젝트가 있는지 확인
		AActor* HeldObject = GetHeldObject();

		// 1. 오브젝트를 들고 있는 경우
		if (HeldObject)
		{
			UPuzzleInteractionComponent* HeldItemComp =
				HeldObject->FindComponentByClass<UPuzzleInteractionComponent>();

			if (HeldItemComp)
			{
				// 받침대인지 확인
				APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);

				if (Pedestal)
				{
					// 받침대에 오브젝트 배치
					HeldItemComp->PlaceOnPedestal(Pedestal);
				}
				else
				{
					// 바닥에 내려놓기
					FVector DropLocation = GetActorLocation() + (GetActorForwardVector() * 100.0f);
					HeldItemComp->PutDown(DropLocation, GetActorRotation());
				}

				// 퀘스트 이벤트 호출 (배치/내려놓기 후)
				if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
				{
					IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
				}
				return;
			}
		}
		// 2. 오브젝트를 들고 있지 않은 경우
		else
		{
			// 2.1 현재 바라보는 대상이 받침대인 경우
			APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
			if (Pedestal)
			{
				// 받침대 위에 오브젝트가 있는지 확인
				AActor* ObjectOnPedestal = Pedestal->GetPlacedObject();
				if (ObjectOnPedestal)
				{
					// 오브젝트의 상호작용 컴포넌트 확인
					UPuzzleInteractionComponent* InteractionComp =
						ObjectOnPedestal->FindComponentByClass<UPuzzleInteractionComponent>();

					if (InteractionComp)
					{
						// 오브젝트 집기 시도
						InteractionComp->PickUp(this);

						// 퀘스트 이벤트 호출 (집기 후)
						if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
						{
							IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
						}
						return;
					}
				}

				// 받침대 자체와 상호작용
				IInteractableInterface::Execute_Interact(Pedestal, this);

				// 퀘스트 이벤트 호출 (받침대 상호작용 후)
				if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
				{
					IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
				}
				return;
			}

			// 2.2 현재 바라보는 대상이 PuzzleInteractionComponent를 가진 오브젝트인 경우
			UPuzzleInteractionComponent* InteractionComp =
				CurrentInteractableActor->FindComponentByClass<UPuzzleInteractionComponent>();

			if (InteractionComp && InteractionComp->bCanBePickedUp)
			{
				// 오브젝트 집기 시도
				InteractionComp->PickUp(this);

				// 퀘스트 이벤트 호출 (픽업 후)
				if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
				{
					IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, this);
				}
				return;
			}
		}

		// 3. 기타 일반 상호작용
		IInteractableInterface::Execute_Interact(CurrentInteractableActor, this);

		// 4. 퀘스트 이벤트 호출 (일반 상호작용 후)
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
			// 플레이어가 바라보는 방향으로 밀어내기
			FVector Direction = CameraComponent->GetForwardVector();
			Direction.Z = 0; // 수평 방향만 고려
			Direction.Normalize();
			Pedestal->Push(Direction);
		}
	}
}

void AHamoniaCharacter::OnDialogueStarted(ESpeakerType Speaker, FText DialogueText, EDialogueType Type, float Duration)
{
	// WBP에서 처리할 수 있도록 빈 구현 또는
	// 기본적인 로그 출력
	UE_LOG(LogTemp, Log, TEXT("Dialogue Started: %s"), *DialogueText.ToString());
}

void AHamoniaCharacter::OnDialogueEnded()
{
	// 대화 종료 시 처리 (입력 모드 복원 등)
	UE_LOG(LogTemp, Log, TEXT("Dialogue Ended"));
}

UDialogueManagerComponent* AHamoniaCharacter::GetDialogueManager()
{
	return DialogueManager;
}