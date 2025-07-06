// LevelQuestManager.h - 최소한 수정 버전
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

    // 메인 퀘스트 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    // 메인 목표 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MainObjective;

    // 소목표 배열 (순차적으로 진행)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SubSteps;

    FLevelInfo()
    {
        LevelName = "";
        PrerequisiteLevel = "";
        LumiDialogueID = "";
        Description = "";
        MainObjective = "";
        SubSteps = {};  // 빈 배열로 초기화
    }
};

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API ALevelQuestManager : public AActor
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    ALevelQuestManager();

    // 데이터 테이블 - 이제 에디터에서 설정 가능
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    UDataTable* LevelDataTable;

    // 자동 맵 감지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bAutoDetectLevel = true;

    // 현재 레벨
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString CurrentLevel;

    // 완료된 레벨들
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedLevels;

    // 각 소목표 완료 상태
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<bool> SubStepCompletionStatus;

    // === 기존 핵심 기능 ===
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

    // === 소목표 시스템 함수들 ===
    UFUNCTION(BlueprintCallable)
    FString GetCurrentMainObjective();

    UFUNCTION(BlueprintCallable)
    bool HasSubSteps();  // 현재 레벨에 소목표가 있는지

    UFUNCTION(BlueprintCallable)
    FString GetCurrentLevelName();  // 현재 레벨 이름

    UFUNCTION(BlueprintCallable)
    bool IsCurrentLevelCompleted();  // 현재 레벨 완료 여부

    // 새로운 함수들 추가
    UFUNCTION(BlueprintCallable)
    void CompleteSubStep(int32 StepIndex);  // 특정 단계 완료

    UFUNCTION(BlueprintCallable)
    bool IsSubStepCompleted(int32 StepIndex);  // 특정 단계 완료 여부

    UFUNCTION(BlueprintCallable)
    TArray<bool> GetAllSubStepStatus();  // 모든 단계 완료 상태

    UFUNCTION(BlueprintCallable)
    int32 GetSubStepCount();  // 총 소목표 개수

    UFUNCTION(BlueprintCallable)
    TArray<FString> GetAllSubStepTexts();  // 모든 소목표 텍스트
};