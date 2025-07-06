// EnhancedQuestComponent.cpp
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

    // �⺻ ������ ���̺� ��� ���� (������Ʈ�� �°� ����)
    static ConstructorHelpers::FObjectFinder<UDataTable> QuestDTObject(TEXT("/Game/Data/DT_LevelQuests"));
    if (QuestDTObject.Succeeded())
    {
        QuestDataTable = QuestDTObject.Object;
    }
}

void UEnhancedQuestComponent::BeginPlay()
{
    Super::BeginPlay();

    // ���� ������ ��������
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentLevelName = World->GetMapName();
        CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
    }

    // Quest Manager ã��
    GetQuestManager();
}

void UEnhancedQuestComponent::CompleteQuestStep()
{
    // ����Ʈ ������ �ƴϸ� �������� ����
    if (!bIsQuestRelated)
    {
        UE_LOG(LogTemp, Warning, TEXT("Object is not quest related. Skipping quest completion."));
        return;
    }

    // ���� ������ Quest Manager ���
    if (QuestManagerRef)
    {
        QuestManagerRef->CompleteSubStep(QuestStepIndex);
        UE_LOG(LogTemp, Warning, TEXT("Quest Step %d completed!"), QuestStepIndex);
        return;
    }

    // ���: �ڵ����� ã��
    if (!QuestManager)
    {
        QuestManager = GetQuestManager();
    }

    if (QuestManager)
    {
        QuestManager->CompleteSubStep(QuestStepIndex);
        UE_LOG(LogTemp, Warning, TEXT("Quest Step %d completed! (Found automatically)"), QuestStepIndex);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Quest Manager not found! Please set QuestManagerRef manually."));
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

ALevelQuestManager* UEnhancedQuestComponent::GetQuestManager()
{
    if (!QuestManager)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // ��� 1: GameMode�� ���� Quest Manager ã�� (��õ)
            AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World);
            if (GameMode)
            {
                // BP_HamoniaGameMode�� GetQuestManager �Լ��� �ִٸ�
                // ���⼭ �������Ʈ �Լ� ȣ��� ������ �� ����

                // �ӽ÷� �±׷� ã�� ��� ���
                TArray<AActor*> FoundActors;
                UGameplayStatics::GetAllActorsOfClassWithTag(World, ALevelQuestManager::StaticClass(),
                    FName("QuestManager"), FoundActors);

                if (FoundActors.Num() > 0)
                {
                    QuestManager = Cast<ALevelQuestManager>(FoundActors[0]);
                }
            }

            // ��� 2: �Ϲ����� Ŭ���� ã�� (���)
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

    // bIsQuestRelated�� QuestStepIndex�� ����Ǹ� ���� ������Ʈ
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UEnhancedQuestComponent, bIsQuestRelated) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(UEnhancedQuestComponent, QuestStepIndex))
    {
        UpdateQuestInfo();
    }
}

void UEnhancedQuestComponent::UpdateQuestInfo()
{
    // ����Ʈ ������ �ƴϸ� ���� �ʱ�ȭ
    if (!bIsQuestRelated)
    {
        CurrentLevelInfo = "�� ������Ʈ�� ����Ʈ�� ���þ����ϴ�.";
        CurrentSubSteps.Empty();
        SelectedStepPreview = "";
        return;
    }

    // ������ ���̺��� ������ ���
    if (!QuestDataTable)
    {
        CurrentLevelInfo = "����Ʈ ������ ���̺��� �������� �ʾҽ��ϴ�.";
        return;
    }

    // �����Ϳ����� ����Ǵ� ������ �ڵ�
    UWorld* EditorWorld = nullptr;

    // �� ������ ������� ������ ���� ��������
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

    // ������ ���̺��� ���� ���� ���� ã��
    FLevelInfo* LevelData = nullptr;
    TArray<FName> RowNames = QuestDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FLevelInfo* Row = QuestDataTable->FindRow<FLevelInfo>(RowName, TEXT(""));
        if (Row && Row->LevelName == CurrentLevelName)
        {
            LevelData = Row;
            break;
        }
    }

    if (LevelData)
    {
        // ���� ���� ������Ʈ
        CurrentLevelInfo = FString::Printf(TEXT("����: %s\n��ǥ: %s\n����: %s"),
            *LevelData->LevelName,
            *LevelData->MainObjective,
            *LevelData->Description);

        CurrentSubSteps = LevelData->SubSteps;

        // ���õ� ���� �̸�����
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