#include "Character/Unia_Demo.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/DialogueManagerComponent.h"

AUnia_Demo::AUnia_Demo()
{
	PrimaryActorTick.bCanEverTick = true;

	// ��ȣ�ۿ� ���Ǿ� ������Ʈ ����
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// ���̾�α� �Ŵ��� ������Ʈ ����
	DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));

	// ĳ���� �⺻ ���� (���� Unia�� ����)
	GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 0.0f;
	GetCharacterMovement()->AirControl = 0.0f;
}

void AUnia_Demo::BeginPlay()
{
	Super::BeginPlay();
	FindPlayerPawn();

	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(InteractionRange);
	}

	UE_LOG(LogTemp, Log, TEXT("Unia_Demo initialized with %d dialogue steps"), DialogueSequence.Num());
}

void AUnia_Demo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// �÷��̾� �ٶ󺸱� (���̾�α� ���� �ƴ� ����)
	if (bLookAtPlayer && PlayerPawn && !IsInDialogue())
	{
		UpdateLookAtPlayer(DeltaTime);
	}
}

void AUnia_Demo::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AUnia_Demo::Interact_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation(Interactor))
	{
		return;
	}
	StartDialogue(Interactor);
}

bool AUnia_Demo::CanInteract_Implementation(AActor* Interactor)
{
	if (IsInDialogue())
	{
		return false;
	}

	APawn* CurrentPlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Interactor != CurrentPlayerPawn)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
	return Distance <= InteractionRange;
}

FString AUnia_Demo::GetInteractionText_Implementation()
{
	if (IsInDialogue())
	{
		return TEXT("Dialogue...");
	}
	return FString::Printf(TEXT("Talk to %s"), *NPCName);
}

EInteractionType AUnia_Demo::GetInteractionType_Implementation()
{
	return EInteractionType::Activate;
}

void AUnia_Demo::StartDialogue(AActor* Interactor)
{
	if (!DialogueManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager is null!"));
		return;
	}

	// �÷��̾� ������ ȸ��
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

	// ����Ʈ ��� ���̸� �ݺ� ���, �ƴϸ� ���� ���̾�α�
	if (ShouldShowRepeatDialogue())
	{
		DialogueIDToUse = GetRandomRepeatDialogue();
		TableToUse = RepeatDialogueTable;
		UE_LOG(LogTemp, Log, TEXT("Unia_Demo showing repeat dialogue: %s"), *DialogueIDToUse);
	}
	else
	{
		DialogueIDToUse = GetCurrentDialogueID();
		TableToUse = MainDialogueTable;
		UE_LOG(LogTemp, Log, TEXT("Unia_Demo showing main dialogue: %s"), *DialogueIDToUse);
	}

	if (TableToUse)
	{
		DialogueManager->DialogueDataTable = TableToUse;
	}

	DialogueManager->bIsInDialogue = true;
	OnDialogueStarted();

	UE_LOG(LogTemp, Log, TEXT("Unia_Demo started dialogue with ID: %s"), *DialogueIDToUse);
}

bool AUnia_Demo::CanProgressToNextStep()
{
	// ���� �ܰ��� ��� ����Ʈ�� �Ϸ�Ǿ����� üũ
	if (CurrentDialogueStep < RequiredQuestsByStep.Num())
	{
		return CheckAllQuestsForStep(CurrentDialogueStep);
	}
	return true;
}

FString AUnia_Demo::GetCurrentDialogueID()
{
	// ���� �ܰ��� ���̾�α� ID ��ȯ
	if (CurrentDialogueStep < DialogueSequence.Num())
	{
		return DialogueSequence[CurrentDialogueStep];
	}

	// ��� ���̾�α׸� �Ϸ��� ��� ������ �ݺ� ���
	return GetRandomRepeatDialogue();
}

FString AUnia_Demo::GetRandomRepeatDialogue()
{
	if (!RepeatDialogueTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("RepeatDialogueTable is null"));
		return TEXT("Default_Repeat_001");
	}

	TArray<FName> RowNames = RepeatDialogueTable->GetRowNames();
	if (RowNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No rows found in RepeatDialogueTable"));
		return TEXT("Default_Repeat_001");
	}

	int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
	FString SelectedDialogueID = RowNames[RandomIndex].ToString();

	UE_LOG(LogTemp, Log, TEXT("Selected random repeat dialogue: %s"), *SelectedDialogueID);
	return SelectedDialogueID;
}

void AUnia_Demo::OnDialogueCompleted()
{
	// ���̾�α� �Ϸ� �� ���� �ܰ�� ���� �������� üũ
	if (!ShouldShowRepeatDialogue())
	{
		CurrentDialogueStep++;
		UE_LOG(LogTemp, Log, TEXT("Dialogue step advanced to: %d"), CurrentDialogueStep);
	}

	OnDialogueEnded();
}

bool AUnia_Demo::IsInDialogue() const
{
	return DialogueManager ? DialogueManager->bIsInDialogue : false;
}

bool AUnia_Demo::IsWaitingForQuest() const
{
	// ���� �ܰ迡 �ʿ��� ����Ʈ�� �ְ�, ���� �Ϸ���� ���� ���
	if (CurrentDialogueStep < RequiredQuestsByStep.Num())
	{
		// const_cast�� ����ϰų� �ٸ� ������� �ذ�
		// BlueprintImplementableEvent�� const ȣ���� �����Ƿ� �ٸ� ��� ���
		const FDialogueStepQuests& CurrentStepQuests = RequiredQuestsByStep[CurrentDialogueStep];

		// ����Ʈ�� ������ ��� ���� �ƴ�
		if (CurrentStepQuests.RequiredQuestIDs.Num() == 0)
		{
			return false;
		}

		// ����Ʈ�� ������ ��� �� (BP���� ���� üũ�� ������)
		return true;
	}
	return false;
}

void AUnia_Demo::SetDialogueState(bool bInDialogue)
{
	if (DialogueManager)
	{
		DialogueManager->bIsInDialogue = bInDialogue;
	}
}

void AUnia_Demo::UpdateLookAtPlayer(float DeltaTime)
{
	if (!PlayerPawn)
	{
		FindPlayerPawn();
		return;
	}

	// �ٶ󺸱� ���� üũ
	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > LookAtRange)
	{
		return;
	}

	// �÷��̾� �������� �ε巴�� ȸ��
	FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	DirectionToPlayer.Z = 0;
	DirectionToPlayer.Normalize();

	FRotator TargetRotation = DirectionToPlayer.Rotation();
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, LookAtSpeed);
	SetActorRotation(NewRotation);
}

void AUnia_Demo::FindPlayerPawn()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find PlayerPawn"));
	}
}

bool AUnia_Demo::ShouldShowRepeatDialogue() const
{
	// ����Ʈ ��� ���̰ų� ��� ���̾�α׸� �Ϸ��� ��� �ݺ� ���
	bool bWaitingForQuest = IsWaitingForQuest();
	bool bAllDialoguesComplete = (CurrentDialogueStep >= DialogueSequence.Num());

	// ���� ����Ʈ �Ϸ� üũ�� non-const �Լ������� �����ϹǷ�
	// ���⼭�� ������ üũ�� �ϰ�, ���� üũ�� StartDialogue����
	return bWaitingForQuest || bAllDialoguesComplete;
}