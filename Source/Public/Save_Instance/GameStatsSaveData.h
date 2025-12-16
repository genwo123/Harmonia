#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameStatsSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FGameStatsSaveData
{
    GENERATED_BODY()

    // 시간 관련 통계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    float TotalPlayTime = 0.0f; // 총 플레이 시간 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    float CurrentSessionTime = 0.0f; // 현재 세션 플레이 시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    FDateTime LastPlayTime; // 마지막 플레이 시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    FDateTime FirstPlayTime; // 처음 플레이한 시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    int32 PlaySessionCount = 0; // 플레이 세션 횟수

    // 퍼즐/게임플레이 통계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 PuzzlesSolved = 0; // 해결한 퍼즐 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 PuzzlesAttempted = 0; // 시도한 퍼즐 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 PasswordsSolved = 0; // 해결한 패스워드 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 MinigamesCompleted = 0; // 완료한 미니게임 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    float AveragePuzzleTime = 0.0f; // 평균 퍼즐 해결 시간

    // 수집 통계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    int32 NotesCollected = 0; // 수집한 노트 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    int32 ItemsCollected = 0; // 수집한 아이템 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    int32 SecretsFound = 0; // 발견한 비밀 요소 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    float CollectionPercentage = 0.0f; // 전체 수집률

    // 대화/스토리 통계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    int32 DialoguesCompleted = 0; // 완료한 대화 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    int32 StoryDialogues = 0; // 메인 스토리 대화 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    int32 HintDialogues = 0; // 힌트 대화 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    float StoryProgress = 0.0f; // 스토리 진행률 (0-1)

    // 실패/재시도 통계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure Stats")
    int32 DeathCount = 0; // 사망 횟수 (있다면)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure Stats")
    int32 PuzzleResets = 0; // 퍼즐 리셋 횟수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure Stats")
    int32 HintUsages = 0; // 힌트 사용 횟수

    // 저장 관련 통계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Stats")
    int32 ManualSaveCount = 0; // 수동 저장 횟수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Stats")
    int32 AutoSaveCount = 0; // 자동 저장 횟수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Stats")
    int32 LoadCount = 0; // 로드 횟수

    // 레벨/진행도 통계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Stats")
    int32 LevelsCompleted = 0; // 완료한 레벨 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Stats")
    int32 CheckpointsReached = 0; // 도달한 체크포인트 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Stats")
    float GameProgress = 0.0f; // 전체 게임 진행률 (0-1)

    // 성과/업적 관련 (나중에 확장)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement Stats")
    int32 AchievementsUnlocked = 0; // 해금한 업적 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement Stats")
    TArray<FString> UnlockedAchievements; // 해금한 업적 목록

    // 생성자
    FGameStatsSaveData()
    {
        TotalPlayTime = 0.0f;
        CurrentSessionTime = 0.0f;
        PlaySessionCount = 0;
        PuzzlesSolved = 0;
        PuzzlesAttempted = 0;
        PasswordsSolved = 0;
        MinigamesCompleted = 0;
        AveragePuzzleTime = 0.0f;
        NotesCollected = 0;
        ItemsCollected = 0;
        SecretsFound = 0;
        CollectionPercentage = 0.0f;
        DialoguesCompleted = 0;
        StoryDialogues = 0;
        HintDialogues = 0;
        StoryProgress = 0.0f;
        DeathCount = 0;
        PuzzleResets = 0;
        HintUsages = 0;
        ManualSaveCount = 0;
        AutoSaveCount = 0;
        LoadCount = 0;
        LevelsCompleted = 0;
        CheckpointsReached = 0;
        GameProgress = 0.0f;
        AchievementsUnlocked = 0;
    }
};