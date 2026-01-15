// Core/LevelQuestManager.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "LevelQuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestUpdated, const FString&, NewLevelID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSubStepCompleted, int32, StepIndex, FString, DialogueID, FString, WaitSpotID);

USTRUCT(BlueprintType)
struct FLevelInfo : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PrerequisiteLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LumiDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MainObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SubSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Dialogue")
    TArray<FString> SubStepDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|AI")
    TArray<FString> SubStepWaitSpotIDs;

    FLevelInfo()
    {
        LevelName = "";
        PrerequisiteLevel = "";
        LumiDialogueID = "";
        Description = "";
        MainObjective = "";
        SubSteps = {};
        SubStepDialogueIDs = {};
        SubStepWaitSpotIDs = {};
    }
};

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API ALevelQuestManager : public AActor
{
    GENERATED_BODY()

public:
    ALevelQuestManager();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    UDataTable* LevelDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoDetectLevel = true;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString CurrentLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedLevels;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<bool> SubStepCompletionStatus;

    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestUpdated OnQuestUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnSubStepCompleted OnSubStepCompleted;

    UFUNCTION(BlueprintCallable)
    void StartLevel(const FString& LevelID);

    UFUNCTION(BlueprintCallable)
    void CompleteCurrentLevel();

    UFUNCTION(BlueprintCallable)
    bool IsLevelCompleted(const FString& LevelID);

    UFUNCTION(BlueprintCallable)
    bool CanStartLevel(const FString& LevelID);

    UFUNCTION(BlueprintCallable)
    FString GetCurrentLevelDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RefreshForCurrentLevel();

    UFUNCTION(BlueprintCallable)
    FString GetCurrentMainObjective();

    UFUNCTION(BlueprintCallable)
    bool HasSubSteps();

    UFUNCTION(BlueprintCallable)
    FString GetCurrentLevelName();

    UFUNCTION(BlueprintCallable)
    bool IsCurrentLevelCompleted();

    UFUNCTION(BlueprintCallable)
    void CompleteSubStep(int32 StepIndex);

    UFUNCTION(BlueprintCallable)
    bool IsSubStepCompleted(int32 StepIndex);

    UFUNCTION(BlueprintCallable)
    TArray<bool> GetAllSubStepStatus();

    UFUNCTION(BlueprintCallable)
    int32 GetSubStepCount();

    UFUNCTION(BlueprintCallable)
    TArray<FString> GetAllSubStepTexts();

    UFUNCTION(BlueprintCallable)
    int32 GetCurrentSubStep();

    UFUNCTION(BlueprintCallable, Category = "Quest|Dialogue")
    FString GetDialogueIDForSubStep(int32 StepIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest|AI")
    FString GetWaitSpotIDForSubStep(int32 StepIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest|Dialogue")
    FString GetCurrentUnlockedDialogueID();

    void SaveQuestProgress();
    void LoadQuestProgress();

private:
    FString LastLoadedLevel;
};