#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UniaSaveData.generated.h"

// ��Ͼ� ���� ������
UENUM(BlueprintType)
enum class EUniaSaveState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Following       UMETA(DisplayName = "Following Player"),
    WaitingAtPoint  UMETA(DisplayName = "Waiting at Point"),
    InDialogue      UMETA(DisplayName = "In Dialogue"),
    Disabled        UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FUniaSaveData
{
    GENERATED_BODY()

    // ��ġ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    FVector UniaLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    FRotator UniaRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    FString CurrentLevel;

    // ������ ��Ͼ� ���� ��ġ (���� ��ȯ �� ���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    TMap<FString, FVector> LevelSpawnPositions; // {"Level_Main_1": Vector(...)}

    // ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    EUniaSaveState CurrentState = EUniaSaveState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    bool bIsFollowingPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    bool bCanInteract = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    float FollowDistance = 200.0f;

    // ��ȭ �ý���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentDialogueID; // ���� Ȱ��ȭ�� ��ȭ ID

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FString> CompletedDialogues; // �Ϸ��� ��ȭ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, int32> DialogueCounters; // ��ȭ�� ���� Ƚ��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, FString> DialogueChoices; // ��ȭ ������ ��� ����

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FString> AvailableDialogues; // ���� ��� ������ ��ȭ��

    // ���丮 ��ȭ vs ���/��Ʈ ��ȭ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Types")
    TArray<FString> StoryDialogues; // ���� ���丮 ��ȭ

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Types")
    TArray<FString> HintDialogues; // ��Ʈ/��� ��ȭ

    // ����Ʈ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FString CurrentQuestPhase = TEXT("Prologue"); // "Tutorial", "Chapter1", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FString CurrentQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TMap<FString, bool> QuestFlags; // ����Ʈ ���� �÷��׵�

    // ��Ͼ� �ൿ ���� (AI Ȯ�� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString CurrentBehaviorPattern = TEXT("Default"); // "Exploring", "Guiding", "Resting"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LastInteractionTime = 0.0f; // ������ ��ȣ�ۿ� �ð�

    // ��Ͼ� ����/���赵 (���߿� Ȯ�� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float PlayerRelationship = 50.0f; // �÷��̾���� ���赵 (0-100)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    TMap<FString, int32> InteractionHistory; // ��ȣ�ۿ� �̷�

    // ������
    FUniaSaveData()
    {
        UniaLocation = FVector::ZeroVector;
        UniaRotation = FRotator::ZeroRotator;
        CurrentLevel = TEXT("Level_Main_1");
        CurrentState = EUniaSaveState::Idle;
        bIsFollowingPlayer = false;
        bCanInteract = true;
        FollowDistance = 200.0f;
        CurrentQuestPhase = TEXT("Prologue");
        CurrentBehaviorPattern = TEXT("Default");
        LastInteractionTime = 0.0f;
        PlayerRelationship = 50.0f;
    }
};