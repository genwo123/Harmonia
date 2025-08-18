#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Engine/DataTable.h"

#include "PlayerSaveData.h"
#include "NoteSaveData.h"
#include "GameProgressSaveData.h"
#include "UniaSaveData.h"
#include "GameStatsSaveData.h"

#include "Hamonia_SaveGame.generated.h"

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API UHamonia_SaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UHamonia_SaveGame();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level System")
    TMap<FString, bool> UnlockedLevels;

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void UpdateSaveTime();

    UFUNCTION(BlueprintPure, Category = "Save System")
    bool IsValidSaveData() const;

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void ResetToDefault();

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void SetSaveInfo(const FString& SlotName, bool bAuto = true, const FString& Description = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Player Data")
    void SetPlayerLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(const FString& ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(const FString& ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(const FString& ItemID, int32 MinQuantity = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Notes")
    void AddNote(const FString& NoteID);

    UFUNCTION(BlueprintCallable, Category = "Notes")
    void MarkNoteAsRead(const FString& NoteID);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void CompletePuzzle(const FString& PuzzleID, float CompletionTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void SolvePassword(const FString& PasswordID);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void SetLevelProgress(const FString& LevelName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void CompleteLevel(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void SetEventFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintPure, Category = "Progress")
    bool GetEventFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Unia")
    void SetUniaLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Unia")
    void SetUniaState(EUniaSaveState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void CompleteDialogue(const FString& DialogueID, bool bIsStoryDialogue = false);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SaveDialogueChoice(const FString& DialogueID, const FString& ChoiceResult);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetCurrentQuest(const FString& QuestPhase, const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Level System")
    void UnlockLevel(const FString& LevelName);

    UFUNCTION(BlueprintPure, Category = "Level System")
    bool IsLevelUnlocked(const FString& LevelName) const;

    UFUNCTION(BlueprintCallable, Category = "Level System")
    void UnlockNextLevel(const FString& CompletedLevel);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdatePlayTime(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void IncrementStat(const FString& StatName, int32 Amount = 1);

    UFUNCTION(BlueprintPure, Category = "Stats")
    float CalculateGameProgress() const;

    UFUNCTION(BlueprintPure, Category = "Stats")
    int32 GetUnlockedLevelCount() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UnlockAllLevels();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UnlockAllItems();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void UnlockAllNotes();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void SetGameCompleted();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintSaveDataInfo() const;

protected:
    bool CheckVersionCompatibility() const;
    bool ValidateDataIntegrity() const;
    void SetupDefaultLevels();
};