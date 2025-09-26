#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UniaWaitSpot.generated.h"
UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API AUniaWaitSpot : public AActor
{
    GENERATED_BODY()
public:
    AUniaWaitSpot();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wait Spot")
    FString SpotID = TEXT("WaitSpot_001");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wait Spot")
    FString LevelName = TEXT("");
    UFUNCTION(BlueprintPure, Category = "Wait Spot")
    FVector GetWaitLocation() const;
    UFUNCTION(BlueprintPure, Category = "Wait Spot")
    bool IsValidForLevel(const FString& CurrentLevelName) const;
};