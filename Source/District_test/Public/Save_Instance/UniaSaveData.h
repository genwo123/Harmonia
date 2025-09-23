#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UniaSaveData.generated.h"

// 우니아 상태 열거형
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

    // 위치 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    FVector UniaLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    FRotator UniaRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    FString CurrentLevel;

    // 레벨별 우니아 스폰 위치 (레벨 전환 시 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia Location")
    TMap<FString, FVector> LevelSpawnPositions; // {"Level_Main_1": Vector(...)}

    // 상태 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    EUniaSaveState CurrentState = EUniaSaveState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    bool bIsFollowingPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    bool bCanInteract = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    float FollowDistance = 200.0f;

    // 대화 시스템
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FString CurrentDialogueID; // 현재 활성화된 대화 ID

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FString> CompletedDialogues; // 완료한 대화들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, int32> DialogueCounters; // 대화별 실행 횟수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<FString, FString> DialogueChoices; // 대화 선택지 결과 저장

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FString> AvailableDialogues; // 현재 사용 가능한 대화들

    // 스토리 대화 vs 잡담/힌트 대화 구분
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Types")
    TArray<FString> StoryDialogues; // 메인 스토리 대화

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Types")
    TArray<FString> HintDialogues; // 힌트/잡담 대화

    // 퀘스트 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FString CurrentQuestPhase = TEXT("Prologue"); // "Tutorial", "Chapter1", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    FString CurrentQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TMap<FString, bool> QuestFlags; // 퀘스트 관련 플래그들

    // 우니아 행동 패턴 (AI 확장 예정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString CurrentBehaviorPattern = TEXT("Default"); // "Exploring", "Guiding", "Resting"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LastInteractionTime = 0.0f; // 마지막 상호작용 시간

    // 우니아 감정/관계도 (나중에 확장 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float PlayerRelationship = 50.0f; // 플레이어와의 관계도 (0-100)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    TMap<FString, int32> InteractionHistory; // 상호작용 이력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unia State")
    bool bCanFollow = false;

    FUniaSaveData()
    {
        UniaLocation = FVector::ZeroVector;
        UniaRotation = FRotator::ZeroRotator;
        CurrentLevel = TEXT("Level_Main_1");
        CurrentState = EUniaSaveState::Idle;
        bIsFollowingPlayer = false;
        bCanFollow = false;  // 이 줄 추가
        bCanInteract = true;
        FollowDistance = 200.0f;
        CurrentQuestPhase = TEXT("Prologue");
        CurrentBehaviorPattern = TEXT("Default");
        LastInteractionTime = 0.0f;
        PlayerRelationship = 50.0f;
    }
};