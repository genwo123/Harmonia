#include "Core/DialogueManagerComponent.h"
#include "Core/LevelQuestManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Save_Instance/Hamoina_GameInstance.h"
#include "TimerManager.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CachedQuestManager = nullptr;
    bIsLevelEnd = false;
}

void UDialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    CachedQuestManager = FindLevelQuestManager();
    bIsInDialogue = false;
    bIsLevelEnd = false;
    CurrentDialogueID = "";
}

bool UDialogueManagerComponent::StartDialogue(const FString& DialogueID)
{
    if (bIsInDialogue)
    {
        return false;
    }

    if (bIsLevelEnd)
    {
        return false;
    }

    FDialogueData* DialogueData = GetDialogueData(DialogueID);
    if (!DialogueData)
    {
        return false;
    }

    if (!CheckAllConditions(*DialogueData))
    {
        return false;
    }

    bIsInDialogue = true;
    CurrentDialogueID = DialogueID;
    CurrentDialogue = *DialogueData;

    ProcessDialogue(*DialogueData);

    if (DialogueData->bIsLevelEnd)
    {
        float Duration = DialogueData->DisplayDuration > 0 ? DialogueData->DisplayDuration : 5.0f;

        FTimerHandle LevelEndTimer;
        GetWorld()->GetTimerManager().SetTimer(LevelEndTimer, [this]()
            {
                bIsLevelEnd = true;
                EndDialogue();
            }, Duration, false);
    }

    return true;
}

void UDialogueManagerComponent::EndDialogue()
{
    if (!bIsInDialogue)
    {
        return;
    }

    bIsInDialogue = false;
    CurrentDialogueID = "";

    OnDialogueEnded.Broadcast();
}

void UDialogueManagerComponent::ProgressDialogue()
{
    if (!bIsInDialogue)
    {
        return;
    }

    if (CurrentDialogue.NextDialogueID.IsEmpty())
    {
        EndDialogue();
        return;
    }

    FString NextID = CurrentDialogue.NextDialogueID;
    FDialogueData* NextDialogueData = GetDialogueData(NextID);

    if (!NextDialogueData)
    {
        EndDialogue();
        return;
    }

    if (NextDialogueData->bChainBreak)
    {
        EndDialogue();
        StartDialogue(NextID);

        if (!NextDialogueData->NextDialogueID.IsEmpty())
        {
            SaveLastDialogueID(NextDialogueData->NextDialogueID);
        }

        FTimerHandle ChainBreakTimer;
        GetWorld()->GetTimerManager().SetTimer(ChainBreakTimer, [this]()
            {
                EndDialogue();
            }, NextDialogueData->DisplayDuration, false);

        return;
    }

    if (NextDialogueData->bIsLocked)
    {
        if (!ValidateSubStepRequirement(*NextDialogueData))
        {
            EndDialogue();

            FString RandomID = GetRandomFromFallbackTable("DT_Unia_Random");
            if (!RandomID.IsEmpty())
            {
                StartDialogue(RandomID);
            }

            SaveLastDialogueID(NextID);

            FTimerHandle LockTimer;
            GetWorld()->GetTimerManager().SetTimer(LockTimer, [this]()
                {
                    EndDialogue();
                }, 3.0f, false);

            return;
        }
    }

    EndDialogue();
    StartDialogue(NextID);
}

void UDialogueManagerComponent::SelectChoice(int32 ChoiceIndex)
{
    if (!bIsInDialogue || !CurrentDialogue.bHasChoices)
    {
        return;
    }

    if (ChoiceIndex < 0 || ChoiceIndex >= CurrentDialogue.ChoiceTargetIDs.Num())
    {
        return;
    }

    FString TargetDialogueID = CurrentDialogue.ChoiceTargetIDs[ChoiceIndex];
    if (TargetDialogueID.IsEmpty())
    {
        EndDialogue();
        return;
    }

    EndDialogue();
    StartDialogue(TargetDialogueID);
}

TArray<FString> UDialogueManagerComponent::GetCurrentChoices()
{
    if (bIsInDialogue && CurrentDialogue.bHasChoices)
    {
        return CurrentDialogue.ChoiceTexts;
    }

    return TArray<FString>();
}

