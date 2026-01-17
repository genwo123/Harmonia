#include "Core/DialogueManagerComponent.h"
#include "Core/LevelQuestManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Save_Instance/Hamoina_GameInstance.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CachedQuestManager = nullptr;
    bIsLevelEnd = false;
    DialogueWidget = nullptr;
    bIsProgressingDialogue = false;
}

void UDialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    CachedQuestManager = FindLevelQuestManager();
    bIsInDialogue = false;
    bIsLevelEnd = false;
    bIsRandomDialogue = false;
    bIsChainBreaking = false;
    CurrentDialogueID = "";
    bUserCanProgress = false;

    InitializeDialogueWidget();
}

void UDialogueManagerComponent::InitializeDialogueWidget()
{
    if (!DialogueWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueWidgetClass is NULL"));
        return;
    }

    if (DialogueWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Widget already exists, skipping creation"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] World is NULL"));
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] PlayerController is NULL"));
        return;
    }

    DialogueWidget = CreateWidget<UUserWidget>(PC, DialogueWidgetClass);
    if (DialogueWidget)
    {
        DialogueWidget->AddToViewport(10);
        DialogueWidget->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Widget created successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] Failed to create widget"));
    }
}

void UDialogueManagerComponent::ShowDialogueWidget()
{
    if (!DialogueWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueWidget is NULL"));
        return;
    }

    DialogueWidget->SetVisibility(ESlateVisibility::Visible);
}

void UDialogueManagerComponent::HideDialogueWidget()
{
    if (!DialogueWidget)
    {
        return;
    }

    DialogueWidget->SetVisibility(ESlateVisibility::Hidden);
}

bool UDialogueManagerComponent::StartDialogue(const FString& DialogueID)
{
    static int32 StartCount = 0;
    StartCount++;

    UE_LOG(LogTemp, Warning, TEXT("======== START DIALOGUE #%d ========"), StartCount);
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] ID: %s"), *DialogueID);

    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueDataTable is NULL"));
        return false;
    }

    FDialogueData* DialogueData = GetDialogueData(DialogueID);
    if (!DialogueData)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueData not found: %s"), *DialogueID);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] NextID: %s, ChainBreak: %s, IsLocked: %s"),
        *DialogueData->NextDialogueID,
        DialogueData->bChainBreak ? TEXT("TRUE") : TEXT("FALSE"),
        DialogueData->bIsLocked ? TEXT("TRUE") : TEXT("FALSE"));

    CurrentDialogue = *DialogueData;
    CurrentDialogueID = DialogueID;
    bIsLevelEnd = DialogueData->bIsLevelEnd;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->SetIgnoreMoveInput(true);
    }

    ProcessDialogue(*DialogueData);
    return true;
}

void UDialogueManagerComponent::ProcessDialogue(const FDialogueData& DialogueData)
{
    CurrentDialogue = DialogueData;
    CurrentDialogueID = DialogueData.DialogueID;
    bIsInDialogue = true;
    bUserCanProgress = false;

    ShowDialogueWidget();

    bool bIsLastDialogue = DialogueData.NextDialogueID.IsEmpty() || bIsChainBreaking;

    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] ProcessDialogue: %s, IsLast: %s"),
        *DialogueData.DialogueID,
        bIsLastDialogue ? TEXT("TRUE") : TEXT("FALSE"));

    OnDialogueStarted.Broadcast(
        DialogueData.Speaker,
        DialogueData.DialogueText,
        DialogueData.DialogueType,
        DialogueData.DisplayDuration,
        bIsLastDialogue
    );

    FTimerHandle UnlockTimer;
    GetWorld()->GetTimerManager().SetTimer(UnlockTimer, [this]()
        {
            bUserCanProgress = true;
            UE_LOG(LogTemp, Log, TEXT("[DialogueManager] User can now progress"));
        }, 0.2f, false);
}

