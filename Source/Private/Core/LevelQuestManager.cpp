// Core/LevelQuestManager.cpp
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

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*LevelID, TEXT(""));
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

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*LevelID, TEXT(""));
    if (!LevelData) return false;

    if (LevelData->PrerequisiteLevel.IsEmpty()) return true;

    return IsLevelCompleted(LevelData->PrerequisiteLevel);
}

FString ALevelQuestManager::GetCurrentLevelDialogue()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return TEXT("");

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    return LevelData ? LevelData->LumiDialogueID : TEXT("");
}

FString ALevelQuestManager::GetCurrentLevelName()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return TEXT("");

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    return LevelData ? LevelData->LevelName : TEXT("");
}

bool ALevelQuestManager::IsCurrentLevelCompleted()
{
    return IsLevelCompleted(CurrentLevel);
}

bool ALevelQuestManager::HasSubSteps()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return false;

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    return LevelData && LevelData->SubSteps.Num() > 0;
}

void ALevelQuestManager::CompleteSubStep(int32 StepIndex)
{
    if (!SubStepCompletionStatus.IsValidIndex(StepIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("[LevelQuestManager] Invalid StepIndex: %d"), StepIndex);
        return;
    }

    if (SubStepCompletionStatus[StepIndex])
    {
        UE_LOG(LogTemp, Warning, TEXT("[LevelQuestManager] SubStep %d already completed"), StepIndex);
        return;
    }

    SubStepCompletionStatus[StepIndex] = true;
    UE_LOG(LogTemp, Log, TEXT("[LevelQuestManager] SubStep %d completed!"), StepIndex);

    FString DialogueID = GetDialogueIDForSubStep(StepIndex);
    FString WaitSpotID = GetWaitSpotIDForSubStep(StepIndex);

    if (!DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("[LevelQuestManager] Triggering Dialogue: %s"), *DialogueID);
    }
    if (!WaitSpotID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("[LevelQuestManager] Unia will move to: %s"), *WaitSpotID);
    }

    OnSubStepCompleted.Broadcast(StepIndex, DialogueID, WaitSpotID);

    bool bAllCompleted = true;
    for (int32 i = 0; i < SubStepCompletionStatus.Num(); i++)
    {
        if (!SubStepCompletionStatus[i])
        {
            bAllCompleted = false;
            UE_LOG(LogTemp, Log, TEXT("[LevelQuestManager] SubStep %d still incomplete"), i);
            break;
        }
    }

    if (bAllCompleted)
    {
        UE_LOG(LogTemp, Log, TEXT("[LevelQuestManager] All SubSteps completed! Level Clear!"));
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

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    return LevelData ? LevelData->SubSteps.Num() : 0;
}

TArray<FString> ALevelQuestManager::GetAllSubStepTexts()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return {};

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    return LevelData ? LevelData->SubSteps : TArray<FString>();
}

FString ALevelQuestManager::GetCurrentMainObjective()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return TEXT("");

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    return LevelData ? LevelData->MainObjective : TEXT("");
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

FString ALevelQuestManager::GetDialogueIDForSubStep(int32 StepIndex)
{
    if (!LevelDataTable || CurrentLevel.IsEmpty())
    {
        return TEXT("");
    }

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    if (!LevelData)
    {
        return TEXT("");
    }

    if (LevelData->SubStepDialogueIDs.IsValidIndex(StepIndex))
    {
        return LevelData->SubStepDialogueIDs[StepIndex];
    }

    return TEXT("");
}

FString ALevelQuestManager::GetWaitSpotIDForSubStep(int32 StepIndex)
{
    if (!LevelDataTable || CurrentLevel.IsEmpty())
    {
        return TEXT("");
    }

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, TEXT(""));
    if (!LevelData)
    {
        return TEXT("");
    }

    if (LevelData->SubStepWaitSpotIDs.IsValidIndex(StepIndex))
    {
        return LevelData->SubStepWaitSpotIDs[StepIndex];
    }

    return TEXT("");
}

FString ALevelQuestManager::GetCurrentUnlockedDialogueID()
{
    int32 CurrentStep = GetCurrentSubStep();

    if (CurrentStep < 0)
    {
        return TEXT("");
    }

    return GetDialogueIDForSubStep(CurrentStep);
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