bool UDialogueManagerComponent::CanProgressToDialogue(const FString& DialogueID)
{
    FDialogueData* DialogueData = GetDialogueData(DialogueID);
    if (!DialogueData)
    {
        return false;
    }

    return ValidateSubStepRequirement(*DialogueData);
}

FString UDialogueManagerComponent::FindDialogueForCurrentLevel()
{
    FString SavedID = GetLastDialogueID();
    if (!SavedID.IsEmpty())
    {
        FDialogueData* SavedData = GetDialogueData(SavedID);
        if (SavedData && !SavedData->bIsLevelEnd)
        {
            return SavedID;
        }
    }

    FString CurrentLevel = GetCurrentLevelName();
    if (CurrentLevel.IsEmpty())
    {
        return "";
    }

    TArray<FString> LevelDialogues = GetDialoguesForLevel(CurrentLevel, EDialogueCategory::MainStory);
    for (const FString& DialogueID : LevelDialogues)
    {
        FDialogueData* DialogueData = GetDialogueData(DialogueID);
        if (DialogueData && !DialogueData->bIsLevelEnd)
        {
            if (CanProgressToDialogue(DialogueID))
            {
                return DialogueID;
            }
        }
    }

    return "";
}

FString UDialogueManagerComponent::GetMacroDialogue(const FString& LevelName, int32 CurrentSubStep)
{
    if (!DialogueDataTable)
    {
        return "";
    }

    FString MacroID = FString::Printf(TEXT("%s_Macro_Step%d"), *LevelName, CurrentSubStep);

    FDialogueData* MacroData = GetDialogueData(MacroID);
    if (MacroData)
    {
        return MacroID;
    }

    TArray<FString> MacroDialogues = GetDialoguesForLevel(LevelName, EDialogueCategory::Macro);
    if (MacroDialogues.Num() > 0)
    {
        int32 Index = CurrentSubStep % MacroDialogues.Num();
        return MacroDialogues[Index];
    }

    return "Unia_Random_001";
}

TArray<FString> UDialogueManagerComponent::GetDialoguesForLevel(const FString& LevelName, EDialogueCategory Category)
{
    TArray<FString> Result;

    if (!DialogueDataTable)
    {
        return Result;
    }

    TArray<FDialogueData*> AllDialogues;
    DialogueDataTable->GetAllRows<FDialogueData>("", AllDialogues);

    for (FDialogueData* DialogueData : AllDialogues)
    {
        if (DialogueData &&
            DialogueData->LevelName == LevelName &&
            DialogueData->Category == Category)
        {
            Result.Add(DialogueData->DialogueID);
        }
    }

    return Result;
}

bool UDialogueManagerComponent::IsSubStepCompleted(int32 SubStepIndex)
{
    if (!CachedQuestManager)
    {
        CachedQuestManager = FindLevelQuestManager();
    }

    if (CachedQuestManager)
    {
        return CachedQuestManager->IsSubStepCompleted(SubStepIndex);
    }

    return false;
}

int32 UDialogueManagerComponent::GetCurrentSubStep()
{
    if (!CachedQuestManager)
    {
        CachedQuestManager = FindLevelQuestManager();
    }

    if (CachedQuestManager)
    {
        return CachedQuestManager->GetCurrentSubStep();
    }

    return 0;
}

FString UDialogueManagerComponent::GetCurrentLevelName()
{
    if (!CachedQuestManager)
    {
        CachedQuestManager = FindLevelQuestManager();
    }

    if (CachedQuestManager)
    {
        return CachedQuestManager->GetCurrentLevelName();
    }

    UWorld* World = GetWorld();
    if (World)
    {
        FString LevelName = World->GetMapName();
        LevelName.RemoveFromStart(World->StreamingLevelsPrefix);
        return LevelName;
    }

    return "";
}

FString UDialogueManagerComponent::FindDialogueForQuest(const FString& QuestState)
{
    return FindDialogueForCurrentLevel();
}

