#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "UniaAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;

UCLASS()
class DISTRICT_TEST_API AUniaAIController : public AAIController
{
    GENERATED_BODY()

public:
    AUniaAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Unia AI")
    void StartFollowingPlayer();

    UFUNCTION(BlueprintCallable, Category = "Unia AI")
    void StopFollowing();

    UFUNCTION(BlueprintCallable, Category = "Unia AI")
    void MoveToTargetLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Unia AI")
    void SetDialogueMode(bool bInDialogue);

    UFUNCTION(BlueprintCallable, Category = "Unia AI")
    bool CheckTeleportDistance();

    UFUNCTION(BlueprintCallable, Category = "Unia AI")
    void TeleportToPlayer();

    UFUNCTION(BlueprintCallable, Category = "Unia AI")
    void SetTeleportCooldown(float CooldownTime);

    UFUNCTION(BlueprintPure, Category = "Unia AI")
    bool IsFollowingPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Unia AI")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Unia AI")
    bool CanTeleport() const;

protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, Category = "AI")
    UBlackboardData* BlackboardAsset;

private:
    void SetBlackboardValues();

    bool bCanTeleportNow = true;
    FTimerHandle TeleportCooldownTimer;

    void OnTeleportCooldownComplete();
};