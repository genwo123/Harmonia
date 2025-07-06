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

    // === �⺻ ����Ʈ ���� ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bIsQuestRelated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings",
        meta = (EditCondition = "bIsQuestRelated"))
    int32 QuestStepIndex = 0;

    // === Quest Manager ���� ���� (���۾� ����) ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings",
        meta = (EditCondition = "bIsQuestRelated"))
    ALevelQuestManager* QuestManagerRef;

    // === ������ ���� ��� (�б� ����) ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info",
        meta = (MultiLine = "true"))
    FString CurrentLevelInfo = "���� ������ �ҷ����� ��...";

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info")
    TArray<FString> CurrentSubSteps;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Info")
    FString SelectedStepPreview = "";

    // === ������ ���̺� ���� ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    UDataTable* QuestDataTable;

public:
    // ����Ʈ �Ϸ� �Լ�
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuestStep();

    // ����Ʈ ���� ���� Ȯ��
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestRelated() const { return bIsQuestRelated; }

    // ���� ������ �̹� �Ϸ�Ǿ����� Ȯ��
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsCurrentStepCompleted();

    // �����Ϳ��� ���� ������Ʈ (������ ����)
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    void UpdateQuestInfo();
#endif

private:
    // ���� ������ ĳ��
    FString CurrentLevelName;

    // Quest Manager ĳ��
    UPROPERTY()
    class ALevelQuestManager* QuestManager;

    // Quest Manager ã��
    class ALevelQuestManager* GetQuestManager();
};