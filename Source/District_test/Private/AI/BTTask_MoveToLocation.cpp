#include "AI/BTTask_MoveToLocation.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToLocation::UBTTask_MoveToLocation()
{
    NodeName = "Move To Location";
    bNotifyTick = false;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!AIController || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    FVector TargetLocation = BlackboardComp->GetValueAsVector(TargetLocationKey.SelectedKeyName);
    if (TargetLocation.IsZero())
    {
        return EBTNodeResult::Failed;
    }

    APawn* OwnPawn = AIController->GetPawn();
    if (!OwnPawn)
    {
        return EBTNodeResult::Failed;
    }

    float CurrentDistance = FVector::Dist(OwnPawn->GetActorLocation(), TargetLocation);
    if (CurrentDistance <= AcceptanceRadius)
    {
        BlackboardComp->ClearValue(TargetLocationKey.SelectedKeyName);
        return EBTNodeResult::Succeeded;
    }

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(TargetLocation);
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

    FPathFollowingRequestResult RequestResult = AIController->MoveTo(MoveRequest);
    if (RequestResult.Code == EPathFollowingRequestResult::RequestSuccessful)
    {
        MoveRequestID = RequestResult.MoveId;
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_MoveToLocation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController && MoveRequestID.IsValid())
    {
        AIController->StopMovement();
        MoveRequestID = FAIRequestID::InvalidRequest;
    }
    return EBTNodeResult::Aborted;
}

void UBTTask_MoveToLocation::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (TaskResult == EBTNodeResult::Succeeded && BlackboardComp)
    {
        BlackboardComp->ClearValue(TargetLocationKey.SelectedKeyName);
    }

    MoveRequestID = FAIRequestID::InvalidRequest;
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_MoveToLocation::GetStaticDescription() const
{
    return FString::Printf(TEXT("Move to target location (radius: %.0f)"), AcceptanceRadius);
}