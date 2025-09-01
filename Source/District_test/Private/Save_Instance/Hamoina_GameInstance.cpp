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

    UnlockedNotePages.SetNum(5);
    for (int32 i = 0; i < 5; i++)
    {
        UnlockedNotePages[i] = false;
    }

}

void UHamoina_GameInstance::Init()
{
    Super::Init();
    InitializeNewSaveData();
    StartAutoSaveTimer();
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
        LoadPath = GetAutoSaveDirectory() + TEXT("/") + LoadSlotName + TEXT(".sav");
    }
    else
    {
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

bool UHamoina_GameInstance::LoadStageAndOpenLevel(int32 StageNumber)
{
    bool bLoadSuccess = LoadFromStageSlot(StageNumber);

    if (bLoadSuccess && CurrentSaveData)
    {
        FString SavedLevelName = CurrentSaveData->PlayerData.CurrentLevel;

        if (SavedLevelName.IsEmpty())
        {
            SavedLevelName = GetLevelNameFromStage(StageNumber);
        }

        UGameplayStatics::OpenLevel(this, *SavedLevelName);
        return true;
    }

    return false;
}

FString UHamoina_GameInstance::GetLevelNameFromStage(int32 StageNumber) const
{
    return FString::Printf(TEXT("Level_Main_%d"), StageNumber);
}

void UHamoina_GameInstance::ApplyLoadedGameState()
{
    if (!CurrentSaveData)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = CurrentSaveData->PlayerData.PlayerLocation;
        FRotator PlayerRotation = CurrentSaveData->PlayerData.PlayerRotation;

        PC->GetPawn()->SetActorLocation(PlayerLocation);
        PC->GetPawn()->SetActorRotation(PlayerRotation);
    }
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
    FString LatestAutoSave = GetLatestAutoSaveFile();
    if (LatestAutoSave.IsEmpty())
    {
        // 기존 방식 시도 (HarmoniaContinue.sav)
        bool bLoadSuccess = LoadGame();
        if (bLoadSuccess && CurrentSaveData)
        {
            FString SavedLevelName = CurrentSaveData->PlayerData.CurrentLevel;
            if (!SavedLevelName.IsEmpty())
            {
                UGameplayStatics::OpenLevel(this, *SavedLevelName);
                return true;
            }
        }
        return false;
    }

    // 직접 경로로 로드 (LoadGame 대신 LoadGameFromCustomPath 사용)
    FString FullPath = GetAutoSaveDirectory() + TEXT("/") + LatestAutoSave;
    bool bLoadSuccess = LoadGameFromCustomPath(FullPath);

    if (bLoadSuccess && CurrentSaveData)
    {
        FString SavedLevelName = CurrentSaveData->PlayerData.CurrentLevel;
        if (!SavedLevelName.IsEmpty())
        {
            UGameplayStatics::OpenLevel(this, *SavedLevelName);
            return true;
        }
    }

    return false;
}

bool UHamoina_GameInstance::HasContinueGame() const
{
    // 새로운 자동저장 파일들 확인
    TArray<FString> AutoSaveFiles = const_cast<UHamoina_GameInstance*>(this)->GetAutoSaveFileList();
    if (AutoSaveFiles.Num() > 0)
        return true;

    // 기존 자동저장 파일 확인
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
        int32 MainIndex = LevelName.Find(TEXT("Level_Main_"), ESearchCase::IgnoreCase);
        if (MainIndex != INDEX_NONE)
        {
            FString AfterMain = LevelName.Mid(MainIndex + 11);
            FString NumberPart;
            for (int32 i = 0; i < AfterMain.Len(); i++)
            {
                TCHAR Ch = AfterMain[i];
                if (FChar::IsDigit(Ch))
                {
                    NumberPart += Ch;
                }
                else
                {
                    break;
                }
            }

            if (!NumberPart.IsEmpty())
            {
                return FCString::Atoi(*NumberPart);
            }
        }
    }

    if (LevelName.Contains(TEXT("MainMenu")))
    {
        return 0;
    }

    return 1;
}

int32 UHamoina_GameInstance::GetCurrentStageNumber() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 1;
    }

    FString MapName = World->GetMapName();
    FString WorldName = World->GetName();
    FString StreamingPrefix = World->StreamingLevelsPrefix;
    FString PackageName = World->GetOutermost() ? World->GetOutermost()->GetName() : TEXT("NoPackage");

    int32 MapNameStage = GetStageNumberFromLevel(MapName);
    int32 WorldNameStage = GetStageNumberFromLevel(WorldName);
    int32 PackageNameStage = GetStageNumberFromLevel(PackageName);

    FString CleanMapName = MapName;
    CleanMapName.RemoveFromStart(StreamingPrefix);
    int32 CleanMapStage = GetStageNumberFromLevel(CleanMapName);

    FString LevelOnlyName;
    if (PackageName.Split(TEXT("/"), nullptr, &LevelOnlyName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        int32 LevelOnlyStage = GetStageNumberFromLevel(LevelOnlyName);
        if (LevelOnlyStage > 1)
        {
            return LevelOnlyStage;
        }
    }

    if (CleanMapStage > 1)
    {
        return CleanMapStage;
    }
    if (WorldNameStage > 1)
    {
        return WorldNameStage;
    }
    if (MapNameStage > 1)
    {
        return MapNameStage;
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

        FString MapName = World->GetMapName();
        MapName.RemoveFromStart(World->StreamingLevelsPrefix);

        CurrentSaveData->SetPlayerLocation(MapName, PlayerLocation, PlayerRotation);
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
            return false;
        }
    }

    TArray<uint8> SaveData;
    FMemoryWriter MemoryWriter(SaveData, true);

    FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
    Ar.ArIsSaveGame = true;

    SaveGame->Serialize(Ar);

    return FFileHelper::SaveArrayToFile(SaveData, *FilePath);
}

