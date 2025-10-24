#include "Character/Unia.h"
#include "Character/HamoniaCharacter.h"
#include "Character/UniaWaitSpot.h"
#include "Core/DialogueManagerComponent.h"
#include "AI/UniaAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Save_Instance/Hamoina_GameInstance.h"

AUnia::AUnia()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_Pawn);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	bLookAtPlayer = true;
	LookAtSpeed = 2.0f;
	LookAtRange = 600.0f;
	bIsFollowingPlayer = false;
	bCanFollow = false;

	AIControllerClass = AUniaAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AUnia::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[Unia] BeginPlay - Location: %s"), *GetActorLocation().ToString());

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUnia::OnInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUnia::OnInteractionSphereEndOverlap);

	UE_LOG(LogTemp, Warning, TEXT("[Unia] Overlap events bound"));

	FindPlayerPawn();
	InitializeLevelSettings();
	LoadStateFromGameInstance();
}

void AUnia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bLookAtPlayer && PlayerPawn && bPlayerInRange)
	{
		UpdateLookAtPlayer(DeltaTime);
	}
}

void AUnia::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AUnia::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("[Unia] Interact_Implementation called"));
	StartDialogue(Interactor);
}

bool AUnia::CanInteract_Implementation(AActor* Interactor)
{
	bool bCan = bPlayerInRange && !IsInDialogue();
	UE_LOG(LogTemp, Warning, TEXT("[Unia] CanInteract: %s"), bCan ? TEXT("TRUE") : TEXT("FALSE"));
	return bCan;
}

FString AUnia::GetInteractionText_Implementation()
{
	return FString::Printf(TEXT("Talk to %s"), *NPCName);
}

EInteractionType AUnia::GetInteractionType_Implementation()
{
	return EInteractionType::Use;
}

void AUnia::StartDialogue(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("[Unia] StartDialogue called"));

	AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(Interactor);
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("[Unia] Interactor is not HamoniaCharacter"));
		return;
	}

	UDialogueManagerComponent* PlayerDM = Player->GetDialogueManagerComponent();
	if (!PlayerDM)
	{
		UE_LOG(LogTemp, Error, TEXT("[Unia] PlayerDM is null"));
		return;
	}

	if (!PlayerDM->DialogueDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[Unia] DialogueDataTable is null"));
		return;
	}

	if (PlayerDM->bIsInDialogue)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Unia] Already in dialogue"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Unia] Broadcasting dialogue: %s"), *DialogueSceneID);
	OnUniaDialogueActivated.Broadcast(DialogueSceneID, PlayerDM->DialogueDataTable);
	OnDialogueStarted();
	UE_LOG(LogTemp, Warning, TEXT("[Unia] Dialogue broadcast complete"));
}

void AUnia::EndDialogue()
{
	SetDialogueState(false);
}

void AUnia::OnDialogueWidgetClosed()
{
	SetDialogueState(false);

	if (bCanFollow && !bIsFollowingPlayer)
	{
		SetAIFollowing(true);
		StartFollowingPlayer();
	}
}

bool AUnia::IsInDialogue() const
{
	if (PlayerPawn)
	{
		AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(PlayerPawn);
		if (Player)
		{
			UDialogueManagerComponent* PlayerDM = Player->GetDialogueManagerComponent();
			if (PlayerDM)
			{
				return PlayerDM->bIsInDialogue;
			}
		}
	}
	return false;
}

void AUnia::SetDialogueState(bool bInDialogue)
{
	if (AUniaAIController* AIController = GetUniaAIController())
	{
		AIController->SetDialogueMode(bInDialogue);
	}
}

void AUnia::HandlePlayerInteraction()
{
	UE_LOG(LogTemp, Warning, TEXT("[Unia] HandlePlayerInteraction called"));
	if (PlayerPawn)
	{
		StartDialogue(PlayerPawn);
	}
}

void AUnia::EnableFollowing()
{
	bCanFollow = true;
	SetAIFollowing(true);
}

void AUnia::SetFollowPlayer(bool bShouldFollow)
{
	bIsFollowingPlayer = bShouldFollow;

	if (AUniaAIController* AIController = GetUniaAIController())
	{
		if (bShouldFollow)
		{
			AIController->StartFollowingPlayer();
		}
		else
		{
			AIController->StopFollowing();
		}
	}
}

AUniaAIController* AUnia::GetUniaAIController() const
{
	return Cast<AUniaAIController>(GetController());
}

void AUnia::SetAIFollowing(bool bShouldFollow)
{
	if (AUniaAIController* AIController = GetUniaAIController())
	{
		if (bShouldFollow)
		{
			AIController->StartFollowingPlayer();
			bIsFollowingPlayer = true;
		}
		else
		{
			AIController->StopFollowing();
			bIsFollowingPlayer = false;
		}
	}
}

void AUnia::MoveAIToLocation(const FVector& Location)
{
	if (AUniaAIController* AIController = GetUniaAIController())
	{
		AIController->MoveToTargetLocation(Location);
	}
}

