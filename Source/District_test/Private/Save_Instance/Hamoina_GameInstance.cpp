#include "Save_Instance/Hamoina_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

UHamoina_GameInstance::UHamoina_GameInstance()
{
    AutoSaveSlotName = TEXT("HarmoniaContinue");
}

void UHamoina_GameInstance::Init()
{
    Super::Init();
    InitializeNewSaveData();
}

void UHamoina_GameInstance::InitializeNewSaveData()
{
    CurrentSaveData = NewObject<UHamonia_SaveGame>();
    if (CurrentSaveData)
    {
        CurrentSaveData->ResetToDefault();
    }
}

bool UHamoina_GameInstance::SaveGame(bool bIsAutoSave, const FString& CustomSlotName)
{
    if (!CurrentSaveData)
        return false;

    CollectCurrentGameState();

    FString SlotName;
    FString SavePath;

    if (bIsAutoSave)
    {
        SlotName = AutoSaveSlotName;
        SavePath = GetAutoSaveDirectory() + TEXT("/") + SlotName + TEXT(".sav");
    }
    else
    {
        SlotName = CustomSlotName.IsEmpty() ?
            FString::Printf(TEXT("Level_Main_%d"), GetCurrentStageNumber()) : CustomSlotName;
        SavePath = GetCustomSaveDirectory() + TEXT("/") + SlotName + TEXT(".sav");
    }

    CurrentSaveData->SetSaveInfo(SlotName, bIsAutoSave);

    bool bSaveSuccess = SaveGameToCustomPath(CurrentSaveData, SavePath);
    OnGameSaved.Broadcast(bSaveSuccess);
    return bSaveSuccess;
}

bool UHamoina_GameInstance::LoadGame(const FString& SlotName)
{
    FString LoadSlotName = SlotName.IsEmpty() ? AutoSaveSlotName : SlotName;
    FString LoadPath;

    if (SlotName.IsEmpty())
    {
        // 자동저장 로드
        LoadPath = GetAutoSaveDirectory() + TEXT("/") + LoadSlotName + TEXT(".sav");
    }
    else
    {
        // 스테이지 파일 로드
        LoadPath = GetCustomSaveDirectory() + TEXT("/") + LoadSlotName + TEXT(".sav");
    }

    bool bLoadSuccess = LoadGameFromCustomPath(LoadPath);
    OnGameLoaded.Broadcast(bLoadSuccess);
    return bLoadSuccess;
}

bool UHamoina_GameInstance::SaveToStageSlot(int32 StageNumber)
{
    if (StageNumber < 2 || StageNumber > 10)
        return false;

    FString StageSlotName = GetStageSlotName(StageNumber);
    return SaveGame(false, StageSlotName);
}

bool UHamoina_GameInstance::LoadFromStageSlot(int32 StageNumber)
{
    if (StageNumber < 2 || StageNumber > 10)
        return false;

    FString StageSlotName = GetStageSlotName(StageNumber);
    return LoadGame(StageSlotName);
}

bool UHamoina_GameInstance::DoesStageSlotExist(int32 StageNumber) const
{
    if (StageNumber < 2 || StageNumber > 10)
        return false;

    FString StageSlotName = GetStageSlotName(StageNumber);
    FString FilePath = GetCustomSaveDirectory() + TEXT("/") + StageSlotName + TEXT(".sav");
    return FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath);
}

TArray<FString> UHamoina_GameInstance::GetAvailableSaveSlots() const
{
    TArray<FString> AvailableSlots;

    for (int32 i = 2; i <= 10; i++)
    {
        if (DoesStageSlotExist(i))
        {
            FString StageSlotName = GetStageSlotName(i);
            AvailableSlots.Add(StageSlotName);
        }
    }

    return AvailableSlots;
}

void UHamoina_GameInstance::SaveContinueGame()
{
    SaveGame(true);
}

bool UHamoina_GameInstance::LoadContinueGame()
{
    return LoadGame();
}

bool UHamoina_GameInstance::HasContinueGame() const
{
    FString FilePath = GetAutoSaveDirectory() + TEXT("/") + AutoSaveSlotName + TEXT(".sav");
    return FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath);
}

void UHamoina_GameInstance::UpdatePlayerLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->SetPlayerLocation(LevelName, Location, Rotation);
    }
}

void UHamoina_GameInstance::UpdateUniaLocation(const FString& LevelName, const FVector& Location, const FRotator& Rotation)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->SetUniaLocation(LevelName, Location, Rotation);
    }
}

void UHamoina_GameInstance::OnPuzzleCompleted(const FString& PuzzleID, float CompletionTime)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->CompletePuzzle(PuzzleID, CompletionTime);
        SaveContinueGame();
    }
}

void UHamoina_GameInstance::OnPasswordSolved(const FString& PasswordID)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->SolvePassword(PasswordID);
        SaveContinueGame();
    }
}

void UHamoina_GameInstance::OnLevelCompleted(const FString& LevelName)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->CompleteLevel(LevelName);
        SaveContinueGame();
    }
}

void UHamoina_GameInstance::OnNoteCollected(const FString& NoteID)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->AddNote(NoteID);
        SaveContinueGame();
    }
}

