#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameProgressSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FGameProgressSaveData
{
    GENERATED_BODY()

    // 퍼즐 완료 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TArray<FString> CompletedPuzzles; // ["Puzzle_Tutorial_Laser", "Puzzle_Level1_Mirror"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TMap<FString, float> PuzzleBestTimes; // 퍼즐별 최고 기록

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Progress")
    TMap<FString, int32> PuzzleAttemptCounts; // 퍼즐별 시도 횟수

    // 패스워드 해결 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Password Progress")
    TArray<FString> SolvedPasswords; // ["Password_Door_Red", "Password_Chest_Tutorial"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Password Progress")
    TMap<FString, int32> PasswordAttemptCounts; // 패스워드별 시도 횟수

    // 레벨별 진행도 (LevelName -> Progress 0.0~1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TMap<FString, float> LevelProgress; // {"Level_Main_1": 1.0, "Level_Main_2": 0.6}

    // 스테이지 클리어 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TArray<FString> ClearedLevels; // ["Level_Main_1", "Level_Tutorial"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progress")
    TMap<FString, FDateTime> LevelClearTimes; // 레벨별 클리어 시간

    // 게임 이벤트 플래그들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Events")
    TMap<FString, bool> EventFlags; // {"Door_Red_Opened": true, "First_Puzzle_Tutorial": true}

    // 히든 요소 발견
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Content")
    TArray<FString> FoundSecrets; // ["Secret_Room_1", "Easter_Egg_Dev"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Content")
    TArray<FString> UnlockedAchievements; // 업적 시스템 (나중에 추가 시)

    // 미니게임 진행 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigames")
    TMap<FString, bool> MinigameCompletions; // {"StrokeGame_Level1": true}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigames")
    TMap<FString, int32> MinigameHighScores; // 미니게임 최고 점수

    // 체크포인트 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
    FString LastCheckpoint; // 마지막 체크포인트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
    FDateTime LastCheckpointTime;

    // 생성자
    FGameProgressSaveData()
    {
        LastCheckpoint = TEXT("");
    }
};