bool AUnia::MoveToWaitSpot(const FString& SpotID)
{
	AUniaWaitSpot* WaitSpot = FindWaitSpot(SpotID);
	if (!WaitSpot)
	{
		return false;
	}

	CurrentTargetSpotID = SpotID;
	MoveAIToLocation(WaitSpot->GetWaitLocation());

	GetWorld()->GetTimerManager().SetTimer(SpotCheckTimer, this, &AUnia::CheckSpotArrival, 0.5f, true);

	return true;
}

void AUnia::SetDialogueSpotMapping(const FString& DialogueID, const FString& SpotID)
{
	DialogueToSpotMap.Add(DialogueID, SpotID);
}

void AUnia::CheckDialogueForAIAction(const FString& DialogueID)
{
	if (FollowActivationDialogues.Contains(DialogueID))
	{
		EnableFollowing();
		return;
	}

	if (DialogueToSpotMap.Contains(DialogueID))
	{
		FString SpotID = DialogueToSpotMap[DialogueID];
		MoveToWaitSpot(SpotID);
	}
}

void AUnia::InitializeLevelSettings()
{
	if (bStartWithFollowing)
	{
		EnableFollowing();
	}

	if (!DefaultWaitSpotID.IsEmpty())
	{
		FTimerHandle InitTimer;
		GetWorld()->GetTimerManager().SetTimer(InitTimer, [this]()
			{
				MoveToWaitSpot(DefaultWaitSpotID);
			}, 0.5f, false);
	}
}

void AUnia::SaveStateToGameInstance()
{
	UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	UHamonia_SaveGame* SaveData = GameInstance->GetCurrentSaveData();
	if (!SaveData)
	{
		return;
	}

	FString CurrentLevel = GetWorld()->GetMapName();
	CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveData->SetUniaLocation(CurrentLevel, GetActorLocation(), GetActorRotation());
	SaveData->UniaData.bCanFollow = bCanFollow;
	SaveData->UniaData.bIsFollowingPlayer = bIsFollowingPlayer;
}

void AUnia::LoadStateFromGameInstance()
{
	UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	UHamonia_SaveGame* SaveData = GameInstance->GetCurrentSaveData();
	if (!SaveData)
	{
		return;
	}

	FString CurrentLevel = GetWorld()->GetMapName();
	CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (SaveData->UniaData.CurrentLevel == CurrentLevel)
	{
		SetActorLocation(SaveData->UniaData.UniaLocation);
		SetActorRotation(SaveData->UniaData.UniaRotation);
		bCanFollow = SaveData->UniaData.bCanFollow;
		bIsFollowingPlayer = SaveData->UniaData.bIsFollowingPlayer;

		if (bIsFollowingPlayer)
		{
			SetAIFollowing(true);
		}
	}
}

void AUnia::LookAtPlayer()
{
	if (!PlayerPawn || !bLookAtPlayer)
	{
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > LookAtRange)
	{
		return;
	}

	FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f;
	FRotator TargetRotation = Direction.Rotation();

	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), LookAtSpeed));
}

void AUnia::UpdateLookAtPlayer(float DeltaTime)
{
	if (!PlayerPawn || IsInDialogue())
	{
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > LookAtRange)
	{
		return;
	}

	FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f;

	if (!Direction.IsNearlyZero())
	{
		FRotator TargetRotation = Direction.Rotation();
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, LookAtSpeed);
		SetActorRotation(NewRotation);
	}
}

void AUnia::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[Unia] Overlap detected with: %s"), *OtherActor->GetName());

	AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(OtherActor);
	if (Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Unia] Player entered range"));
		bPlayerInRange = true;
		Player->SetCurrentInteractableNPC(this);
		OnPlayerEnterRange(Player);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Unia] Not a player"));
	}
}

void AUnia::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("[Unia] Overlap ended with: %s"), *OtherActor->GetName());

	AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(OtherActor);
	if (Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Unia] Player exited range"));
		bPlayerInRange = false;
		Player->RemoveInteractableNPC(this);
		OnPlayerExitRange(Player);
	}
}

void AUnia::FindPlayerPawn()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Unia] Player found: %s"), *PlayerPawn->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Unia] Player not found!"));
	}
}

void AUnia::CheckSpotArrival()
{
	if (CurrentTargetSpotID.IsEmpty())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpotCheckTimer);
		return;
	}

	AUniaWaitSpot* WaitSpot = FindWaitSpot(CurrentTargetSpotID);
	if (!WaitSpot)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpotCheckTimer);
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), WaitSpot->GetWaitLocation());
	if (Distance <= SpotArrivalThreshold)
	{
		CurrentTargetSpotID = TEXT("");
		GetWorld()->GetTimerManager().ClearTimer(SpotCheckTimer);

		if (AUniaAIController* AIController = GetUniaAIController())
		{
			AIController->StopMovement();
		}
	}
}

AUniaWaitSpot* AUnia::FindWaitSpot(const FString& SpotID)
{
	TArray<AActor*> FoundSpots;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUniaWaitSpot::StaticClass(), FoundSpots);

	FString CurrentLevel = GetWorld()->GetMapName();
	CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	for (AActor* Actor : FoundSpots)
	{
		AUniaWaitSpot* WaitSpot = Cast<AUniaWaitSpot>(Actor);
		if (WaitSpot && WaitSpot->SpotID == SpotID)
		{
			if (WaitSpot->IsValidForLevel(CurrentLevel))
			{
				return WaitSpot;
			}
		}
	}

	return nullptr;
}