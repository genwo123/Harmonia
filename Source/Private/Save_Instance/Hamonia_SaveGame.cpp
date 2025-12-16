#include "Save_Instance/Hamonia_SaveGame.h"
#include "Engine/Engine.h"

UHamonia_SaveGame::UHamonia_SaveGame()
{
    SaveSlotName = TEXT("HarmoniaAutoSave");
    UserIndex = 0;
    GameVersion = TEXT("1.0.0");



    ResetToDefault();
}

void UHamonia_SaveGame::UpdateSaveTime()
{
    SaveTime = FDateTime::Now();
    StatsData.LastPlayTime = SaveTime;

    if (StatsData.FirstPlayTime == FDateTime())
    {
        StatsData.FirstPlayTime = SaveTime;
    }
}

bool UHamonia_SaveGame::IsValidSaveData() const
{
    if (PlayerData.CurrentLevel.IsEmpty())
    {
        return false;
    }

    if (GameVersion.IsEmpty())
    {
        return false;
    }

    return CheckVersionCompatibility() && ValidateDataIntegrity();
}

void UHamonia_SaveGame::ResetToDefault()
{
    PlayerData.CurrentLevel = TEXT("Level_Main_1");
    PlayerData.PlayerLocation = FVector::ZeroVector;
    PlayerData.PlayerRotation = FRotator::ZeroRotator;
    PlayerData.InventoryItems.Empty();
    PlayerData.ItemQuantities.Empty();
    PlayerData.CurrentEquippedItem = TEXT("");

    NoteData.CollectedNotes.Empty();
    NoteData.NoteReadStatus.Empty();
    NoteData.NotePageProgress.Empty();

    ProgressData.CompletedPuzzles.Empty();
    ProgressData.SolvedPasswords.Empty();
    ProgressData.LevelProgress.Empty();
    ProgressData.ClearedLevels.Empty();
    ProgressData.EventFlags.Empty();
    ProgressData.LevelStoryStep.Empty();
    ProgressData.PendingTriggerDialogue = TEXT("");

    UniaData.UniaLocation = FVector::ZeroVector;
    UniaData.UniaRotation = FRotator::ZeroRotator;
    UniaData.CurrentLevel = TEXT("Level_Main_1");
    UniaData.CurrentState = EUniaSaveState::Idle;
    UniaData.CompletedDialogues.Empty();

    StatsData = FGameStatsSaveData();

    HintData.LevelHintStates.Empty();

    SetupDefaultLevels();
}

void UHamonia_SaveGame::SetSaveInfo(const FString& SlotName, bool bAuto, const FString& Description)
{
    SaveSlotName = SlotName;
    bIsAutoSave = bAuto;
    SaveDescription = Description.IsEmpty() ? (bAuto ? TEXT("Auto Save") : TEXT("Manual Save")) : Description;
    UpdateSaveTime();
}

void UHamonia_SaveGame::SetPlayerLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation)
{
    PlayerData.CurrentLevel = LevelName;
    PlayerData.PlayerLocation = Location;
    PlayerData.PlayerRotation = Rotation;
}

void UHamonia_SaveGame::AddItem(const FString& ItemID, int32 Quantity)
{
    if (!PlayerData.InventoryItems.Contains(ItemID))
    {
        PlayerData.InventoryItems.Add(ItemID);
        PlayerData.ItemQuantities.Add(ItemID, Quantity);
    }
    else
    {
        int32* ExistingQuantity = PlayerData.ItemQuantities.Find(ItemID);
        if (ExistingQuantity)
        {
            *ExistingQuantity += Quantity;
        }
    }
}

bool UHamonia_SaveGame::RemoveItem(const FString& ItemID, int32 Quantity)
{
    int32* ExistingQuantity = PlayerData.ItemQuantities.Find(ItemID);
    if (!ExistingQuantity || *ExistingQuantity < Quantity)
    {
        return false;
    }

    *ExistingQuantity -= Quantity;
    if (*ExistingQuantity <= 0)
    {
        PlayerData.InventoryItems.Remove(ItemID);
        PlayerData.ItemQuantities.Remove(ItemID);
    }

    return true;
}

bool UHamonia_SaveGame::HasItem(const FString& ItemID, int32 MinQuantity) const
{
    const int32* Quantity = PlayerData.ItemQuantities.Find(ItemID);
    return Quantity && *Quantity >= MinQuantity;
}

void UHamonia_SaveGame::AddNote(const FString& NoteID)
{
    if (!NoteData.CollectedNotes.Contains(NoteID))
    {
        NoteData.CollectedNotes.Add(NoteID);
        NoteData.NoteReadStatus.Add(NoteID, false);
        NoteData.NoteFoundTime.Add(NoteID, FDateTime::Now());

        StatsData.NotesCollected++;
    }
}

