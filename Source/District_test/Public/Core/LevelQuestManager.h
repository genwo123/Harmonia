// LevelQuestManager.h - ���� ����
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "LevelQuestManager.generated.h"

// ���� ���� ����ü
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

    FLevelInfo()
    {
        LevelName = "";
        PrerequisiteLevel = "";
        LumiDialogueID = "";
    }
};

UCLASS(BlueprintType)
class DISTRICT_TEST_API ALevelQuestManager : public AActor
{
    GENERATED_BODY()

public:
    ALevelQuestManager();

    // ������ ���̺�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    UDataTable* LevelDataTable;

    // ���� ����
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString CurrentLevel;

    // �Ϸ�� ������
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedLevels;

    // === �ٽ� ��� ===
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
};