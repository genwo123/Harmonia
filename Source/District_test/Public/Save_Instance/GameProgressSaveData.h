#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameProgressSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FGameProgressSaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    FString CurrentQuestLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    TArray<FString> CompletedQuestLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progress")
    TArray<bool> CurrentSubStepStatus;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TArray<FString> CompletedPuzzles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TMap<FString, float> PuzzleBestTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TMap<FString, int32> PuzzleAttemptCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Password Progress")
    TArray<FString> SolvedPasswords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Password Progress")
    TMap<FString, int32> PasswordAttemptCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TMap<FString, float> LevelProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TArray<FString> ClearedLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TMap<FString, FDateTime> LevelClearTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events")
    TMap<FString, bool> EventFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Content")
    TArray<FString> FoundSecrets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Content")
    TArray<FString> UnlockedAchievements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigames")
    TMap<FString, bool> MinigameCompletions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigames")
    TMap<FString, int32> MinigameHighScores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
    FString LastCheckpoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
    FDateTime LastCheckpointTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    TMap<FString, int32> LevelStoryStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    FString PendingTriggerDialogue;

    FGameProgressSaveData()
    {
        LastCheckpoint = TEXT("");
        PendingTriggerDialogue = TEXT("");
    }
};