void UHamonia_SaveGame::MarkNoteAsRead(const FString& NoteID)
{
    if (NoteData.CollectedNotes.Contains(NoteID))
    {
        NoteData.NoteReadStatus.FindOrAdd(NoteID) = true;

        int32& ReadCount = NoteData.NoteReadCount.FindOrAdd(NoteID, 0);
        ReadCount++;
    }
}

void UHamonia_SaveGame::CompletePuzzle(const FString& PuzzleID, float CompletionTime)
{
    if (!ProgressData.CompletedPuzzles.Contains(PuzzleID))
    {
        ProgressData.CompletedPuzzles.Add(PuzzleID);
        StatsData.PuzzlesSolved++;

        if (CompletionTime > 0.0f)
        {
            ProgressData.PuzzleBestTimes.Add(PuzzleID, CompletionTime);
        }
    }
}

void UHamonia_SaveGame::SolvePassword(const FString& PasswordID)
{
    if (!ProgressData.SolvedPasswords.Contains(PasswordID))
    {
        ProgressData.SolvedPasswords.Add(PasswordID);
        StatsData.PasswordsSolved++;
    }
}

void UHamonia_SaveGame::SetLevelProgress(const FString& LevelName, float Progress)
{
    ProgressData.LevelProgress.FindOrAdd(LevelName) = FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UHamonia_SaveGame::CompleteLevel(const FString& LevelName)
{
    if (!ProgressData.ClearedLevels.Contains(LevelName))
    {
        ProgressData.ClearedLevels.Add(LevelName);
        ProgressData.LevelClearTimes.Add(LevelName, FDateTime::Now());
        SetLevelProgress(LevelName, 1.0f);

        UnlockNextLevel(LevelName);
        StatsData.LevelsCompleted++;
    }
}

void UHamonia_SaveGame::SetEventFlag(const FString& FlagName, bool bValue)
{
    ProgressData.EventFlags.FindOrAdd(FlagName) = bValue;
}

bool UHamonia_SaveGame::GetEventFlag(const FString& FlagName) const
{
    const bool* Flag = ProgressData.EventFlags.Find(FlagName);
    return Flag ? *Flag : false;
}

void UHamonia_SaveGame::SetUniaLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation)
{
    UniaData.CurrentLevel = LevelName;
    UniaData.UniaLocation = Location;
    UniaData.UniaRotation = Rotation;
    UniaData.LevelSpawnPositions.FindOrAdd(LevelName) = Location;
}

void UHamonia_SaveGame::SetUniaState(EUniaSaveState NewState)
{
    UniaData.CurrentState = NewState;
}

void UHamonia_SaveGame::CompleteDialogue(const FString& DialogueID, bool bIsStoryDialogue)
{
    if (!UniaData.CompletedDialogues.Contains(DialogueID))
    {
        UniaData.CompletedDialogues.Add(DialogueID);

        if (bIsStoryDialogue)
        {
            UniaData.StoryDialogues.Add(DialogueID);
            StatsData.StoryDialogues++;
        }
        else
        {
            UniaData.HintDialogues.Add(DialogueID);
            StatsData.HintDialogues++;
        }

        StatsData.DialoguesCompleted++;
    }

    int32& Counter = UniaData.DialogueCounters.FindOrAdd(DialogueID, 0);
    Counter++;
}

void UHamonia_SaveGame::SaveDialogueChoice(const FString& DialogueID, const FString& ChoiceResult)
{
    UniaData.DialogueChoices.FindOrAdd(DialogueID) = ChoiceResult;
}

void UHamonia_SaveGame::SetCurrentQuest(const FString& QuestPhase, const FString& QuestID)
{
    UniaData.CurrentQuestPhase = QuestPhase;
    UniaData.CurrentQuestID = QuestID;
}

void UHamonia_SaveGame::UnlockLevel(const FString& LevelName)
{
    UnlockedLevels.FindOrAdd(LevelName) = true;
}

bool UHamonia_SaveGame::IsLevelUnlocked(const FString& LevelName) const
{
    const bool* Unlocked = UnlockedLevels.Find(LevelName);
    return Unlocked ? *Unlocked : false;
}

void UHamonia_SaveGame::UnlockNextLevel(const FString& CompletedLevel)
{
    TMap<FString, FString> LevelSequence;
    LevelSequence.Add(TEXT("Level_Main_1"), TEXT("Level_Main_2"));
    LevelSequence.Add(TEXT("Level_Main_2"), TEXT("Level_Main_3"));
    LevelSequence.Add(TEXT("Level_Main_3"), TEXT("Level_Main_4"));
    LevelSequence.Add(TEXT("Level_Main_4"), TEXT("Level_Main_5"));
    LevelSequence.Add(TEXT("Level_Main_5"), TEXT("Level_Main_6"));
    LevelSequence.Add(TEXT("Level_Main_6"), TEXT("Level_Main_7"));
    LevelSequence.Add(TEXT("Level_Main_7"), TEXT("Level_Main_8"));
    LevelSequence.Add(TEXT("Level_Main_8"), TEXT("Level_Main_9"));
    LevelSequence.Add(TEXT("Level_Main_9"), TEXT("Level_Main_10"));

    FString* NextLevel = LevelSequence.Find(CompletedLevel);
    if (NextLevel && !NextLevel->IsEmpty())
    {
        UnlockLevel(*NextLevel);
    }
}

