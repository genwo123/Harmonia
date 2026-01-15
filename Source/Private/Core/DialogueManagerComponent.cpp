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
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] BeginPlay called"));

    CachedQuestManager = FindLevelQuestManager();
    bIsInDialogue = false;
    bIsLevelEnd = false;
    bIsRandomDialogue = false;
    bIsChainBreaking = false;
    CurrentDialogueID = "";
    bUserCanProgress = false;  // 초기에는 진행 불가

    InitializeDialogueWidget();
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] BeginPlay completed"));
}

void UDialogueManagerComponent::InitializeDialogueWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] InitializeDialogueWidget called"));

    if (!DialogueWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueWidgetClass is NULL!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] World is NULL!"));
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] PlayerController is NULL!"));
        return;
    }

    DialogueWidget = CreateWidget<UUserWidget>(PC, DialogueWidgetClass);
    if (DialogueWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Widget created successfully"));
        DialogueWidget->AddToViewport(10);
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Widget added to viewport"));
        DialogueWidget->SetVisibility(ESlateVisibility::Hidden);
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Widget visibility set to Hidden"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] Failed to create widget!"));
    }
}

void UDialogueManagerComponent::ShowDialogueWidget()
{
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] ShowDialogueWidget called"));

    if (!DialogueWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueWidget is NULL in ShowDialogueWidget!"));
        return;
    }

    DialogueWidget->SetVisibility(ESlateVisibility::Visible);
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Widget visibility set to Visible"));
}

void UDialogueManagerComponent::HideDialogueWidget()
{
    if (!DialogueWidget)
    {
        return;
    }

    DialogueWidget->SetVisibility(ESlateVisibility::Hidden);
}

// StartDialogue 함수에 로그 추가
bool UDialogueManagerComponent::StartDialogue(const FString& DialogueID)
{
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] StartDialogue called with ID: %s"), *DialogueID);

    if (!DialogueDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueDataTable is NULL!"));
        return false;
    }

    FDialogueData* DialogueData = GetDialogueData(DialogueID);
    if (!DialogueData)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] DialogueData not found for ID: %s"), *DialogueID);
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Dialogue found - NextID: %s, ChainBreak: %s, IsLocked: %s"),
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

    // 새 대화가 시작되면 무조건 진행 불가 상태로 설정
    bUserCanProgress = false;

    ShowDialogueWidget();

    bool bIsLastDialogue = DialogueData.NextDialogueID.IsEmpty() || bIsChainBreaking;

    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] ProcessDialogue - ID: %s"), *DialogueData.DialogueID);
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] IsLastDialogue: %s, UserCanProgress: FALSE (BLOCKED)"),
        bIsLastDialogue ? TEXT("TRUE") : TEXT("FALSE"));

    OnDialogueStarted.Broadcast(
        DialogueData.Speaker,
        DialogueData.DialogueText,
        DialogueData.DialogueType,
        DialogueData.DisplayDuration,
        bIsLastDialogue
    );

    //  Broadcast 후 0.2초 뒤에 진행 허용
    FTimerHandle UnlockTimer;
    GetWorld()->GetTimerManager().SetTimer(UnlockTimer, [this]()
        {
            bUserCanProgress = true;
            UE_LOG(LogTemp, Warning, TEXT("[DialogueManager]  User can now progress"));
        }, 0.2f, false);
}

// ProgressDialogue - 완전 차단
void UDialogueManagerComponent::ProgressDialogue()
{
    UE_LOG(LogTemp, Warning, TEXT("----------------------------------------"));
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] ProgressDialogue called"));
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] UserCanProgress: %s"),
        bUserCanProgress ? TEXT("TRUE") : TEXT("FALSE"));

    //  진행 불가 상태면 무조건 차단
    if (!bUserCanProgress)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager]  BLOCKED! User cannot progress yet. Ignoring call."));
        return;
    }

    // 진행 시작하면 다시 차단
    bUserCanProgress = false;

    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Current State - InDialogue: %s, IsProgressing: %s"),
        bIsInDialogue ? TEXT("TRUE") : TEXT("FALSE"),
        bIsProgressingDialogue ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] CurrentDialogueID: %s"), *CurrentDialogueID);

    if (bIsProgressingDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Already progressing, returning"));
        return;
    }

    if (!bIsInDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Not in dialogue, returning"));
        bIsProgressingDialogue = false;
        return;
    }

    if (bIsChainBreaking)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Chain breaking, ending dialogue"));
        EndDialogue();
        bIsChainBreaking = false;
        OnDialogueChainBreak.Broadcast();
        return;
    }

    bIsProgressingDialogue = true;

    FString NextID = CurrentDialogue.NextDialogueID;
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] NextDialogueID: %s"), *NextID);

    if (NextID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] No next dialogue, ending"));
        EndDialogue();
        bIsProgressingDialogue = false;
        return;
    }

    FDialogueData* NextDialogueData = GetDialogueData(NextID);
    if (!NextDialogueData)
    {
        UE_LOG(LogTemp, Error, TEXT("[DialogueManager] Next dialogue data not found for ID: %s"), *NextID);
        EndDialogue();
        bIsProgressingDialogue = false;
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Next dialogue found - ChainBreak: %s, IsLocked: %s"),
        NextDialogueData->bChainBreak ? TEXT("TRUE") : TEXT("FALSE"),
        NextDialogueData->bIsLocked ? TEXT("TRUE") : TEXT("FALSE"));

    if (NextDialogueData->bChainBreak)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Next dialogue is chain break, handling..."));
        HandleChainBreak(NextID, *NextDialogueData);
        bIsProgressingDialogue = false;
        return;
    }

    if (NextDialogueData->bIsLocked && !ValidateSubStepRequirement(*NextDialogueData))
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Next dialogue is locked, handling..."));
        HandleLockedDialogue(NextID, *NextDialogueData);
        bIsProgressingDialogue = false;
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager]  ALLOWED: Ending current dialogue and starting next: %s"), *NextID);
    EndDialogue();
    StartDialogue(NextID);
    bIsProgressingDialogue = false;
}

// EndDialogue 함수에 로그 추가
void UDialogueManagerComponent::EndDialogue()
{
    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] EndDialogue called - CurrentID: %s"), *CurrentDialogueID);

    if (!bIsInDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Not in dialogue, skipping end"));
        return;
    }

    bIsInDialogue = false;

    if (bIsLevelEnd || bIsRandomDialogue)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Level end or random dialogue, clearing CurrentDialogueID"));
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

    UE_LOG(LogTemp, Warning, TEXT("[DialogueManager] Dialogue ended"));
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
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