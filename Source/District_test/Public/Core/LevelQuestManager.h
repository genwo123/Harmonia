// LevelQuestManager.h - �ּ��� ���� ����
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "LevelQuestManager.generated.h"

USTRUCT(BlueprintType)
struct FLevelInfo : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PrerequisiteLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LumiDialogueID;

    // ���� ����Ʈ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    // ���� ��ǥ �߰�
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MainObjective;

    // �Ҹ�ǥ �迭 (���������� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SubSteps;

    FLevelInfo()
    {
        LevelName = "";
        PrerequisiteLevel = "";
        LumiDialogueID = "";
        Description = "";
        MainObjective = "";
        SubSteps = {};  // �� �迭�� �ʱ�ȭ
    }
};

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API ALevelQuestManager : public AActor
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    ALevelQuestManager();

    // ������ ���̺� - ���� �����Ϳ��� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    UDataTable* LevelDataTable;

    // �ڵ� �� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoDetectLevel = true;

    // ���� ����
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString CurrentLevel;

    // �Ϸ�� ������
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedLevels;

    // �� �Ҹ�ǥ �Ϸ� ����
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<bool> SubStepCompletionStatus;

    // === ���� �ٽ� ��� ===
    UFUNCTION(BlueprintCallable)
    void StartLevel(const FString& LevelID);

    UFUNCTION(BlueprintCallable)
    void CompleteCurrentLevel();

    UFUNCTION(BlueprintCallable)
    bool IsLevelCompleted(const FString& LevelID);

    UFUNCTION(BlueprintCallable)
    bool CanStartLevel(const FString& LevelID);

    UFUNCTION(BlueprintCallable)
    FString GetCurrentLevelDialogue();

    // === �Ҹ�ǥ �ý��� �Լ��� ===
    UFUNCTION(BlueprintCallable)
    FString GetCurrentMainObjective();

    UFUNCTION(BlueprintCallable)
    bool HasSubSteps();  // ���� ������ �Ҹ�ǥ�� �ִ���

    UFUNCTION(BlueprintCallable)
    FString GetCurrentLevelName();  // ���� ���� �̸�

    UFUNCTION(BlueprintCallable)
    bool IsCurrentLevelCompleted();  // ���� ���� �Ϸ� ����

    // ���ο� �Լ��� �߰�
    UFUNCTION(BlueprintCallable)
    void CompleteSubStep(int32 StepIndex);  // Ư�� �ܰ� �Ϸ�

    UFUNCTION(BlueprintCallable)
    bool IsSubStepCompleted(int32 StepIndex);  // Ư�� �ܰ� �Ϸ� ����

    UFUNCTION(BlueprintCallable)
    TArray<bool> GetAllSubStepStatus();  // ��� �ܰ� �Ϸ� ����

    UFUNCTION(BlueprintCallable)
    int32 GetSubStepCount();  // �� �Ҹ�ǥ ����

    UFUNCTION(BlueprintCallable)
    TArray<FString> GetAllSubStepTexts();  // ��� �Ҹ�ǥ �ؽ�Ʈ
};