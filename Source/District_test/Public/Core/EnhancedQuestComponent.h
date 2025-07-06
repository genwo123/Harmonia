// EnhancedQuestComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "EnhancedQuestComponent.generated.h"

// Forward Declaration
class ALevelQuestManager;
struct FLevelInfo;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UEnhancedQuestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnhancedQuestComponent();

protected:
    virtual void BeginPlay() override;

    // === 기본 퀘스트 설정 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bIsQuestRelated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings",
        meta = (EditCondition = "bIsQuestRelated"))
    int32 QuestStepIndex = 0;

    // === Quest Manager 직접 참조 (수작업 연결) ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings",
        meta = (EditCondition = "bIsQuestRelated"))
    ALevelQuestManager* QuestManagerRef;

    // === 에디터 편의 기능 (읽기 전용) ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info",
        meta = (MultiLine = "true"))
    FString CurrentLevelInfo = "레벨 정보를 불러오는 중...";

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info")
    TArray<FString> CurrentSubSteps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info")
    FString SelectedStepPreview = "";

    // === 데이터 테이블 참조 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    UDataTable* QuestDataTable;

public:
    // 퀘스트 완료 함수
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuestStep();

    // 퀘스트 관련 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestRelated() const { return bIsQuestRelated; }

    // 현재 스텝이 이미 완료되었는지 확인
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsCurrentStepCompleted();

    // 에디터에서 정보 업데이트 (에디터 전용)
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    void UpdateQuestInfo();
#endif

private:
    // 현재 레벨명 캐시
    FString CurrentLevelName;

    // Quest Manager 캐시
    UPROPERTY()
    class ALevelQuestManager* QuestManager;

    // Quest Manager 찾기
    class ALevelQuestManager* GetQuestManager();
};