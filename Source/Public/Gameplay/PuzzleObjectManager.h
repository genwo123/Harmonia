#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleObjectManager.generated.h"

UCLASS(Blueprintable)
class DISTRICT_TEST_API APuzzleObjectManager : public AActor
{
    GENERATED_BODY()

public:
    APuzzleObjectManager();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Manager")
    void ActivateAllPuzzleObjects();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Manager")
    void DeactivateAllPuzzleObjects();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<AActor*> ManagedObjects;

    void FindAllPuzzleObjects();
    void SetObjectsActive(bool bActive);
};