void UHamoina_GameInstance::AddItemToInventory(const FString& ItemID, int32 Quantity)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->AddItem(ItemID, Quantity);
    }
}

bool UHamoina_GameInstance::RemoveItemFromInventory(const FString& ItemID, int32 Quantity)
{
    if (CurrentSaveData)
    {
        return CurrentSaveData->RemoveItem(ItemID, Quantity);
    }
    return false;
}

bool UHamoina_GameInstance::HasItemInInventory(const FString& ItemID, int32 MinQuantity) const
{
    if (CurrentSaveData)
    {
        return CurrentSaveData->HasItem(ItemID, MinQuantity);
    }
    return false;
}

void UHamoina_GameInstance::OnDialogueCompleted(const FString& DialogueID, bool bIsStoryDialogue)
{
    if (CurrentSaveData)
    {
        CurrentSaveData->CompleteDialogue(DialogueID, bIsStoryDialogue);
    }
}

void UHamoina_GameInstance::DevSaveToStage(int32 StageNumber)
{
    SaveToStageSlot(StageNumber);
}

void UHamoina_GameInstance::DevSaveCurrentStage()
{
    int32 CurrentStage = GetCurrentStageNumber();
    if (CurrentStage >= 2 && CurrentStage <= 10)
    {
        DevSaveToStage(CurrentStage);
    }
}

void UHamoina_GameInstance::DevUnlockAllLevels()
{
    if (CurrentSaveData)
    {
        CurrentSaveData->UnlockAllLevels();
    }
}

int32 UHamoina_GameInstance::GetStageNumberFromLevel(const FString& LevelName) const
{
    if (LevelName.Contains(TEXT("Level_Main_")))
    {
        FString NumberPart = LevelName.Replace(TEXT("Level_Main_"), TEXT(""));
        if (NumberPart.IsNumeric())
        {
            return FCString::Atoi(*NumberPart);
        }
    }
    return 1;
}

FString UHamoina_GameInstance::GetLevelNameFromStage(int32 StageNumber) const
{
    return FString::Printf(TEXT("Level_Main_%d"), StageNumber);
}

int32 UHamoina_GameInstance::GetCurrentStageNumber() const
{
    if (CurrentSaveData)
    {
        return GetStageNumberFromLevel(CurrentSaveData->PlayerData.CurrentLevel);
    }
    return 1;
}

void UHamoina_GameInstance::CollectCurrentGameState()
{
    if (!CurrentSaveData)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        FRotator PlayerRotation = PC->GetPawn()->GetActorRotation();

        FString WorldName = World->GetName();
        FString MapName = World->GetMapName();
        FString CurrentLevel = MapName.IsEmpty() ? WorldName : MapName;

        CurrentSaveData->SetPlayerLocation(CurrentLevel, PlayerLocation, PlayerRotation);
    }

    CurrentSaveData->UpdatePlayTime(World->GetDeltaSeconds());
}

FString UHamoina_GameInstance::GetCustomSaveDirectory() const
{
    return FPaths::ProjectDir() + TEXT("Load");
}

FString UHamoina_GameInstance::GetAutoSaveDirectory() const
{
    return FPaths::ProjectDir() + TEXT("Load/Auto");
}

FString UHamoina_GameInstance::GetStageSlotName(int32 StageNumber) const
{
    return FString::Printf(TEXT("Level_Main_%d"), StageNumber);
}

bool UHamoina_GameInstance::SaveGameToCustomPath(USaveGame* SaveGame, const FString& FilePath)
{
    if (!SaveGame)
        return false;

    FString Directory = FPaths::GetPath(FilePath);
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*Directory))
    {
        if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*Directory))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create directory: %s"), *Directory);
            return false;
        }
    }

    TArray<uint8> SaveData;
    FMemoryWriter MemoryWriter(SaveData, true);

    // 언리얼 5에서는 FObjectAndNameAsStringProxyArchive 생성자가 다릅니다
    FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
    Ar.ArIsSaveGame = true;

    SaveGame->Serialize(Ar);

    if (FFileHelper::SaveArrayToFile(SaveData, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Game saved successfully to: %s"), *FilePath);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game to: %s"), *FilePath);
        return false;
    }
}

bool UHamoina_GameInstance::LoadGameFromCustomPath(const FString& FilePath)
{
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Save file does not exist: %s"), *FilePath);
        return false;
    }

    TArray<uint8> SaveData;
    if (!FFileHelper::LoadFileToArray(SaveData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load save file: %s"), *FilePath);
        return false;
    }

    FMemoryReader MemoryReader(SaveData, true);

    // 언리얼 5에서는 FObjectAndNameAsStringProxyArchive 생성자가 다릅니다
    FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
    Ar.ArIsSaveGame = true;

    UHamonia_SaveGame* LoadedData = NewObject<UHamonia_SaveGame>();
    LoadedData->Serialize(Ar);

    if (!LoadedData->IsValidSaveData())
    {
        UE_LOG(LogTemp, Error, TEXT("Loaded save data is invalid"));
        return false;
    }

    CurrentSaveData = LoadedData;
    UE_LOG(LogTemp, Log, TEXT("Game loaded successfully from: %s"), *FilePath);
    return true;
}