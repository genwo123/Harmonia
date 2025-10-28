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

	bLevelDialogueEnded = false;
	CurrentLevelName = "";
}

void AUnia::BeginPlay()
{
	Super::BeginPlay();

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUnia::OnInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUnia::OnInteractionSphereEndOverlap);

	FindPlayerPawn();
	InitializeLevelSettings();
	LoadStateFromGameInstance();

	UWorld* World = GetWorld();
	if (World)
	{
		CurrentLevelName = World->GetMapName();
		CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
	}

	bLevelDialogueEnded = false;
}

void AUnia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (World)
	{
		FString NewLevelName = World->GetMapName();
		NewLevelName.RemoveFromStart(World->StreamingLevelsPrefix);

		if (NewLevelName != CurrentLevelName && !NewLevelName.IsEmpty())
		{
			CurrentLevelName = NewLevelName;
			ResetLevelDialogueState();
		}
	}

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
	StartDialogue(Interactor);
}

bool AUnia::CanInteract_Implementation(AActor* Interactor)
{
	if (bLevelDialogueEnded)
	{
		return false;
	}

	AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(Interactor);
	if (!Player)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	return Distance <= InteractionRange && !IsInDialogue();
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
	AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(Interactor);
	if (!Player)
	{
		return;
	}

	UDialogueManagerComponent* PlayerDM = Player->GetDialogueManagerComponent();
	if (!PlayerDM || !PlayerDM->DialogueDataTable || PlayerDM->bIsInDialogue)
	{
		return;
	}

	OnUniaDialogueActivated.Broadcast(DialogueSceneID, PlayerDM->DialogueDataTable);
	OnDialogueStarted();
}

void AUnia::EndDialogue()
{
	if (PlayerPawn)
	{
		AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(PlayerPawn);
		if (Player)
		{
			UDialogueManagerComponent* PlayerDM = Player->GetDialogueManagerComponent();
			if (PlayerDM && PlayerDM->IsCurrentDialogueLevelEnd())
			{
				SetLevelDialogueEnded(true);
			}
		}
	}

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
	if (PlayerPawn)
	{
		StartDialogue(PlayerPawn);
	}
}

void AUnia::SetLevelDialogueEnded(bool bEnded)
{
	bLevelDialogueEnded = bEnded;
}

void AUnia::ResetLevelDialogueState()
{
	bLevelDialogueEnded = false;
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
	AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(OtherActor);
	if (Player)
	{
		bPlayerInRange = true;
		Player->SetCurrentInteractableNPC(this);
		OnPlayerEnterRange(Player);
	}
}

void AUnia::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	AHamoniaCharacter* Player = Cast<AHamoniaCharacter>(OtherActor);
	if (Player)
	{
		bPlayerInRange = false;
		Player->RemoveInteractableNPC(this);
		OnPlayerExitRange(Player);
	}
}

void AUnia::FindPlayerPawn()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
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