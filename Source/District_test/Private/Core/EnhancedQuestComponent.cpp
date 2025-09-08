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

    static ConstructorHelpers::FObjectFinder<UDataTable> QuestDTObject(TEXT("/Game/Data/DT_LevelQuests"));
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
        CurrentLevelInfo = "�� ������Ʈ�� ����Ʈ�� ���þ����ϴ�.";
        CurrentSubSteps.Empty();
        SelectedStepPreview = "";
        return;
    }

    if (!QuestDataTable)
    {
        CurrentLevelInfo = "����Ʈ ������ ���̺��� �������� �ʾҽ��ϴ�.";
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

    // Row Name���� ���� ã�� (������ �κ�)
    FName LevelRowName(*CurrentLevelName);
    FLevelInfo* LevelData = QuestDataTable->FindRow<FLevelInfo>(LevelRowName, TEXT(""));

    if (LevelData)
    {
        CurrentLevelInfo = FString::Printf(TEXT("����: %s\n��ǥ: %s\n����: %s"),
            *LevelData->LevelName,
            *LevelData->MainObjective,
            *LevelData->Description);

        CurrentSubSteps = LevelData->SubSteps;

        if (QuestStepIndex >= 0 && QuestStepIndex < CurrentSubSteps.Num())
        {
            SelectedStepPreview = FString::Printf(TEXT("���� %d: %s"),
                QuestStepIndex,
                *CurrentSubSteps[QuestStepIndex]);
        }
        else
        {
            SelectedStepPreview = "�߸��� ���� �ε����Դϴ�.";
        }
    }
    else
    {
        CurrentLevelInfo = FString::Printf(TEXT("���� '%s'�� ����Ʈ ������ ã�� �� �����ϴ�."), *CurrentLevelName);
        CurrentSubSteps.Empty();
        SelectedStepPreview = "";
    }
}
#endif