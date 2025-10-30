#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Engine/DataTable.h"

#include "PlayerSaveData.h"
#include "NoteSaveData.h"
#include "GameProgressSaveData.h"
#include "UniaSaveData.h"
#include "GameStatsSaveData.h"
#include "HintSaveData.h"

#include "Hamonia_SaveGame.generated.h"

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API UHamonia_SaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UHamonia_SaveGame();

    // Story Progress Functions
    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void SetLevelStep(const FString& LevelName, int32 Step);

    UFUNCTION(BlueprintPure, Category = "Story Progress")
    int32 GetLevelStep(const FString& LevelName) const;

    UFUNCTION(BlueprintPure, Category = "Story Progress")
    FString GetCurrentDialogueForLevel(const FString& LevelName) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void SetPendingTriggerDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintPure, Category = "Story Progress")
    FString GetPendingTriggerDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Save|Dialogue")
    void SetTriggerDialogueID(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Save|Dialogue")
    FString GetTriggerDialogueID() const;

    // Save Info
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Info")
    FString SaveSlotName = TEXT("HarmoniaAutoSave");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Info")
    int32 UserIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Info")
    FDateTime SaveTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Info")
    FString GameVersion = TEXT("1.0.0");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Info")
    bool bIsAutoSave = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Info")
    FString SaveDescription = TEXT("Auto Save");

    // Game Data (여기 한 번만!)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
    FPlayerSaveData PlayerData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
    FNoteSaveData NoteData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
    FGameProgressSaveData ProgressData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
    FUniaSaveData UniaData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
    FGameStatsSaveData StatsData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Data")
    FHintSaveData HintData;  // 힌트 데이터 추가

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
    TMap<FString, bool> UnlockedLevels;

    // Save System Functions
    UFUNCTION(BlueprintCallable, Category = "Save System")
    void UpdateSaveTime();

    UFUNCTION(BlueprintPure, Category = "Save System")
    bool IsValidSaveData() const;

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void ResetToDefault();

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void SetSaveInfo(const FString& SlotName, bool bAuto = true, const FString& Description = TEXT(""));

    // Player Data Functions
    UFUNCTION(BlueprintCallable, Category = "Player Data")
    void SetPlayerLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation);

    // Inventory Functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(const FString& ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(const FString& ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(const FString& ItemID, int32 MinQuantity = 1) const;

    // Notes Functions
    UFUNCTION(BlueprintCallable, Category = "Notes")
    void AddNote(const FString& NoteID);

    UFUNCTION(BlueprintCallable, Category = "Notes")
    void MarkNoteAsRead(const FString& NoteID);

    // Progress Functions
    UFUNCTION(BlueprintCallable, Category = "Progress")
    void CompletePuzzle(const FString& PuzzleID, float CompletionTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void SolvePassword(const FString& PasswordID);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void SetLevelProgress(const FString& LevelName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Unia Dialogue")
    void SetCurrentDialogueID(const FString& DialogueID);

    UFUNCTION(BlueprintPure, Category = "Unia Dialogue")
    FString GetCurrentDialogueID() const;

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void CompleteLevel(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void SetEventFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintPure, Category = "Progress")
    bool GetEventFlag(const FString& FlagName) const;

    // Unia Functions
    UFUNCTION(BlueprintCallable, Category = "Unia")
    void SetUniaLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Unia")
    void SetUniaState(EUniaSaveState NewState);

    // Dialogue Functions
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CompleteDialogue(const FString& DialogueID, bool bIsStoryDialogue = false);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SaveDialogueChoice(const FString& DialogueID, const FString& ChoiceResult);

    // Quest Functions
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetCurrentQuest(const FString& QuestPhase, const FString& QuestID);

    // Level System Functions
    UFUNCTION(BlueprintCallable, Category = "Level System")
    void UnlockLevel(const FString& LevelName);

    UFUNCTION(BlueprintPure, Category = "Level System")
    bool IsLevelUnlocked(const FString& LevelName) const;

    UFUNCTION(BlueprintCallable, Category = "Level System")
    void UnlockNextLevel(const FString& CompletedLevel);

    // Stats Functions
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdatePlayTime(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void IncrementStat(const FString& StatName, int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category = "Stats")
    float CalculateGameProgress() const;

    UFUNCTION(BlueprintPure, Category = "Stats")
    int32 GetUnlockedLevelCount() const;

    // Hint System Functions (힌트 함수들)
    UFUNCTION(BlueprintCallable, Category = "Hint")
    void InitializeHintForLevel(int32 LevelNumber);

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void SetHintBlockStates(int32 LevelNumber, const TArray<bool>& BlockStates);

    UFUNCTION(BlueprintPure, Category = "Hint")
    TArray<bool> GetHintBlockStates(int32 LevelNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void SetHintCooldownTime(int32 LevelNumber, float CooldownTime);

    UFUNCTION(BlueprintPure, Category = "Hint")
    float GetHintCooldownTime(int32 LevelNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Hint")
    void ResetHintForLevel(int32 LevelNumber);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UnlockAllLevels();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString CurrentQuestLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> CompletedQuestLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<bool> CurrentSubStepStatus;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SaveQuestProgress(const FString& CurrentLevel, const TArray<FString>& CompletedLevels, const TArray<bool>& SubStepStatus);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void LoadQuestProgress(FString& OutCurrentLevel, TArray<FString>& OutCompletedLevels, TArray<bool>& OutSubStepStatus);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UnlockAllItems();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UnlockAllNotes();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetGameCompleted();

protected:
    bool CheckVersionCompatibility() const;
    bool ValidateDataIntegrity() const;
    void SetupDefaultLevels();
};