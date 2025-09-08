// LevelQuestManager.cpp - �ּ��� ���� ����
#include "Core/LevelQuestManager.h"

ALevelQuestManager::ALevelQuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentLevel = "Level_01"; // �⺻ ���� ����

    // �⺻ ������ ���̺� ��� ���� (���û���)
    static ConstructorHelpers::FObjectFinder<UDataTable> QuestDTObject(TEXT("/Game/Data/DT_LevelQuests"));
    if (QuestDTObject.Succeeded())
    {
        LevelDataTable = QuestDTObject.Object;
    }
}

void ALevelQuestManager::BeginPlay()
{
    Super::BeginPlay();

    FString LevelToStart;

    if (bAutoDetectLevel)
    {
        // �ڵ� �� �̸� ����
        UWorld* World = GetWorld();
        if (World)
        {
            LevelToStart = World->GetMapName();
            LevelToStart.RemoveFromStart(World->StreamingLevelsPrefix);
        }
    }
    else
    {
        // �⺻ ���� ���
        LevelToStart = CurrentLevel;
    }

    // �ڵ����� �ش� �� ����Ʈ ����
    StartLevel(LevelToStart);

    UE_LOG(LogTemp, Warning, TEXT("Quest Manager started with level: %s"), *LevelToStart);
}

void ALevelQuestManager::StartLevel(const FString& LevelID)
{
    if (!LevelDataTable) return;
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*LevelID, "");
    if (!LevelData) return;

    CurrentLevel = LevelID;

    // SubStepCompletionStatus �ʱ�ȭ
    SubStepCompletionStatus.SetNum(LevelData->SubSteps.Num());
    for (int32 i = 0; i < SubStepCompletionStatus.Num(); i++)
    {
        SubStepCompletionStatus[i] = false;
    }
}

void ALevelQuestManager::CompleteCurrentLevel()
{
    if (CurrentLevel.IsEmpty()) return;
    CompletedLevels.AddUnique(CurrentLevel);
}

bool ALevelQuestManager::IsLevelCompleted(const FString& LevelID)
{
    return CompletedLevels.Contains(LevelID);
}

bool ALevelQuestManager::CanStartLevel(const FString& LevelID)
{
    if (!LevelDataTable) return false;
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*LevelID, "");
    if (!LevelData) return false;

    // ���� ������ ������ ���� ����
    if (LevelData->PrerequisiteLevel.IsEmpty()) return true;

    // ���� ���� �Ϸ������� ���� ����
    return IsLevelCompleted(LevelData->PrerequisiteLevel);
}

FString ALevelQuestManager::GetCurrentLevelDialogue()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->LumiDialogueID : "";
}

// ���� ���� �̸� ��ȯ
FString ALevelQuestManager::GetCurrentLevelName()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->LevelName : "";
}

// ���� ���� �Ϸ� ����
bool ALevelQuestManager::IsCurrentLevelCompleted()
{
    return IsLevelCompleted(CurrentLevel);
}

// ���� ������ �Ҹ�ǥ�� �ִ��� üũ
bool ALevelQuestManager::HasSubSteps()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return false;
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData && LevelData->SubSteps.Num() > 0;
}

// Ư�� �Ҹ�ǥ �Ϸ�
void ALevelQuestManager::CompleteSubStep(int32 StepIndex)
{
    if (SubStepCompletionStatus.IsValidIndex(StepIndex))
    {
        SubStepCompletionStatus[StepIndex] = true;

        // ��� �Ҹ�ǥ �Ϸ� �� ���� �Ϸ�
        bool bAllCompleted = true;
        for (bool bCompleted : SubStepCompletionStatus)
        {
            if (!bCompleted)
            {
                bAllCompleted = false;
                break;
            }
        }

        if (bAllCompleted)
        {
            CompleteCurrentLevel();
        }
    }
}

// Ư�� �Ҹ�ǥ �Ϸ� ���� Ȯ��
bool ALevelQuestManager::IsSubStepCompleted(int32 StepIndex)
{
    if (SubStepCompletionStatus.IsValidIndex(StepIndex))
    {
        return SubStepCompletionStatus[StepIndex];
    }
    return false;
}

// ��� �Ҹ�ǥ �Ϸ� ���� ��ȯ
TArray<bool> ALevelQuestManager::GetAllSubStepStatus()
{
    return SubStepCompletionStatus;
}

// �� �Ҹ�ǥ ���� ��ȯ
int32 ALevelQuestManager::GetSubStepCount()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return 0;
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->SubSteps.Num() : 0;
}

// ��� �Ҹ�ǥ �ؽ�Ʈ ��ȯ
TArray<FString> ALevelQuestManager::GetAllSubStepTexts()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return {};
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->SubSteps : TArray<FString>();
}

FString ALevelQuestManager::GetCurrentMainObjective()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->MainObjective : "";
}

int32 ALevelQuestManager::GetCurrentSubStep()
{
    // �Ϸ���� ���� ù ��° �ܰ踦 ��ȯ
    for (int32 i = 0; i < SubStepCompletionStatus.Num(); i++)
    {
        if (!SubStepCompletionStatus[i])
        {
            return i;  // ù ��° �̿Ϸ� �ܰ� ��ȯ
        }
    }

    // ��� �ܰ谡 �Ϸ�Ǿ����� ������ �ε��� ��ȯ
    return FMath::Max(0, SubStepCompletionStatus.Num() - 1);
}