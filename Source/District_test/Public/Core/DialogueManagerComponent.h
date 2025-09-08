// DialogueManagerComponent.h - 통합 버전
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "DialogueManagerComponent.generated.h"

UENUM(BlueprintType)
enum class EDialogueType : uint8
{
    Monologue   UMETA(DisplayName = "Monologue"),
    Dialogue    UMETA(DisplayName = "Dialogue"),
};

UENUM(BlueprintType)
enum class ESpeakerType : uint8
{
    Noah        UMETA(DisplayName = "Noah"),
    Unia        UMETA(DisplayName = "Unia"),
    System      UMETA(DisplayName = "System")
};

UENUM(BlueprintType)
enum class EDialogueCategory : uint8
{
    MainStory   UMETA(DisplayName = "Main Story"),
    Macro       UMETA(DisplayName = "Macro"),
    Hint        UMETA(DisplayName = "Hint"),
    Ending      UMETA(DisplayName = "Ending")
};

USTRUCT(BlueprintType)
struct FDialogueData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    EDialogueType DialogueType = EDialogueType::Dialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    ESpeakerType Speaker = ESpeakerType::Unia;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
    EDialogueCategory Category = EDialogueCategory::MainStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
    int32 RequiredSubStep = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
    bool bIsLevelEnd = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
    bool bBlockOnIncomplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    bool bHasChoices = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> ChoiceTexts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> ChoiceTargetIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    float DisplayDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
    bool bAutoProgress = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    TArray<FString> CustomConditions;

    FDialogueData()
    {
        DialogueID = "";
        DialogueType = EDialogueType::Dialogue;
        Speaker = ESpeakerType::Unia;
        DialogueText = FText::GetEmpty();
        NextDialogueID = "";
        LevelName = "";
        Category = EDialogueCategory::MainStory;
        RequiredSubStep = -1;
        bIsLevelEnd = false;
        bBlockOnIncomplete = false;
        DisplayDuration = 0.0f;
        bHasChoices = false;
        bAutoProgress = false;
        CustomConditions = {};
    }
};

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Settings")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsInDialogue = false;

    UPROPERTY(BlueprintReadWrite, Category = "Dialogue State")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueProgressed OnDialogueProgressed;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ProgressDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetCurrentChoices();

    UFUNCTION(BlueprintCallable, Category = "Level Dialogue")
    bool CanProgressToDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Level Dialogue")
    FString FindDialogueForCurrentLevel();

    UFUNCTION(BlueprintCallable, Category = "Level Dialogue")
    FString GetMacroDialogue(const FString& LevelName, int32 CurrentSubStep);

    UFUNCTION(BlueprintCallable, Category = "Level Dialogue")
    TArray<FString> GetDialoguesForLevel(const FString& LevelName, EDialogueCategory Category = EDialogueCategory::MainStory);

    UFUNCTION(BlueprintCallable, Category = "Level Dialogue")
    bool IsSubStepCompleted(int32 SubStepIndex);

    UFUNCTION(BlueprintCallable, Category = "Level Dialogue")
    int32 GetCurrentSubStep();

    UFUNCTION(BlueprintCallable, Category = "Level Dialogue")
    FString GetCurrentLevelName();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString FindDialogueForQuest(const FString& QuestState);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayMonologue(const FString& MonologueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool CheckAllConditions(const FDialogueData& DialogueData);

protected:
    FDialogueData* GetDialogueData(const FString& DialogueID);
    void ProcessDialogue(const FDialogueData& DialogueData);

    class ALevelQuestManager* FindLevelQuestManager();
    bool ValidateSubStepRequirement(const FDialogueData& DialogueData);

private:
    FDialogueData CurrentDialogue;

    UPROPERTY()
    class ALevelQuestManager* CachedQuestManager;
};