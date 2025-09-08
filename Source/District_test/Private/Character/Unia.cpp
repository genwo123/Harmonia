#include "Character/Unia.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/DialogueManagerComponent.h"
#include "Character/HamoniaCharacter.h"

AUnia::AUnia()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);
	InteractionSphere->bHiddenInGame = false;

	DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));

	GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 0.0f;
	GetCharacterMovement()->AirControl = 0.0f;

	bIsFollowingPlayer = false;
	bPlayerInRange = false;
}

void AUnia::BeginPlay()
{
	Super::BeginPlay();
	FindPlayerPawn();

	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(InteractionRange);
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AUnia::OnInteractionSphereBeginOverlap);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AUnia::OnInteractionSphereEndOverlap);
	}
}

void AUnia::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bLookAtPlayer && PlayerPawn && !IsInDialogue())
	{
		UpdateLookAtPlayer(DeltaTime);
	}
}

void AUnia::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AUnia::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHamoniaCharacter* HamoniaCharacter = Cast<AHamoniaCharacter>(OtherActor);
	if (HamoniaCharacter)
	{
		bPlayerInRange = true;
		PlayerPawn = HamoniaCharacter;

		HamoniaCharacter->SetCurrentInteractableNPC(this);

		OnPlayerEnterRange(HamoniaCharacter);
	}
}

void AUnia::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	AHamoniaCharacter* HamoniaCharacter = Cast<AHamoniaCharacter>(OtherActor);
	if (HamoniaCharacter)
	{
		bPlayerInRange = false;

		HamoniaCharacter->RemoveInteractableNPC(this);

		OnPlayerExitRange(HamoniaCharacter);
	}
}

void AUnia::HandlePlayerInteraction()
{
	if (bPlayerInRange && PlayerPawn)
	{
		Interact_Implementation(PlayerPawn);
	}
}

void AUnia::Interact_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return;
	}
	StartDialogue(Interactor);
}

bool AUnia::CanInteract_Implementation(AActor* Interactor)
{
	AHamoniaCharacter* HamoniaCharacter = Cast<AHamoniaCharacter>(Interactor);
	if (!HamoniaCharacter)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
	return Distance <= InteractionRange;
}

FString AUnia::GetInteractionText_Implementation()
{
	if (IsInDialogue())
	{
		return TEXT("Dialogue...");
	}
	return FString::Printf(TEXT("Talk to %s"), *NPCName);
}

EInteractionType AUnia::GetInteractionType_Implementation()
{
	return EInteractionType::Activate;
}

void AUnia::StartDialogue(AActor* Interactor)
{
	if (!DialogueManager)
	{
		return;
	}

	DialogueManager->bIsInDialogue = false;

	if (bIsFollowingPlayer)
	{
		StopFollowingPlayer();
	}

	if (PlayerPawn)
	{
		FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
		DirectionToPlayer.Z = 0;
		DirectionToPlayer.Normalize();
		FRotator TargetRotation = DirectionToPlayer.Rotation();
		SetActorRotation(TargetRotation);
	}

	FString DialogueIDToUse;
	UDataTable* TableToUse = nullptr;

	if (ShouldShowMainStoryDialogue())
	{
		DialogueIDToUse = GetCurrentStoryDialogueID();
		TableToUse = MainStoryDialogueTable;
	}
	else
	{
		FString MacroDialogue = GetQuestMacroDialogue();
		if (!MacroDialogue.IsEmpty())
		{
			DialogueIDToUse = MacroDialogue;
			TableToUse = UniaRandomDialogueTable;
		}
		else
		{
			DialogueIDToUse = GetRandomDialogueID();
			TableToUse = UniaRandomDialogueTable;
		}
	}

	if (TableToUse)
	{
		DialogueManager->DialogueDataTable = TableToUse;
	}

	if (DialogueIDToUse.IsEmpty())
	{
		DialogueIDToUse = DialogueSceneID;
	}

	AHamoniaCharacter* HamoniaCharacter = Cast<AHamoniaCharacter>(Interactor);
	if (HamoniaCharacter)
	{
		UDialogueManagerComponent* PlayerDialogueManager = HamoniaCharacter->GetDialogueManagerComponent();
		if (PlayerDialogueManager && PlayerDialogueManager->DialogueDataTable)
		{
			TableToUse = PlayerDialogueManager->DialogueDataTable;
		}
	}

	DialogueManager->bIsInDialogue = true;
	OnUniaDialogueActivated.Broadcast(DialogueIDToUse, TableToUse);
	OnDialogueStarted();
}

void AUnia::SetFollowPlayer(bool bShouldFollow)
{
	if (bShouldFollow && bCanFollow && !IsInDialogue())
	{
		bIsFollowingPlayer = true;
		StartFollowingPlayer();
	}
	else
	{
		bIsFollowingPlayer = false;
		StopFollowingPlayer();
	}
}

void AUnia::LookAtPlayer()
{
	if (!PlayerPawn || IsInDialogue())
	{
		return;
	}

	FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	DirectionToPlayer.Z = 0;
	DirectionToPlayer.Normalize();
	FRotator TargetRotation = DirectionToPlayer.Rotation();
	SetActorRotation(TargetRotation);
}

bool AUnia::ShouldShowMainStoryDialogue()
{
	if (RequiredQuestID.IsEmpty())
	{
		return false;
	}

	if (!CurrentStoryDialogueID.IsEmpty() && CheckQuestRequirement(RequiredQuestID))
	{
		return true;
	}

	return false;
}

FString AUnia::GetCurrentStoryDialogueID()
{
	return CurrentStoryDialogueID;
}

FString AUnia::GetRandomDialogueID()
{
	if (!UniaRandomDialogueTable)
	{
		return DialogueSceneID;
	}

	TArray<FName> RowNames = UniaRandomDialogueTable->GetRowNames();

	if (RowNames.Num() == 0)
	{
		return DialogueSceneID;
	}

	int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
	FString SelectedDialogueID = RowNames[RandomIndex].ToString();

	return SelectedDialogueID;
}

FString AUnia::GetQuestMacroDialogue()
{
	for (const FUniaQuestMacro& Macro : QuestMacroList)
	{
		if (Macro.QuestID.IsEmpty())
		{
			continue;
		}

		if (IsQuestActive(Macro.QuestID))
		{
			if (IsQuestCompleted(Macro.QuestID))
			{
				if (!CompletedMacroDialogues.Contains(Macro.QuestID))
				{
					CompletedMacroDialogues.Add(Macro.QuestID);
					return Macro.CompletionDialogueID;
				}
			}
			else
			{
				return Macro.ProgressDialogueID;
			}
		}
	}
	return FString();
}

void AUnia::UpdateStoryProgress(const FString& NewStoryDialogueID)
{
	CurrentStoryDialogueID = NewStoryDialogueID;
}

bool AUnia::IsInDialogue() const
{
	return DialogueManager ? DialogueManager->bIsInDialogue : false;
}

void AUnia::SetDialogueState(bool bInDialogue)
{
	if (DialogueManager)
	{
		DialogueManager->bIsInDialogue = bInDialogue;
	}
}

void AUnia::UpdateLookAtPlayer(float DeltaTime)
{
	if (!PlayerPawn)
	{
		FindPlayerPawn();
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > InteractionRange * 1.5f)
	{
		return;
	}

	FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	DirectionToPlayer.Z = 0;
	DirectionToPlayer.Normalize();

	FRotator TargetRotation = DirectionToPlayer.Rotation();
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, LookAtSpeed);
	SetActorRotation(NewRotation);
}

void AUnia::FindPlayerPawn()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}