#include "AI/UniaAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

AUniaAIController::AUniaAIController()
{
    PrimaryActorTick.bCanEverTick = false;
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
        BBComp->SetValueAsFloat(TEXT("FollowDistance"), 200.0f);
        BBComp->SetValueAsFloat(TEXT("StopDistance"), 100.0f);
        BBComp->SetValueAsFloat(TEXT("TeleportDistance"), 1000.0f);
        BBComp->SetValueAsBool(TEXT("bCanFollow"), false);
        BBComp->SetValueAsBool(TEXT("bIsInDialogue"), false);
    }
}