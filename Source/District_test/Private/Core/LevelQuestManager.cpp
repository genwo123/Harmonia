#include "Core/LevelQuestManager.h"
#include "Kismet/GameplayStatics.h"
#include "Save_Instance/Hamoina_GameInstance.h"

ALevelQuestManager::ALevelQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentLevel = "Level_Main_0";

    static ConstructorHelpers::FObjectFinder<UDataTable> QuestDTObject(TEXT("/Game/Hamonia/H_DataTable/DT_LevelQuest"));
    if (QuestDTObject.Succeeded())
    {
        LevelDataTable = QuestDTObject.Object;
    }
}

void ALevelQuestManager::BeginPlay()
{
    Super::BeginPlay();

    LoadQuestProgress();

    FString LevelToStart;

    if (bAutoDetectLevel)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            LevelToStart = World->GetMapName();
            LevelToStart.RemoveFromStart(World->StreamingLevelsPrefix);
        }
    }
    else
    {
        LevelToStart = CurrentLevel;
    }

    if (!LevelToStart.IsEmpty())
    {
        LastLoadedLevel = LevelToStart;
        StartLevel(LevelToStart);
    }
}

void ALevelQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bAutoDetectLevel) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FString CurrentMapName = World->GetMapName();
    CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);

    if (CurrentMapName != LastLoadedLevel && !CurrentMapName.IsEmpty())
    {
        LastLoadedLevel = CurrentMapName;
        LoadQuestProgress();
        StartLevel(CurrentMapName);
    }
}

void ALevelQuestManager::StartLevel(const FString& LevelID)
{
    if (!LevelDataTable) return;

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*LevelID, "");
    if (!LevelData) return;

    if (CurrentLevel == LevelID && SubStepCompletionStatus.Num() > 0)
    {

        OnQuestUpdated.Broadcast(CurrentLevel);
        return;
    }

    CurrentLevel = LevelID;

    SubStepCompletionStatus.SetNum(LevelData->SubSteps.Num());
    for (int32 i = 0; i < SubStepCompletionStatus.Num(); i++)
    {
        SubStepCompletionStatus[i] = false;
    }

    OnQuestUpdated.Broadcast(CurrentLevel);
}

void ALevelQuestManager::CompleteCurrentLevel()
{
    if (CurrentLevel.IsEmpty()) return;

    CompletedLevels.AddUnique(CurrentLevel);
    SaveQuestProgress();
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

    if (LevelData->PrerequisiteLevel.IsEmpty()) return true;

    return IsLevelCompleted(LevelData->PrerequisiteLevel);
}

FString ALevelQuestManager::GetCurrentLevelDialogue()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->LumiDialogueID : "";
}

FString ALevelQuestManager::GetCurrentLevelName()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->LevelName : "";
}

bool ALevelQuestManager::IsCurrentLevelCompleted()
{
    return IsLevelCompleted(CurrentLevel);
}

bool ALevelQuestManager::HasSubSteps()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return false;

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData && LevelData->SubSteps.Num() > 0;
}

void ALevelQuestManager::CompleteSubStep(int32 StepIndex)
{
    if (!SubStepCompletionStatus.IsValidIndex(StepIndex)) return;

    SubStepCompletionStatus[StepIndex] = true;

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

    SaveQuestProgress();
}

bool ALevelQuestManager::IsSubStepCompleted(int32 StepIndex)
{
    if (SubStepCompletionStatus.IsValidIndex(StepIndex))
    {
        return SubStepCompletionStatus[StepIndex];
    }
    return false;
}

TArray<bool> ALevelQuestManager::GetAllSubStepStatus()
{
    return SubStepCompletionStatus;
}

int32 ALevelQuestManager::GetSubStepCount()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return 0;

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->SubSteps.Num() : 0;
}

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
    for (int32 i = 0; i < SubStepCompletionStatus.Num(); i++)
    {
        if (!SubStepCompletionStatus[i])
        {
            return i;
        }
    }

    return FMath::Max(0, SubStepCompletionStatus.Num() - 1);
}

void ALevelQuestManager::SaveQuestProgress()
{
    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(
        UGameplayStatics::GetGameInstance(this));

    if (GameInstance && GameInstance->CurrentSaveData)
    {
        GameInstance->CurrentSaveData->SaveQuestProgress(
            CurrentLevel,
            CompletedLevels,
            SubStepCompletionStatus
        );
    }
}

void ALevelQuestManager::RefreshForCurrentLevel()
{
    LoadQuestProgress();

    if (bAutoDetectLevel)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            FString LevelName = World->GetMapName();
            LevelName.RemoveFromStart(World->StreamingLevelsPrefix);

            if (!LevelName.IsEmpty())
            {
                StartLevel(LevelName);
            }
        }
    }
}

void ALevelQuestManager::LoadQuestProgress()
{
    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(
        UGameplayStatics::GetGameInstance(this));

    if (GameInstance && GameInstance->CurrentSaveData)
    {
        FString LoadedLevel;
        TArray<FString> LoadedCompleted;
        TArray<bool> LoadedSubSteps;

        GameInstance->CurrentSaveData->LoadQuestProgress(
            LoadedLevel,
            LoadedCompleted,
            LoadedSubSteps
        );

        if (!LoadedLevel.IsEmpty())
        {
            CurrentLevel = LoadedLevel;
            CompletedLevels = LoadedCompleted;
            SubStepCompletionStatus = LoadedSubSteps;
        }
    }
}