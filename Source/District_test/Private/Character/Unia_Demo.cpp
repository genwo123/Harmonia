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

	// 상호작용 스피어 컴포넌트 생성
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 다이얼로그 매니저 컴포넌트 생성
	DialogueManager = CreateDefaultSubobject<UDialogueManagerComponent>(TEXT("DialogueManager"));

	// 캐릭터 기본 설정 (기존 Unia와 동일)
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

	// 플레이어 바라보기 (다이얼로그 중이 아닐 때만)
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

	// 플레이어 쪽으로 회전
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

	// 퀘스트 대기 중이면 반복 대사, 아니면 메인 다이얼로그
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
	// 현재 단계의 모든 퀘스트가 완료되었는지 체크
	if (CurrentDialogueStep < RequiredQuestsByStep.Num())
	{
		return CheckAllQuestsForStep(CurrentDialogueStep);
	}
	return true;
}

FString AUnia_Demo::GetCurrentDialogueID()
{
	// 현재 단계의 다이얼로그 ID 반환
	if (CurrentDialogueStep < DialogueSequence.Num())
	{
		return DialogueSequence[CurrentDialogueStep];
	}

	// 모든 다이얼로그를 완료한 경우 마지막 반복 대사
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
	// 다이얼로그 완료 후 다음 단계로 진행 가능한지 체크
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
	// 현재 단계에 필요한 퀘스트가 있고, 아직 완료되지 않은 경우
	if (CurrentDialogueStep < RequiredQuestsByStep.Num())
	{
		// const_cast를 사용하거나 다른 방법으로 해결
		// BlueprintImplementableEvent는 const 호출이 어려우므로 다른 방식 사용
		const FDialogueStepQuests& CurrentStepQuests = RequiredQuestsByStep[CurrentDialogueStep];

		// 퀘스트가 없으면 대기 중이 아님
		if (CurrentStepQuests.RequiredQuestIDs.Num() == 0)
		{
			return false;
		}

		// 퀘스트가 있으면 대기 중 (BP에서 실제 체크는 별도로)
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

	// 바라보기 범위 체크
	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > LookAtRange)
	{
		return;
	}

	// 플레이어 방향으로 부드럽게 회전
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
	// 퀘스트 대기 중이거나 모든 다이얼로그를 완료한 경우 반복 대사
	bool bWaitingForQuest = IsWaitingForQuest();
	bool bAllDialoguesComplete = (CurrentDialogueStep >= DialogueSequence.Num());

	// 실제 퀘스트 완료 체크는 non-const 함수에서만 가능하므로
	// 여기서는 간단한 체크만 하고, 실제 체크는 StartDialogue에서
	return bWaitingForQuest || bAllDialoguesComplete;
}