#include "AI/BTTask_TeleportToPlayer.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_TeleportToPlayer::UBTTask_TeleportToPlayer()
{
    NodeName = "Teleport To Player";
    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_TeleportToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    APawn* PlayerPawn = Cast<APawn>(BlackboardComp->GetValueAsObject(PlayerPawnKey.SelectedKeyName));
    APawn* OwnPawn = AIController->GetPawn();

    if (!PlayerPawn || !OwnPawn)
    {
        return EBTNodeResult::Failed;
    }

    float Distance = FVector::Dist(OwnPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (Distance <= TeleportThreshold)
    {
        return EBTNodeResult::Failed;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector PlayerForward = PlayerPawn->GetActorForwardVector();
    FVector TeleportLocation = PlayerLocation - (PlayerForward * TeleportBehindDistance);

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(AIController->GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(TeleportLocation, NavLocation, FVector(200.0f, 200.0f, 500.0f)))
        {
            TeleportLocation = NavLocation.Location;
        }
    }

    OwnPawn->SetActorLocation(TeleportLocation);
    AIController->StopMovement();

    return EBTNodeResult::Succeeded;
}

FString UBTTask_TeleportToPlayer::GetStaticDescription() const
{
    return FString::Printf(TEXT("Teleport when >%.0fcm, spawn %.0fcm behind player"), TeleportThreshold, TeleportBehindDistance);
}