#include "AI/BTTask_WaitAtLocation.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_WaitAtLocation::UBTTask_WaitAtLocation()
{
    NodeName = "Wait At Location";
    bNotifyTick = false;
    bFacePlayer = true;
}

EBTNodeResult::Type UBTTask_WaitAtLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    AIController->StopMovement();

    if (bFacePlayer)
    {
        UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
        if (BlackboardComp)
        {
            APawn* PlayerPawn = Cast<APawn>(BlackboardComp->GetValueAsObject(PlayerPawnKey.SelectedKeyName));
            APawn* OwnPawn = AIController->GetPawn();

            if (PlayerPawn && OwnPawn)
            {
                FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - OwnPawn->GetActorLocation();
                DirectionToPlayer.Z = 0;
                if (!DirectionToPlayer.IsNearlyZero())
                {
                    FRotator LookRotation = DirectionToPlayer.Rotation();
                    OwnPawn->SetActorRotation(LookRotation);
                }
            }
        }
    }

    return EBTNodeResult::Succeeded;
}

FString UBTTask_WaitAtLocation::GetStaticDescription() const
{
    return FString::Printf(TEXT("Wait at current location%s"),
        bFacePlayer ? TEXT(" and face player") : TEXT(""));
}