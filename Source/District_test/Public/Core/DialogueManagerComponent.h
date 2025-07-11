// DialogueManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "DialogueManagerComponent.generated.h"

// 대화 타입 enum (기존 그대로)
UENUM(BlueprintType)
enum class EDialogueType : uint8
{
    Monologue   UMETA(DisplayName = "Monologue"),   // 독백, 나레이션
    Dialogue    UMETA(DisplayName = "Dialogue"),    // 대화
};

// 캐릭터 타입 enum (이름 변경)
UENUM(BlueprintType)
enum class ESpeakerType : uint8
{
    Noah        UMETA(DisplayName = "Noah"),        // 플레이어(기존 Player)
    Unia        UMETA(DisplayName = "Unia"),        // 우니아(기존 Lumi)
    System      UMETA(DisplayName = "System")       // 시스템/나레이션
};

// 대화 데이터 구조체
USTRUCT(BlueprintType)
struct FDialogueData : public FTableRowBase
{
    GENERATED_BODY()

    // 대화 ID (고유 식별자)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    // 대화 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueType DialogueType = EDialogueType::Dialogue;

    // 화자
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESpeakerType Speaker = ESpeakerType::Unia;

    // 대화 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    // 다음 대화 ID (비어있으면 대화 종료)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    // 표시 시간 (독백용, 0이면 수동 진행)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration = 0.0f;

    // 선택지 시스템
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    bool bHasChoices = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> ChoiceTexts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> ChoiceTargetIDs;

    // 대화 제어
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    bool bAutoProgress = false;

    // 퀘스트 연동
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RequiredQuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RequiredQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestEventID;

    // 아이템 조건
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    bool bRequireItem = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FString RequiredItemID;

    FDialogueData()
    {
        DialogueID = "";
        DialogueType = EDialogueType::Dialogue;
        Speaker = ESpeakerType::Unia;
        DialogueText = FText::GetEmpty();
        NextDialogueID = "";
        DisplayDuration = 0.0f;
        bHasChoices = false;
        bAutoProgress = false;
        RequiredQuestState = "";
        RequiredQuestID = "";
        QuestEventID = "";
        bRequireItem = false;
        RequiredItemID = "";
    }
};

// 대화 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDialogueStarted, ESpeakerType, Speaker, FText, DialogueText, EDialogueType, Type, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDialogueProgressed, ESpeakerType, Speaker, FText, DialogueText, EDialogueType, Type, float, Duration);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UDialogueManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueManagerComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 대화 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Settings")
    UDataTable* DialogueDataTable;

    // 현재 대화 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsInDialogue = false;

    UPROPERTY(BlueprintReadWrite, Category = "Dialogue State")
    FString CurrentDialogueID;

    // 대화 이벤트들
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueProgressed OnDialogueProgressed;

    // 주요 대화 함수들
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ProgressDialogue();

    // 선택지 관련
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetCurrentChoices();

    // 퀘스트 상태에 따른 대화 찾기
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString FindDialogueForQuest(const FString& QuestState);

    // 독백 재생
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayMonologue(const FString& MonologueID);

    // 조건 체크
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckQuestCondition(const FString& RequiredQuestState);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckItemCondition(const FString& RequiredItemID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckAllConditions(const FDialogueData& DialogueData);

protected:
    // 내부 함수들
    FDialogueData* GetDialogueData(const FString& DialogueID);
    void ProcessDialogue(const FDialogueData& DialogueData);

private:
    // 현재 대화 데이터
    FDialogueData CurrentDialogue;
};