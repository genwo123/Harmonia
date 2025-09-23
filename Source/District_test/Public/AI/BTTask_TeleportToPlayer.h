#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TeleportToPlayer.generated.h"

UCLASS()
class DISTRICT_TEST_API UBTTask_TeleportToPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_TeleportToPlayer();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector PlayerPawnKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float TeleportThreshold = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "AI")
    float TeleportBehindDistance = 300.0f;
};