void UDialogueManagerComponent::ProgressDialogue()
{
    static int32 CallCount = 0;
    CallCount++;

    UE_LOG(LogTemp, Warning, TEXT("======== PROGRESS DIALOGUE CALL #%d ========"), CallCount);
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] CurrentID: %s"), *CurrentDialogueID);

    if (bIsProgressingDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] BLOCKED: Already progressing"));
        return;
    }

    if (!bIsInDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] BLOCKED: Not in dialogue"));
        return;
    }

    if (!bUserCanProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] BLOCKED: User cannot progress yet"));
        return;
    }

    bUserCanProgress = false;
    bIsProgressingDialogue = true;

    if (bIsChainBreaking)
    {
        UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Chain breaking"));
        EndDialogue();
        bIsChainBreaking = false;
        bIsProgressingDialogue = false;
        OnDialogueChainBreak.Broadcast();
        return;
    }

    FString NextID = CurrentDialogue.NextDialogueID;
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] NextDialogueID: %s"), *NextID);

    if (NextID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("[DialogueManager] No next dialogue, ending"));
        EndDialogue();
        bIsProgressingDialogue = false;
        return;
    }

    FDialogueData* NextDialogueData = GetDialogueData(NextID);
    if (!NextDialogueData)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] Next dialogue not found: %s"), *NextID);
        EndDialogue();
        bIsProgressingDialogue = false;
        return;
    }

    if (NextDialogueData->bChainBreak)
    {
        UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Next is chain break"));
        HandleChainBreak(NextID, *NextDialogueData);
        bIsProgressingDialogue = false;
        return;
    }

    if (NextDialogueData->bIsLocked && !ValidateSubStepRequirement(*NextDialogueData))
    {
        UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Next is locked"));
        HandleLockedDialogue(NextID, *NextDialogueData);
        bIsProgressingDialogue = false;
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] Moving to next: %s"), *NextID);
    EndDialogue();
    StartDialogue(NextID);
    bIsProgressingDialogue = false;
}

void UDialogueManagerComponent::EndDialogue()
{
    UE_LOG(LogTemp, Log, TEXT("[DialogueManager] EndDialogue: %s"), *CurrentDialogueID);

    if (!bIsInDialogue)
    {
        return;
    }

    bIsInDialogue = false;

    if (bIsLevelEnd || bIsRandomDialogue)
    {
        CurrentDialogueID = "";
    }

    CurrentDialogue = FDialogueData();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
        PC->SetIgnoreMoveInput(false);
    }

    HideDialogueWidget();
    OnDialogueEnded.Broadcast();
}

FString UDialogueManagerComponent::PlayRandomDialogue()
{
    FString RandomID = GetRandomFromFallbackTable("DT_Unia_Random");
    if (!RandomID.IsEmpty())
    {
        StartDialogue(RandomID);
    }
    return RandomID;
}

bool UDialogueManagerComponent::CanStartDialogue(const FString& DialogueID)
{
    FDialogueData* DialogueData = GetDialogueData(DialogueID);
    if (!DialogueData)
    {
        return false;
    }

    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetWorld()->GetGameInstance());
    if (GameInstance && GameInstance->GetCurrentSaveData())
    {
        UHamonia_SaveGame* SaveData = GameInstance->GetCurrentSaveData();
        if (SaveData->UniaData.CompletedDialogues.Contains(DialogueID))
        {
            return false;
        }
    }

    if (DialogueData->bIsLevelEnd)
    {
        return false;
    }

    return CheckAllConditions(*DialogueData);
}

void UDialogueManagerComponent::HandleChainBreak(const FString& DialogueID, const FDialogueData& DialogueData)
{
    bIsChainBreaking = true;

    if (!DialogueData.NextDialogueID.IsEmpty())
    {
        SaveLastDialogueID(DialogueData.NextDialogueID);

        UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetWorld()->GetGameInstance());
        if (GameInstance)
        {
            GameInstance->SetCurrentDialogueID(DialogueData.NextDialogueID);
        }
    }

    EndDialogue();
    StartDialogue(DialogueID);
}

void UDialogueManagerComponent::HandleLockedDialogue(const FString& DialogueID, const FDialogueData& DialogueData)
{
    bIsChainBreaking = true;

    FString FallbackTable = DialogueData.FallbackTableName.IsEmpty() ?
        TEXT("DT_Unia_Random") : DialogueData.FallbackTableName;
    FString RandomDialogueID = GetRandomFromFallbackTable(FallbackTable);

    SaveLastDialogueID(DialogueID);

    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetWorld()->GetGameInstance());
    if (GameInstance)
    {
        GameInstance->SetCurrentDialogueID(RandomDialogueID);
    }

    EndDialogue();
    StartDialogue(DialogueID);
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
        return Cast<ALevelQuestManager>(QuestManagers[0]);
    }
    return nullptr;
}

bool UDialogueManagerComponent::ValidateSubStepRequirement(const FDialogueData& DialogueData)
{
    if (DialogueData.RequiredSubStep < 0)
    {
        return true;
    }
    return IsSubStepCompleted(DialogueData.RequiredSubStep);
}

void UDialogueManagerComponent::SaveLastDialogueID(const FString& DialogueID)
{
    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetWorld()->GetGameInstance());
    if (GameInstance)
    {
        if (UHamonia_SaveGame* SaveData = GameInstance->GetCurrentSaveData())
        {
            SaveData->SetPendingTriggerDialogue(DialogueID);
        }
    }
}

FString UDialogueManagerComponent::GetLastDialogueID()
{
    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(GetWorld()->GetGameInstance());
    if (GameInstance)
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