bool UHamoina_GameInstance::LoadGameFromCustomPath(const FString& FilePath)
{
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        return false;
    }

    TArray<uint8> SaveData;
    if (!FFileHelper::LoadFileToArray(SaveData, *FilePath))
    {
        return false;
    }

    FMemoryReader MemoryReader(SaveData, true);
    FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
    Ar.ArIsSaveGame = true;

    UHamonia_SaveGame* LoadedData = NewObject<UHamonia_SaveGame>();
    LoadedData->Serialize(Ar);

    if (!LoadedData->IsValidSaveData())
    {
        return false;
    }

    CurrentSaveData = LoadedData;
    return true;
}


bool UHamoina_GameInstance::SaveToAutoSlot()
{
    if (!CurrentSaveData)
        return false;

    CollectCurrentGameState();

    // 현재 슬롯에 저장
    FString AutoSlotName = FString::Printf(TEXT("AutoSave_%d"), CurrentAutoSaveSlot);
    FString SavePath = GetAutoSaveDirectory() + TEXT("/") + AutoSlotName + TEXT(".sav");

    CurrentSaveData->SetSaveInfo(AutoSlotName, true);
    bool bSaveSuccess = SaveGameToCustomPath(CurrentSaveData, SavePath);

    if (bSaveSuccess)
    {
        // 다음 슬롯으로 이동 (순환)
        CurrentAutoSaveSlot = (CurrentAutoSaveSlot % MaxAutoSaveSlots) + 1;
    }

    OnGameSaved.Broadcast(bSaveSuccess);
    return bSaveSuccess;
}

FString UHamoina_GameInstance::GetLatestAutoSaveFile()
{
    TArray<FString> AutoSaveFiles = GetAutoSaveFileList();
    if (AutoSaveFiles.Num() == 0)
        return TEXT("");

    // 파일 생성시간으로 정렬해서 최신 파일 반환
    FString LatestFile;
    FDateTime LatestTime = FDateTime::MinValue();

    for (const FString& FileName : AutoSaveFiles)
    {
        FString FullPath = GetAutoSaveDirectory() + TEXT("/") + FileName;
        FDateTime FileTime = FPlatformFileManager::Get().GetPlatformFile().GetTimeStamp(*FullPath);

        if (FileTime > LatestTime)
        {
            LatestTime = FileTime;
            LatestFile = FileName;
        }
    }

    return LatestFile;
}

TArray<FString> UHamoina_GameInstance::GetAutoSaveFileList()
{
    TArray<FString> AutoSaveFiles;
    FString AutoSaveDir = GetAutoSaveDirectory();

    for (int32 i = 1; i <= MaxAutoSaveSlots; i++)
    {
        FString FileName = FString::Printf(TEXT("AutoSave_%d.sav"), i);
        FString FullPath = AutoSaveDir + TEXT("/") + FileName;

        if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FullPath))
        {
            AutoSaveFiles.Add(FileName);
        }
    }

    return AutoSaveFiles;
}

void UHamoina_GameInstance::StartAutoSaveTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            AutoSaveTimerHandle,
            [this]() { SaveToAutoSlot(); },
            AutoSaveInterval,
            true
        );
    }
}

void UHamoina_GameInstance::StopAutoSaveTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
    }
}

void UHamoina_GameInstance::SetAutoSaveInterval(float NewInterval)
{
    AutoSaveInterval = NewInterval;

    // 타이머가 실행 중이면 재시작
    if (AutoSaveTimerHandle.IsValid())
    {
        StopAutoSaveTimer();
        StartAutoSaveTimer();
    }
}

void UHamoina_GameInstance::UnlockNotePage(int32 PageIndex)
{
    if (PageIndex >= 0 && PageIndex < UnlockedNotePages.Num())
    {
        UnlockedNotePages[PageIndex] = true;
        //SaveContinueGame(); // 자동 저장
    }
}

bool UHamoina_GameInstance::IsNotePageUnlocked(int32 PageIndex) const
{
    if (PageIndex >= 0 && PageIndex < UnlockedNotePages.Num())
    {
        return UnlockedNotePages[PageIndex];
    }
    return false;
}