void UHamonia_SaveGame::UpdatePlayTime(float DeltaTime)
{
    StatsData.TotalPlayTime += DeltaTime;
    StatsData.CurrentSessionTime += DeltaTime;
}

void UHamonia_SaveGame::IncrementStat(const FString& StatName, int32 Amount)
{
    if (StatName == TEXT("PuzzlesAttempted"))
    {
        StatsData.PuzzlesAttempted += Amount;
    }
    else if (StatName == TEXT("DeathCount"))
    {
        StatsData.DeathCount += Amount;
    }
    else if (StatName == TEXT("PuzzleResets"))
    {
        StatsData.PuzzleResets += Amount;
    }
    else if (StatName == TEXT("HintUsages"))
    {
        StatsData.HintUsages += Amount;
    }
}

float UHamonia_SaveGame::CalculateGameProgress() const
{
    int32 TotalLevels = 10;
    int32 CompletedLevels = ProgressData.ClearedLevels.Num();

    return TotalLevels > 0 ? (float)CompletedLevels / (float)TotalLevels : 0.0f;
}

int32 UHamonia_SaveGame::GetUnlockedLevelCount() const
{
    int32 Count = 0;
    for (const auto& Level : UnlockedLevels)
    {
        if (Level.Value)
        {
            Count++;
        }
    }
    return Count;
}

void UHamonia_SaveGame::UnlockAllLevels()
{
    TArray<FString> AllLevels = {
        TEXT("Level_Main_1"), TEXT("Level_Main_2"), TEXT("Level_Main_3"),
        TEXT("Level_Main_4"), TEXT("Level_Main_5"), TEXT("Level_Main_6"),
        TEXT("Level_Main_7"), TEXT("Level_Main_8"), TEXT("Level_Main_9"),
        TEXT("Level_Main_10")
    };

    for (const FString& Level : AllLevels)
    {
        UnlockLevel(Level);
    }
}

void UHamonia_SaveGame::UnlockAllItems()
{
    TArray<FString> AllItems = {
        TEXT("Key_Red"), TEXT("Key_Blue"), TEXT("Key_Yellow"),
        TEXT("Tool_Flashlight"), TEXT("Tool_Magnifier")
    };

    for (const FString& Item : AllItems)
    {
        AddItem(Item, 1);
    }
}

void UHamonia_SaveGame::UnlockAllNotes()
{
    TArray<FString> AllNotes = {
        TEXT("Note_Tutorial_01"), TEXT("Note_Tutorial_02"),
        TEXT("Note_Level1_01"), TEXT("Note_Level1_02"),
        TEXT("Note_Secret_01"), TEXT("Note_Hint_01")
    };

    for (const FString& Note : AllNotes)
    {
        AddNote(Note);
    }
}

void UHamonia_SaveGame::SetGameCompleted()
{
    UnlockAllLevels();
    UnlockAllItems();
    UnlockAllNotes();

    for (int32 i = 1; i <= 10; i++)
    {
        FString LevelName = FString::Printf(TEXT("Level_Main_%d"), i);
        CompleteLevel(LevelName);
    }

    StatsData.GameProgress = 1.0f;
}

bool UHamonia_SaveGame::CheckVersionCompatibility() const
{
    return GameVersion.Equals(TEXT("1.0.0"));
}

bool UHamonia_SaveGame::ValidateDataIntegrity() const
{
    return !PlayerData.CurrentLevel.IsEmpty();
}

void UHamonia_SaveGame::SetupDefaultLevels()
{
    UnlockLevel(TEXT("Level_Main_1"));
}

void UHamonia_SaveGame::SetLevelStep(const FString& LevelName, int32 Step)
{
    ProgressData.LevelStoryStep.FindOrAdd(LevelName) = Step;
}

int32 UHamonia_SaveGame::GetLevelStep(const FString& LevelName) const
{
    const int32* Step = ProgressData.LevelStoryStep.Find(LevelName);
    return Step ? *Step : 0;
}

FString UHamonia_SaveGame::GetCurrentDialogueForLevel(const FString& LevelName) const
{
    int32 CurrentStep = GetLevelStep(LevelName);
    return FString::Printf(TEXT("%s_Step%d_001"), *LevelName, CurrentStep);
}