void UDialogueManagerComponent::PlayMonologue(const FString& MonologueID)
{
    FDialogueData* MonologueData = GetDialogueData(MonologueID);
    if (!MonologueData)
    {
        return;
    }

    ProcessDialogue(*MonologueData);
}

bool UDialogueManagerComponent::CheckAllConditions(const FDialogueData& DialogueData)
{
    if (!ValidateSubStepRequirement(DialogueData))
    {
        return false;
    }

    return true;
}

FDialogueData* UDialogueManagerComponent::GetDialogueData(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        return nullptr;
    }

    TArray<FDialogueData*> AllDialogues;
    DialogueDataTable->GetAllRows<FDialogueData>("", AllDialogues);

    for (FDialogueData* DialogueData : AllDialogues)
    {
        if (DialogueData && DialogueData->DialogueID == DialogueID)
        {
            return DialogueData;
        }
    }

    return nullptr;
}

void UDialogueManagerComponent::ProcessDialogue(const FDialogueData& DialogueData)
{
    OnDialogueStarted.Broadcast(
        DialogueData.Speaker,
        DialogueData.DialogueText,
        DialogueData.DialogueType,
        DialogueData.DisplayDuration
    );
}

ALevelQuestManager* UDialogueManagerComponent::FindLevelQuestManager()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> QuestManagers;
    UGameplayStatics::GetAllActorsOfClass(World, ALevelQuestManager::StaticClass(), QuestManagers);

    if (QuestManagers.Num() > 0)
    {
        ALevelQuestManager* QuestMgr = Cast<ALevelQuestManager>(QuestManagers[0]);
        return QuestMgr;
    }

    return nullptr;
}

bool UDialogueManagerComponent::ValidateSubStepRequirement(const FDialogueData& DialogueData)
{
    if (DialogueData.RequiredSubStep < 0)
    {
        return true;
    }

    bool bCompleted = IsSubStepCompleted(DialogueData.RequiredSubStep);
    return bCompleted;
}

void UDialogueManagerComponent::SaveLastDialogueID(const FString& DialogueID)
{
    if (UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetWorld()->GetGameInstance()))
    {
        if (UHamonia_SaveGame* SaveData = GameInstance->GetCurrentSaveData())
        {
            SaveData->SetPendingTriggerDialogue(DialogueID);
        }
    }
}

FString UDialogueManagerComponent::GetLastDialogueID()
{
    if (UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetWorld()->GetGameInstance()))
    {
        if (UHamonia_SaveGame* SaveData = GameInstance->GetCurrentSaveData())
        {
            return SaveData->GetPendingTriggerDialogue();
        }
    }
    return "";
}

FString UDialogueManagerComponent::GetRandomFromFallbackTable(const FString& TableName)
{
    TArray<FString> HintDialogues = {
        TEXT("Hint_Puzzle_001"),
        TEXT("Hint_Puzzle_002"),
        TEXT("Hint_Puzzle_003"),
        TEXT("Hint_Puzzle_004"),
        TEXT("Hint_Puzzle_005")
    };

    if (HintDialogues.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, HintDialogues.Num() - 1);
        return HintDialogues[RandomIndex];
    }

    return "";
}

bool UDialogueManagerComponent::IsDialogueLocked(const FString& DialogueID)
{
    if (FDialogueData* DialogueData = GetDialogueData(DialogueID))
    {
        return DialogueData->bIsLocked;
    }
    return false;
}

FString UDialogueManagerComponent::GetLockedDialogueReplacement(const FString& DialogueID)
{
    if (FDialogueData* DialogueData = GetDialogueData(DialogueID))
    {
        if (DialogueData->bIsLocked)
        {
            return GetRandomFromFallbackTable(DialogueData->FallbackTableName);
        }
    }
    return DialogueID;
}

FDialogueData* UDialogueManagerComponent::GetCurrentDialogueData()
{
    if (CurrentDialogueID.IsEmpty())
    {
        return nullptr;
    }

    return &CurrentDialogue;
}

bool UDialogueManagerComponent::IsCurrentDialogueLevelEnd() const
{
    return bIsLevelEnd;
}