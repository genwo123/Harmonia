#include "Core/EnhancedQuestComponent.h"
#include "Core/LevelQuestManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

UEnhancedQuestComponent::UEnhancedQuestComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UDataTable> QuestDTObject(TEXT("/Game/Hamonia/H_DataTable/DT_LevelQuest"));
    if (QuestDTObject.Succeeded())
    {
        QuestDataTable = QuestDTObject.Object;
    }
}

void UEnhancedQuestComponent::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World)
    {
        CurrentLevelName = World->GetMapName();
        CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
    }

    GetQuestManager();
}

void UEnhancedQuestComponent::CompleteQuestStep()
{
    if (!bIsQuestRelated)
    {
        return;
    }

    if (QuestManagerRef)
    {
        QuestManagerRef->CompleteSubStep(QuestStepIndex);
        return;
    }

    if (!QuestManager)
    {
        QuestManager = GetQuestManager();
    }

    if (QuestManager)
    {
        QuestManager->CompleteSubStep(QuestStepIndex);
    }
}

void UEnhancedQuestComponent::CompleteQuestStepByIndex(int32 StepIndex)
{
    if (!bIsQuestRelated)
    {
        return;
    }

    if (QuestManagerRef)
    {
        QuestManagerRef->CompleteSubStep(StepIndex);
        return;
    }

    if (!QuestManager)
    {
        QuestManager = GetQuestManager();
    }

    if (QuestManager)
    {
        QuestManager->CompleteSubStep(StepIndex);
    }
}

bool UEnhancedQuestComponent::IsCurrentStepCompleted()
{
    if (!bIsQuestRelated)
        return false;

    if (!QuestManager)
    {
        QuestManager = GetQuestManager();
    }

    if (QuestManager)
    {
        return QuestManager->IsSubStepCompleted(QuestStepIndex);
    }

    return false;
}

void UEnhancedQuestComponent::SetQuestStepIndex(int32 NewStepIndex)
{
    QuestStepIndex = NewStepIndex;

#if WITH_EDITOR
    UpdateQuestInfo();
#endif
}

ALevelQuestManager* UEnhancedQuestComponent::GetQuestManager()
{
    if (!QuestManager)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClassWithTag(World, ALevelQuestManager::StaticClass(),
                FName("QuestManager"), FoundActors);

            if (FoundActors.Num() > 0)
            {
                QuestManager = Cast<ALevelQuestManager>(FoundActors[0]);
            }

            if (!QuestManager)
            {
                TArray<AActor*> FoundManagers;
                UGameplayStatics::GetAllActorsOfClass(World, ALevelQuestManager::StaticClass(), FoundManagers);

                if (FoundManagers.Num() > 0)
                {
                    QuestManager = Cast<ALevelQuestManager>(FoundManagers[0]);
                }
            }
        }
    }

    return QuestManager;
}

#if WITH_EDITOR
void UEnhancedQuestComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = PropertyChangedEvent.GetPropertyName();

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UEnhancedQuestComponent, bIsQuestRelated) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(UEnhancedQuestComponent, QuestStepIndex))
    {
        UpdateQuestInfo();
    }
}

void UEnhancedQuestComponent::UpdateQuestInfo()
{
    if (!bIsQuestRelated)
    {
        CurrentLevelInfo = "이 오브젝트는 퀘스트와 관련없습니다.";
        CurrentSubSteps.Empty();
        SelectedStepPreview = "";
        return;
    }

    if (!QuestDataTable)
    {
        CurrentLevelInfo = "퀘스트 데이터 테이블이 설정되지 않았습니다.";
        return;
    }

    UWorld* EditorWorld = nullptr;

    if (GetOwner())
    {
        EditorWorld = GetOwner()->GetWorld();
    }

    if (EditorWorld)
    {
        CurrentLevelName = EditorWorld->GetMapName();
        CurrentLevelName.RemoveFromStart(EditorWorld->StreamingLevelsPrefix);
    }
    else
    {
        CurrentLevelName = "Unknown";
    }

    // Row Name으로 직접 찾기 (수정된 부분)
    FName LevelRowName(*CurrentLevelName);
    FLevelInfo* LevelData = QuestDataTable->FindRow<FLevelInfo>(LevelRowName, TEXT(""));

    if (LevelData)
    {
        CurrentLevelInfo = FString::Printf(TEXT("레벨: %s\n목표: %s\n설명: %s"),
            *LevelData->LevelName,
            *LevelData->MainObjective,
            *LevelData->Description);

        CurrentSubSteps = LevelData->SubSteps;

        if (QuestStepIndex >= 0 && QuestStepIndex < CurrentSubSteps.Num())
        {
            SelectedStepPreview = FString::Printf(TEXT("스텝 %d: %s"),
                QuestStepIndex,
                *CurrentSubSteps[QuestStepIndex]);
        }
        else
        {
            SelectedStepPreview = "잘못된 스텝 인덱스입니다.";
        }
    }
    else
    {
        CurrentLevelInfo = FString::Printf(TEXT("레벨 '%s'의 퀘스트 정보를 찾을 수 없습니다."), *CurrentLevelName);
        CurrentSubSteps.Empty();
        SelectedStepPreview = "";
    }
}
#endif