#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameProgressSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FGameProgressSaveData
{
    GENERATED_BODY()

    // ���� �Ϸ� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TArray<FString> CompletedPuzzles; // ["Puzzle_Tutorial_Laser", "Puzzle_Level1_Mirror"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TMap<FString, float> PuzzleBestTimes; // ���� �ְ� ���

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TMap<FString, int32> PuzzleAttemptCounts; // ���� �õ� Ƚ��

    // �н����� �ذ� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Password Progress")
    TArray<FString> SolvedPasswords; // ["Password_Door_Red", "Password_Chest_Tutorial"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Password Progress")
    TMap<FString, int32> PasswordAttemptCounts; // �н����庰 �õ� Ƚ��

    // ������ ���൵ (LevelName -> Progress 0.0~1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TMap<FString, float> LevelProgress; // {"Level_Main_1": 1.0, "Level_Main_2": 0.6}

    // �������� Ŭ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TArray<FString> ClearedLevels; // ["Level_Main_1", "Level_Tutorial"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TMap<FString, FDateTime> LevelClearTimes; // ������ Ŭ���� �ð�

    // ���� �̺�Ʈ �÷��׵�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events")
    TMap<FString, bool> EventFlags; // {"Door_Red_Opened": true, "First_Puzzle_Tutorial": true}

    // ���� ��� �߰�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Content")
    TArray<FString> FoundSecrets; // ["Secret_Room_1", "Easter_Egg_Dev"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Content")
    TArray<FString> UnlockedAchievements; // ���� �ý��� (���߿� �߰� ��)

    // �̴ϰ��� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigames")
    TMap<FString, bool> MinigameCompletions; // {"StrokeGame_Level1": true}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigames")
    TMap<FString, int32> MinigameHighScores; // �̴ϰ��� �ְ� ����

    // üũ����Ʈ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
    FString LastCheckpoint; // ������ üũ����Ʈ

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
    FDateTime LastCheckpointTime;

    // ������
    FGameProgressSaveData()
    {
        LastCheckpoint = TEXT("");
    }
};