void UHamonia_SaveGame::SetPendingTriggerDialogue(const FString& DialogueID)
{
    ProgressData.PendingTriggerDialogue = DialogueID;
}

FString UHamonia_SaveGame::GetPendingTriggerDialogue() const
{
    return ProgressData.PendingTriggerDialogue;
}

void UHamonia_SaveGame::SetCurrentDialogueID(const FString& DialogueID)
{
    UniaData.CurrentDialogueID = DialogueID;
}

FString UHamonia_SaveGame::GetCurrentDialogueID() const
{
    return UniaData.CurrentDialogueID;
}



void UHamonia_SaveGame::InitializeHintForLevel(int32 LevelNumber)
{
    if (!HintData.LevelHintStates.Contains(LevelNumber))
    {
        FHintBlockState NewState;
        HintData.LevelHintStates.Add(LevelNumber, NewState);
    }
}

void UHamonia_SaveGame::SetHintBlockStates(int32 LevelNumber, const TArray<bool>& BlockStates)
{
    FHintBlockState* State = HintData.LevelHintStates.Find(LevelNumber);
    if (!State)
    {
        InitializeHintForLevel(LevelNumber);
        State = HintData.LevelHintStates.Find(LevelNumber);
    }

    if (State && BlockStates.Num() == 6)
    {
        State->RevealedBlocks = BlockStates;
        State->LastSaveTime = FDateTime::Now();
    }
}

TArray<bool> UHamonia_SaveGame::GetHintBlockStates(int32 LevelNumber) const
{
    const FHintBlockState* State = HintData.LevelHintStates.Find(LevelNumber);
    if (State)
    {
        return State->RevealedBlocks;
    }

    // 없으면 초기 상태 반환
    TArray<bool> EmptyState;
    EmptyState.SetNum(6);
    for (int32 i = 0; i < 6; i++)
    {
        EmptyState[i] = false;
    }
    return EmptyState;
}

void UHamonia_SaveGame::SetHintCooldownTime(int32 LevelNumber, float CooldownTime)
{
    FHintBlockState* State = HintData.LevelHintStates.Find(LevelNumber);
    if (!State)
    {
        InitializeHintForLevel(LevelNumber);
        State = HintData.LevelHintStates.Find(LevelNumber);
    }

    if (State)
    {
        FDateTime Now = FDateTime::Now();

        if (State->LastSaveTime != FDateTime::MinValue())
        {
            FTimespan TimeDiff = Now - State->LastSaveTime;
            float ElapsedTime = TimeDiff.GetTotalSeconds();

            CooldownTime = FMath::Max(0.0f, CooldownTime - ElapsedTime);
        }

        State->CurrentCooldown = CooldownTime;
        State->bIsOnCooldown = (CooldownTime > 0.0f);
        State->LastSaveTime = Now;
    }
}

float UHamonia_SaveGame::GetHintCooldownTime(int32 LevelNumber) const
{
    const FHintBlockState* State = HintData.LevelHintStates.Find(LevelNumber);
    if (State)
    {
        // 마지막 저장 이후 경과 시간 계산
        FDateTime Now = FDateTime::Now();
        FTimespan TimeDiff = Now - State->LastSaveTime;
        float ElapsedTime = TimeDiff.GetTotalSeconds();

        float RemainingCooldown = FMath::Max(0.0f, State->CurrentCooldown - ElapsedTime);
        return RemainingCooldown;
    }

    return 0.0f;
}

void UHamonia_SaveGame::SetTriggerDialogueID(const FString& DialogueID)
{
    UniaData.TriggerDialogueID = DialogueID;
}

FString UHamonia_SaveGame::GetTriggerDialogueID() const
{
    return UniaData.TriggerDialogueID;
}

void UHamonia_SaveGame::ResetHintForLevel(int32 LevelNumber)
{
    FHintBlockState ResetState;
    HintData.LevelHintStates.Add(LevelNumber, ResetState);
}

void UHamonia_SaveGame::SaveQuestProgress(const FString& CurrentLevel, const TArray<FString>& CompletedLevels, const TArray<bool>& SubStepStatus)
{
    ProgressData.CurrentQuestLevel = CurrentLevel;
    ProgressData.CompletedQuestLevels = CompletedLevels;
    ProgressData.CurrentSubStepStatus = SubStepStatus;
}

void UHamonia_SaveGame::LoadQuestProgress(FString& OutCurrentLevel, TArray<FString>& OutCompletedLevels, TArray<bool>& OutSubStepStatus)
{
    OutCurrentLevel = ProgressData.CurrentQuestLevel;
    OutCompletedLevels = ProgressData.CompletedQuestLevels;
    OutSubStepStatus = ProgressData.CurrentSubStepStatus;
}