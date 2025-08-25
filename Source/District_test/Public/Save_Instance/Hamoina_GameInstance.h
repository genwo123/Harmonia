#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Save_Instance/Hamonia_SaveGame.h"
#include "Hamoina_GameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSaved, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLoaded, bool, bSuccess);

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API UHamoina_GameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UHamoina_GameInstance();

protected:
    virtual void Init() override;

public:
    UPROPERTY(BlueprintReadOnly, Category = "Save System")
    UHamonia_SaveGame* CurrentSaveData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save System")
    FString AutoSaveSlotName = TEXT("HarmoniaContinue");

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool SaveGame(bool bIsAutoSave = true, const FString& CustomSlotName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool LoadGame(const FString& SlotName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool SaveToStageSlot(int32 StageNumber);

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool LoadFromStageSlot(int32 StageNumber);

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void SaveContinueGame();

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool LoadContinueGame();

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool HasContinueGame() const;

    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool DoesStageSlotExist(int32 StageNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Save System")
    TArray<FString> GetAvailableSaveSlots() const;

    UFUNCTION(BlueprintPure, Category = "Save System")
    UHamonia_SaveGame* GetCurrentSaveData() const { return CurrentSaveData; }

    UFUNCTION(BlueprintCallable, Category = "Player Data")
    void UpdatePlayerLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Player Data")
    void UpdateUniaLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void OnPuzzleCompleted(const FString& PuzzleID, float CompletionTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void OnPasswordSolved(const FString& PasswordID);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void OnLevelCompleted(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void OnNoteCollected(const FString& NoteID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItemToInventory(const FString& ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemFromInventory(const FString& ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItemInInventory(const FString& ItemID, int32 MinQuantity = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnDialogueCompleted(const FString& DialogueID, bool bIsStoryDialogue = false);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DevSaveToStage(int32 StageNumber);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DevSaveCurrentStage();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DevUnlockAllLevels();

    UFUNCTION(BlueprintPure, Category = "Level Utils")
    int32 GetStageNumberFromLevel(const FString& LevelName) const;

    UFUNCTION(BlueprintPure, Category = "Level Utils")
    FString GetLevelNameFromStage(int32 StageNumber) const;

    UFUNCTION(BlueprintPure, Category = "Level Utils")
    int32 GetCurrentStageNumber() const;

    UPROPERTY(BlueprintAssignable)
    FOnGameSaved OnGameSaved;

    UPROPERTY(BlueprintAssignable)
    FOnGameLoaded OnGameLoaded;

protected:
    void InitializeNewSaveData();
    void CollectCurrentGameState();
    FString GetCustomSaveDirectory() const;
    FString GetAutoSaveDirectory() const;
    FString GetStageSlotName(int32 StageNumber) const;
    bool SaveGameToCustomPath(USaveGame* SaveGame, const FString& FilePath);
    bool LoadGameFromCustomPath(const FString& FilePath);
};