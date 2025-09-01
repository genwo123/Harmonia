// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Unia.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/DialogueManagerComponent.h"

AUnia::AUnia()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));

	GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 0.0f;
	GetCharacterMovement()->AirControl = 0.0f;

	bIsFollowingPlayer = false;
}

void AUnia::BeginPlay()
{
	Super::BeginPlay();
	FindPlayerPawn();
	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(InteractionRange);
	}
	UE_LOG(LogTemp, Log, TEXT("Unia NPC initialized with DialogueManager"));
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

FString AUnia::GetInteractionText_Implementation()
{
	if (IsInDialogue())
	{
		return TEXT("Dialogue...");
	}
	return FString::Printf(TEXT("%s And Dialogue"), *NPCName);
}

EInteractionType AUnia::GetInteractionType_Implementation()
{
	return EInteractionType::Activate;
}

void AUnia::StartDialogue(AActor* Interactor)
{
	if (!DialogueManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager is null!"));
		return;
	}

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

	// 1순위: 메인 스토리 대화
	if (ShouldShowMainStoryDialogue())
	{
		DialogueIDToUse = GetCurrentStoryDialogueID();
		TableToUse = MainStoryDialogueTable;
		UE_LOG(LogTemp, Log, TEXT("Unia showing story dialogue: %s"), *DialogueIDToUse);
	}
	// 2순위: 퀘스트 매크로 대화 (새로 추가!)
	else
	{
		FString MacroDialogue = GetQuestMacroDialogue();
		if (!MacroDialogue.IsEmpty())
		{
			DialogueIDToUse = MacroDialogue;
			TableToUse = MainStoryDialogueTable; // 또는 별도 테이블 사용 가능
			UE_LOG(LogTemp, Log, TEXT("Unia showing quest macro dialogue: %s"), *DialogueIDToUse);
		}
		// 3순위: 랜덤 대화
		else
		{
			DialogueIDToUse = GetRandomDialogueID();
			TableToUse = UniaRandomDialogueTable;
			UE_LOG(LogTemp, Log, TEXT("Unia showing random dialogue: %s"), *DialogueIDToUse);
		}
	}

	if (TableToUse)
	{
		DialogueManager->DialogueDataTable = TableToUse;
	}

	DialogueManager->bIsInDialogue = true;

	OnDialogueStarted();
	UE_LOG(LogTemp, Log, TEXT("Unia started dialogue with ID: %s"), *DialogueIDToUse);
}

void AUnia::SetFollowPlayer(bool bShouldFollow)
{
	if (bShouldFollow && bCanFollow && !IsInDialogue())
	{
		bIsFollowingPlayer = true;
		StartFollowingPlayer();
		UE_LOG(LogTemp, Log, TEXT("Unia started following player"));
	}
	else
	{
		bIsFollowingPlayer = false;
		StopFollowingPlayer();
		UE_LOG(LogTemp, Log, TEXT("Unia stopped following player"));
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
		UE_LOG(LogTemp, Warning, TEXT("UniaRandomDialogueTable is null"));
		return TEXT("Unia_Default_001");
	}

	TArray<FName> RowNames = UniaRandomDialogueTable->GetRowNames();

	if (RowNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No rows found in UniaRandomDialogueTable"));
		return TEXT("Unia_Default_001");
	}

	int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
	FString SelectedDialogueID = RowNames[RandomIndex].ToString();

	UE_LOG(LogTemp, Log, TEXT("Selected random dialogue: %s"), *SelectedDialogueID);
	return SelectedDialogueID;
}

// 새로 추가된 핵심 함수!
FString AUnia::GetQuestMacroDialogue()
{
	// 퀘스트 매크로 리스트를 순회하면서 활성 퀘스트 확인
	for (const FUniaQuestMacro& Macro : QuestMacroList)
	{
		if (Macro.QuestID.IsEmpty())
		{
			continue;
		}

		// 퀘스트가 활성 상태인지 확인
		if (IsQuestActive(Macro.QuestID))
		{
			// 퀘스트가 완료되었는지 확인
			if (IsQuestCompleted(Macro.QuestID))
			{
				// 완료 대화를 이미 보여줬는지 확인
				if (!CompletedMacroDialogues.Contains(Macro.QuestID))
				{
					// 완료 대화 표시하고 기록에 추가
					CompletedMacroDialogues.Add(Macro.QuestID);
					UE_LOG(LogTemp, Log, TEXT("Quest %s completed, showing completion dialogue: %s"),
						*Macro.QuestID, *Macro.CompletionDialogueID);
					return Macro.CompletionDialogueID;
				}
			}
			else
			{
				// 퀘스트 진행 중이면 진행 대화 표시
				UE_LOG(LogTemp, Log, TEXT("Quest %s in progress, showing progress dialogue: %s"),
					*Macro.QuestID, *Macro.ProgressDialogueID);
				return Macro.ProgressDialogueID;
			}
		}
	}

	// 활성 퀘스트 매크로가 없으면 빈 문자열 반환
	return FString();
}

void AUnia::UpdateStoryProgress(const FString& NewStoryDialogueID)
{
	CurrentStoryDialogueID = NewStoryDialogueID;
	UE_LOG(LogTemp, Log, TEXT("Story progress updated to: %s"), *CurrentStoryDialogueID);
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
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find PlayerPawn"));
	}
}