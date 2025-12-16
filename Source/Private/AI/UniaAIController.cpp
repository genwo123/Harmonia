#include "AI/UniaAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
AUniaAIController::AUniaAIController()
{
    PrimaryActorTick.bCanEverTick = false;
    bCanTeleportNow = true;
}
void AUniaAIController::BeginPlay()
{
    Super::BeginPlay();
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}
void AUniaAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (InPawn)
    {
        if (ACharacter* UniaCharacter = Cast<ACharacter>(InPawn))
        {
            if (UCharacterMovementComponent* MovementComp = UniaCharacter->GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = 400.0f;
            }
        }
    }
}
void AUniaAIController::StartFollowingPlayer()
{
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->SetValueAsBool(TEXT("bCanFollow"), true);
    }
}
void AUniaAIController::StopFollowing()
{
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->SetValueAsBool(TEXT("bCanFollow"), false);
    }
    StopMovement();
}
void AUniaAIController::MoveToTargetLocation(const FVector& TargetLocation)
{
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->SetValueAsBool(TEXT("bCanFollow"), false);
        BBComp->SetValueAsVector(TEXT("TargetLocation"), TargetLocation);
    }
}
void AUniaAIController::SetDialogueMode(bool bInDialogue)
{
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->SetValueAsBool(TEXT("bIsInDialogue"), bInDialogue);
    }
}
bool AUniaAIController::IsFollowingPlayer() const
{
    const UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        return BBComp->GetValueAsBool(TEXT("bCanFollow"));
    }
    return false;
}
float AUniaAIController::GetDistanceToPlayer() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && GetPawn())
    {
        return FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
    }
    return -1.0f;
}
void AUniaAIController::SetBlackboardValues()
{
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->SetValueAsFloat(TEXT("FollowDistance"), 400.0f);
        BBComp->SetValueAsFloat(TEXT("StopDistance"), 200.0f);
        BBComp->SetValueAsFloat(TEXT("TeleportDistance"), 1000.0f);
        BBComp->SetValueAsBool(TEXT("bCanFollow"), false);
        BBComp->SetValueAsBool(TEXT("bIsInDialogue"), false);
    }
}

bool AUniaAIController::CheckTeleportDistance()
{
    if (!bCanTeleportNow)
    {
        return false;
    }

    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (!BBComp)
    {
        return false;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    APawn* OwnPawn = GetPawn();

    if (!PlayerPawn || !OwnPawn)
    {
        return false;
    }

    float TeleportDistance = BBComp->GetValueAsFloat(TEXT("TeleportDistance"));
    if (TeleportDistance <= 0)
    {
        TeleportDistance = 1500.0f;
    }

    float CurrentDistance = FVector::Dist(OwnPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

    return CurrentDistance > TeleportDistance;
}

void AUniaAIController::TeleportToPlayer()
{
    if (!bCanTeleportNow)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    APawn* OwnPawn = GetPawn();

    if (!PlayerPawn || !OwnPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector PlayerForward = PlayerPawn->GetActorForwardVector();
    FVector TeleportLocation = PlayerLocation - (PlayerForward * 300.0f);

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(TeleportLocation, NavLocation, FVector(200.0f, 200.0f, 500.0f)))
        {
            TeleportLocation = NavLocation.Location;
        }
    }

    OwnPawn->SetActorLocation(TeleportLocation);

    FVector DirectionToPlayer = PlayerLocation - TeleportLocation;
    DirectionToPlayer.Z = 0;
    if (!DirectionToPlayer.IsNearlyZero())
    {
        DirectionToPlayer.Normalize();
        FRotator NewRotation = DirectionToPlayer.Rotation();
        OwnPawn->SetActorRotation(NewRotation);
    }

    SetTeleportCooldown(3.0f);
}

void AUniaAIController::SetTeleportCooldown(float CooldownTime)
{
    bCanTeleportNow = false;

    GetWorld()->GetTimerManager().SetTimer(
        TeleportCooldownTimer,
        this,
        &AUniaAIController::OnTeleportCooldownComplete,
        CooldownTime,
        false
    );
}

bool AUniaAIController::CanTeleport() const
{
    return bCanTeleportNow;
}

void AUniaAIController::OnTeleportCooldownComplete()
{
    bCanTeleportNow = true;
}