#include "AI/BTService_UpdatePlayerData.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdatePlayerData::UBTService_UpdatePlayerData()
{
    NodeName = "Update Player Data";
    Interval = 0.2f;
    RandomDeviation = 0.05f;
}

void UBTService_UpdatePlayerData::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIController->GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    BlackboardComp->SetValueAsObject(PlayerPawnKey.SelectedKeyName, PlayerPawn);

    APawn* OwnPawn = AIController->GetPawn();
    if (OwnPawn)
    {
        FVector CurrentLocation = OwnPawn->GetActorLocation();
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        float Distance = FVector::Dist(CurrentLocation, PlayerLocation);

        float TeleportDistance = BlackboardComp->GetValueAsFloat(TeleportDistanceKey.SelectedKeyName);
        if (Distance < TeleportDistance)
        {
            BlackboardComp->SetValueAsVector(LastValidPositionKey.SelectedKeyName, CurrentLocation);
        }
    }
}