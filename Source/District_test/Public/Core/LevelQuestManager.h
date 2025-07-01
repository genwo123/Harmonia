// LevelQuestManager.h - 간단 버전
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "LevelQuestManager.generated.h"

// 레벨 정보 구조체
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

    // 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    UDataTable* LevelDataTable;

    // 현재 레벨
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString CurrentLevel;

    // 완료된 레벨들
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedLevels;

    // === 핵심 기능 ===
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