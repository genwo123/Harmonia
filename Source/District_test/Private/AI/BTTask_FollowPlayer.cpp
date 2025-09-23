#include "AI/BTTask_FollowPlayer.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_FollowPlayer::UBTTask_FollowPlayer()
{
    NodeName = "Follow Player";
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_FollowPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    APawn* PlayerPawn = Cast<APawn>(BlackboardComp->GetValueAsObject(PlayerPawnKey.SelectedKeyName));
    if (!PlayerPawn)
    {
        return EBTNodeResult::Failed;
    }

    DelayTimer = 0.0f;
    bPlayerMoved = false;
    LastPlayerLocation = PlayerPawn->GetActorLocation();

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_FollowPlayer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController)
    {
        AIController->StopMovement();
    }

    return EBTNodeResult::Aborted;
}

void UBTTask_FollowPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    APawn* PlayerPawn = Cast<APawn>(BlackboardComp->GetValueAsObject(PlayerPawnKey.SelectedKeyName));
    APawn* OwnPawn = AIController->GetPawn();

    if (!PlayerPawn || !OwnPawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector CurrentPlayerLocation = PlayerPawn->GetActorLocation();
    float FollowDistance = BlackboardComp->GetValueAsFloat(FollowDistanceKey.SelectedKeyName);
    float StopDistance = BlackboardComp->GetValueAsFloat(StopDistanceKey.SelectedKeyName);

    if (FollowDistance <= 0) FollowDistance = 400.0f;
    if (StopDistance <= 0) StopDistance = 200.0f;

    float PlayerMoveDist = FVector::Dist(LastPlayerLocation, CurrentPlayerLocation);
    if (PlayerMoveDist > 50.0f)
    {
        if (!bPlayerMoved)
        {
            bPlayerMoved = true;
            DelayTimer = 0.0f;
        }
        LastPlayerLocation = CurrentPlayerLocation;
    }

    if (bPlayerMoved)
    {
        DelayTimer += DeltaSeconds;

        if (DelayTimer >= FollowDelay)
        {
            float CurrentDistance = FVector::Dist(OwnPawn->GetActorLocation(), CurrentPlayerLocation);

            if (CurrentDistance > FollowDistance)
            {
                FVector PlayerForward = PlayerPawn->GetActorForwardVector();
                FVector TargetLocation = CurrentPlayerLocation - (PlayerForward * StopDistance);

                FAIMoveRequest MoveRequest;
                MoveRequest.SetGoalLocation(TargetLocation);
                MoveRequest.SetAcceptanceRadius(50.0f);

                AIController->MoveTo(MoveRequest);
            }
            else if (CurrentDistance <= StopDistance + 50.0f)
            {
                AIController->StopMovement();
                bPlayerMoved = false;
                DelayTimer = 0.0f;
            }
        }
    }
}
FString UBTTask_FollowPlayer::GetStaticDescription() const
{
    return FString::Printf(TEXT("Follow player with %.1fs delay"), FollowDelay);
}