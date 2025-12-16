#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AITypes.h"
#include "BTTask_MoveToLocation.generated.h"

UCLASS()
class DISTRICT_TEST_API UBTTask_MoveToLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToLocation();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
    virtual FString GetStaticDescription() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TargetLocationKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float AcceptanceRadius = 50.0f;

private:
    FAIRequestID MoveRequestID;
};