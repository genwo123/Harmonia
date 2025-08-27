#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameStatsSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FGameStatsSaveData
{
    GENERATED_BODY()

    // �ð� ���� ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    float TotalPlayTime = 0.0f; // �� �÷��� �ð� (��)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    float CurrentSessionTime = 0.0f; // ���� ���� �÷��� �ð�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    FDateTime LastPlayTime; // ������ �÷��� �ð�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    FDateTime FirstPlayTime; // ó�� �÷����� �ð�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Stats")
    int32 PlaySessionCount = 0; // �÷��� ���� Ƚ��

    // ����/�����÷��� ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 PuzzlesSolved = 0; // �ذ��� ���� ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 PuzzlesAttempted = 0; // �õ��� ���� ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 PasswordsSolved = 0; // �ذ��� �н����� ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    int32 MinigamesCompleted = 0; // �Ϸ��� �̴ϰ��� ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Stats")
    float AveragePuzzleTime = 0.0f; // ��� ���� �ذ� �ð�

    // ���� ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    int32 NotesCollected = 0; // ������ ��Ʈ ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    int32 ItemsCollected = 0; // ������ ������ ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    int32 SecretsFound = 0; // �߰��� ��� ��� ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection Stats")
    float CollectionPercentage = 0.0f; // ��ü ������

    // ��ȭ/���丮 ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    int32 DialoguesCompleted = 0; // �Ϸ��� ��ȭ ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    int32 StoryDialogues = 0; // ���� ���丮 ��ȭ ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    int32 HintDialogues = 0; // ��Ʈ ��ȭ ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Stats")
    float StoryProgress = 0.0f; // ���丮 ����� (0-1)

    // ����/��õ� ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure Stats")
    int32 DeathCount = 0; // ��� Ƚ�� (�ִٸ�)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure Stats")
    int32 PuzzleResets = 0; // ���� ���� Ƚ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure Stats")
    int32 HintUsages = 0; // ��Ʈ ��� Ƚ��

    // ���� ���� ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Stats")
    int32 ManualSaveCount = 0; // ���� ���� Ƚ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Stats")
    int32 AutoSaveCount = 0; // �ڵ� ���� Ƚ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Stats")
    int32 LoadCount = 0; // �ε� Ƚ��

    // ����/���൵ ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Stats")
    int32 LevelsCompleted = 0; // �Ϸ��� ���� ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Stats")
    int32 CheckpointsReached = 0; // ������ üũ����Ʈ ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Stats")
    float GameProgress = 0.0f; // ��ü ���� ����� (0-1)

    // ����/���� ���� (���߿� Ȯ��)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement Stats")
    int32 AchievementsUnlocked = 0; // �ر��� ���� ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement Stats")
    TArray<FString> UnlockedAchievements; // �ر��� ���� ���

    // ������
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