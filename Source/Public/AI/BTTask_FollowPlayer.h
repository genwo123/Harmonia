#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FollowPlayer.generated.h"

UCLASS()
class DISTRICT_TEST_API UBTTask_FollowPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FollowPlayer();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual FString GetStaticDescription() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector PlayerPawnKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector FollowDistanceKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector StopDistanceKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float FollowDelay = 3.0f;

private:
    float DelayTimer = 0.0f;
    bool bPlayerMoved = false;
    FVector LastPlayerLocation = FVector::ZeroVector;
};