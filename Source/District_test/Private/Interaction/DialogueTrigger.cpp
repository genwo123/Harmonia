#include "Interaction/DialogueTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Character/HamoniaCharacter.h"
#include "Core/DialogueManagerComponent.h"

ADialogueTrigger::ADialogueTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create trigger box
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create debug mesh
	DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugMesh"));
	DebugMesh->SetupAttachment(RootComponent);
	DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DebugMesh->SetVisibility(true);

	// Set default dialogue scene
	bOneTimeOnly = true;
	bAutoGetFromCharacter = true;
	bHasTriggered = false;
}

void ADialogueTrigger::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADialogueTrigger::OnTriggerBeginOverlap);

	// Auto-get dialogue settings from character if enabled
	if (bAutoGetFromCharacter)
	{
		GetDefaultDialogueSettings();
	}
}

void ADialogueTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if it's the player character
	AHamoniaCharacter* HamoniaCharacter = Cast<AHamoniaCharacter>(OtherActor);
	if (!HamoniaCharacter)
	{
		return;
	}

	// Check if we can trigger
	if (!CanTrigger(HamoniaCharacter))
	{
		return;
	}

	// Trigger dialogue
	TriggerDialogue(HamoniaCharacter);
}

void ADialogueTrigger::TriggerDialogue(AHamoniaCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	// 캐릭터의 기본 DataTable만 사용
	UDialogueManagerComponent* DialogueManager = Character->GetDialogueManagerComponent();
	if (!DialogueManager || !DialogueManager->DialogueDataTable)
	{
		return;
	}

	// BP에서 처리할 수 있도록 이벤트 발생
	OnDialogueTriggerActivated.Broadcast(DialogueSceneID, DialogueManager->DialogueDataTable);

	if (bOneTimeOnly)
	{
		bHasTriggered = true;
	}

	SetDebugMeshVisible(false);
}


bool ADialogueTrigger::CanTrigger(AHamoniaCharacter* Character)
{
	// Check if already triggered and one-time only
	if (bOneTimeOnly && bHasTriggered)
	{
		return false;
	}

	// Check quest requirements (placeholder for future implementation)
	if (bRequireQuestComplete && !RequiredQuestID.IsEmpty())
	{
		// TODO: Implement quest system check
	}

	// Check if dialogue scene ID is valid
	if (DialogueSceneID.IsEmpty())
	{
		return false;
	}

	return true;
}

void ADialogueTrigger::GetDefaultDialogueSettings()
{
	// Get player character
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	AHamoniaCharacter* HamoniaCharacter = Cast<AHamoniaCharacter>(PlayerController->GetPawn());
	if (!HamoniaCharacter)
	{
		return;
	}

	// Get default dialogue ID from character if our ID is empty
	if (DialogueSceneID.IsEmpty() || DialogueSceneID == TEXT("Scene01_001"))
	{
		FString CharacterDefaultID = HamoniaCharacter->GetDefaultDialogueID();
		if (!CharacterDefaultID.IsEmpty())
		{
			DialogueSceneID = CharacterDefaultID;
		}
	}
}

void ADialogueTrigger::SetDebugMeshVisible(bool bVisible)
{
	if (DebugMesh)
	{
		DebugMesh->SetVisibility(bVisible);
	}
}