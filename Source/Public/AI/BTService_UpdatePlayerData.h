#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTService_UpdatePlayerData.generated.h"

UCLASS()
class DISTRICT_TEST_API UBTService_UpdatePlayerData : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdatePlayerData();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector PlayerPawnKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector FollowDistanceKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TeleportDistanceKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector LastValidPositionKey;
};