#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "EnhancedQuestComponent.generated.h"

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bIsQuestRelated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings", meta = (EditCondition = "bIsQuestRelated"))
    int32 QuestStepIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings", meta = (EditCondition = "bIsQuestRelated"))
    ALevelQuestManager* QuestManagerRef;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info", meta = (MultiLine = "true"))
    FString CurrentLevelInfo = "레벨 정보를 불러오는 중...";

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info")
    TArray<FString> CurrentSubSteps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info")
    FString SelectedStepPreview = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    UDataTable* QuestDataTable;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuestStep();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuestStepByIndex(int32 StepIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestRelated() const { return bIsQuestRelated; }

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsCurrentStepCompleted();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetQuestStepIndex(int32 NewStepIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetQuestStepIndex() const { return QuestStepIndex; }

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    void UpdateQuestInfo();
#endif

private:
    FString CurrentLevelName;

    UPROPERTY()
    class ALevelQuestManager* QuestManager;

    class ALevelQuestManager* GetQuestManager();
};