#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitAtLocation.generated.h"

UCLASS()
class DISTRICT_TEST_API UBTTask_WaitAtLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_WaitAtLocation();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    bool bFacePlayer = true;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector PlayerPawnKey;
};