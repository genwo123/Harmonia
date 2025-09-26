#include "Character/Unia.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/DialogueManagerComponent.h"
#include "Character/HamoniaCharacter.h"
#include "Character/UniaWaitSpot.h"
#include "Save_Instance/Hamoina_GameInstance.h"
#include "BehaviorTree/BlackboardComponent.h"

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

    AIControllerClass = AUniaAIController::StaticClass();

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

    bCanFollow = false;

    if (AUniaAIController* AIController = GetUniaAIController())
    {
        AIController->StopFollowing();
        if (AIController->GetBlackboardComponent())
        {
            AIController->GetBlackboardComponent()->ClearValue(TEXT("TargetLocation"));
        }
    }
    InitializeLevelSettings();
}

void AUnia::CheckSpotArrival()
{
    if (CurrentTargetSpotID.IsEmpty())
    {
        GetWorld()->GetTimerManager().ClearTimer(SpotCheckTimer);
        return;
    }

    AUniaWaitSpot* TargetSpot = FindWaitSpot(CurrentTargetSpotID);
    if (TargetSpot)
    {
        float Distance = FVector::Dist(GetActorLocation(), TargetSpot->GetWaitLocation());
        if (Distance <= SpotArrivalThreshold)
        {
            GetWorld()->GetTimerManager().ClearTimer(SpotCheckTimer);
            CurrentTargetSpotID = TEXT("");

            if (AUniaAIController* AIController = GetUniaAIController())
            {
                if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
                {
                    BBComp->ClearValue(TEXT("TargetLocation"));
                }
            }
        }
    }
}

void AUnia::InitializeLevelSettings()
{
    if (AUniaAIController* AIController = GetUniaAIController())
    {
        if (bStartWithFollowing)
        {
            bCanFollow = true;
            AIController->StartFollowingPlayer();
        }
        else if (!DefaultWaitSpotID.IsEmpty())
        {
            MoveToWaitSpot(DefaultWaitSpotID);
        }
        else
        {
            bCanFollow = false;
            AIController->StopFollowing();
        }

        if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
        {
            BBComp->SetValueAsFloat(TEXT("TeleportDistance"), TeleportDistance);
        }
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

    if (bIsFollowingPlayer)
    {
        StopFollowingPlayer();
    }

    if (AUniaAIController* AIController = GetUniaAIController())
    {
        AIController->SetDialogueMode(true);
        AIController->StopFollowing();
    }

    if (PlayerPawn)
    {
        FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
        DirectionToPlayer.Z = 0;
        DirectionToPlayer.Normalize();
        FRotator TargetRotation = DirectionToPlayer.Rotation();
        SetActorRotation(TargetRotation);
    }

    AHamoniaCharacter* HamoniaCharacter = Cast<AHamoniaCharacter>(Interactor);
    UDataTable* TableToUse = nullptr;

    if (HamoniaCharacter)
    {
        UDialogueManagerComponent* PlayerDialogueManager = HamoniaCharacter->GetDialogueManagerComponent();
        if (PlayerDialogueManager && PlayerDialogueManager->DialogueDataTable)
        {
            TableToUse = PlayerDialogueManager->DialogueDataTable;
            DialogueManager->DialogueDataTable = TableToUse;
        }
    }

    if (!TableToUse && UniaRandomDialogueTable)
    {
        TableToUse = UniaRandomDialogueTable;
        DialogueManager->DialogueDataTable = TableToUse;
    }

    FString DialogueIDToUse = DialogueManager->FindDialogueForCurrentLevel();

    if (DialogueIDToUse.IsEmpty())
    {
        DialogueIDToUse = DialogueManager->GetMacroDialogue(
            DialogueManager->GetCurrentLevelName(),
            DialogueManager->GetCurrentSubStep()
        );
    }

    if (DialogueIDToUse.IsEmpty())
    {
        DialogueIDToUse = UniaRandomDialogueID;
    }

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
    if (Distance <= LookAtRange)
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
        }
        else
        {
            AIController->StopFollowing();
        }
    }

    SetFollowPlayer(bShouldFollow);
}

void AUnia::MoveAIToLocation(const FVector& Location)
{
    if (AUniaAIController* AIController = GetUniaAIController())
    {
        AIController->MoveToTargetLocation(Location);
    }
}

void AUnia::EnableFollowing()
{
    bCanFollow = true;
    SetAIFollowing(true);
}

void AUnia::EndDialogue()
{
    if (AUniaAIController* AIController = GetUniaAIController())
    {
        AIController->SetDialogueMode(false);

        if (bCanFollow)
        {
            AIController->StartFollowingPlayer();
        }
    }

    if (DialogueManager)
    {
        DialogueManager->bIsInDialogue = false;
    }
}

bool AUnia::MoveToWaitSpot(const FString& SpotID)
{
    AUniaWaitSpot* TargetSpot = FindWaitSpot(SpotID);
    if (!TargetSpot)
    {
        return false;
    }

    if (AUniaAIController* AIController = GetUniaAIController())
    {
        AIController->MoveToTargetLocation(TargetSpot->GetWaitLocation());

        GetWorld()->GetTimerManager().SetTimer(SpotCheckTimer, this, &AUnia::CheckSpotArrival, 0.5f, true);

        return true;
    }

    return false;
}

void AUnia::SetDialogueSpotMapping(const FString& DialogueID, const FString& SpotID)
{
    DialogueToSpotMap.Add(DialogueID, SpotID);
}

void AUnia::CheckDialogueForAIAction(const FString& DialogueID)
{
    // 디버깅용 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("CheckDialogueForAIAction called with: %s"), *DialogueID);

    if (FollowActivationDialogues.Contains(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Enabling following for dialogue: %s"), *DialogueID);
        EnableFollowing();
        return;
    }

    if (FString* SpotID = DialogueToSpotMap.Find(DialogueID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Moving to spot for dialogue: %s"), *DialogueID);
        MoveToWaitSpot(*SpotID);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("No action found for dialogue: %s"), *DialogueID);
}

AUniaWaitSpot* AUnia::FindWaitSpot(const FString& SpotID)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUniaWaitSpot::StaticClass(), FoundActors);

    FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(GetWorld());

    for (AActor* Actor : FoundActors)
    {
        if (AUniaWaitSpot* Spot = Cast<AUniaWaitSpot>(Actor))
        {
            if (Spot->SpotID.Equals(SpotID) && Spot->IsValidForLevel(CurrentLevel))
            {
                return Spot;
            }
        }
    }

    return nullptr;
}

void AUnia::SaveStateToGameInstance()
{
    if (UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetGameInstance()))
    {

    }
}

void AUnia::LoadStateFromGameInstance()
{
    if (UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetGameInstance()))
    {

    }
}

void AUnia::OnDialogueWidgetClosed()
{
    if (AUniaAIController* AIController = GetUniaAIController())
    {
        // 대화 상태 해제
        AIController->SetDialogueMode(false);

        // 팔로잉 다시 시작 (bCanFollow가 true인 경우)
        if (bCanFollow)
        {
            AIController->StartFollowingPlayer();
        }
    }

    if (DialogueManager)
    {
        DialogueManager->bIsInDialogue = false;
    }
}