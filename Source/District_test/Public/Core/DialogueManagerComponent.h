// DialogueManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "DialogueManagerComponent.generated.h"

// ��ȭ Ÿ�� enum (���� �״��)
UENUM(BlueprintType)
enum class EDialogueType : uint8
{
    Monologue   UMETA(DisplayName = "Monologue"),   // ����, �����̼�
    Dialogue    UMETA(DisplayName = "Dialogue"),    // ��ȭ
};

// ĳ���� Ÿ�� enum (�̸� ����)
UENUM(BlueprintType)
enum class ESpeakerType : uint8
{
    Noah        UMETA(DisplayName = "Noah"),        // �÷��̾�(���� Player)
    Unia        UMETA(DisplayName = "Unia"),        // ��Ͼ�(���� Lumi)
    System      UMETA(DisplayName = "System")       // �ý���/�����̼�
};

// ��ȭ ������ ����ü
USTRUCT(BlueprintType)
struct FDialogueData : public FTableRowBase
{
    GENERATED_BODY()

    // ��ȭ ID (���� �ĺ���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    // ��ȭ Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueType DialogueType = EDialogueType::Dialogue;

    // ȭ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ESpeakerType Speaker = ESpeakerType::Unia;

    // ��ȭ �ؽ�Ʈ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    // ���� ��ȭ ID (��������� ��ȭ ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    // ǥ�� �ð� (�����, 0�̸� ���� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration = 0.0f;

    // ������ �ý���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    bool bHasChoices = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> ChoiceTexts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> ChoiceTargetIDs;

    // ��ȭ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    bool bAutoProgress = false;

    // ����Ʈ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RequiredQuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RequiredQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestEventID;

    // ������ ����
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

// ��ȭ �̺�Ʈ ��������Ʈ
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
    // ��ȭ ������ ���̺�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Settings")
    UDataTable* DialogueDataTable;

    // ���� ��ȭ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsInDialogue = false;

    UPROPERTY(BlueprintReadWrite, Category = "Dialogue State")
    FString CurrentDialogueID;

    // ��ȭ �̺�Ʈ��
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueProgressed OnDialogueProgressed;

    // �ֿ� ��ȭ �Լ���
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ProgressDialogue();

    // ������ ����
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetCurrentChoices();

    // ����Ʈ ���¿� ���� ��ȭ ã��
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString FindDialogueForQuest(const FString& QuestState);

    // ���� ���
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayMonologue(const FString& MonologueID);

    // ���� üũ
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckQuestCondition(const FString& RequiredQuestState);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckItemCondition(const FString& RequiredItemID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckAllConditions(const FDialogueData& DialogueData);

protected:
    // ���� �Լ���
    FDialogueData* GetDialogueData(const FString& DialogueID);
    void ProcessDialogue(const FDialogueData& DialogueData);

private:
    // ���� ��ȭ ������
    FDialogueData CurrentDialogue;
};