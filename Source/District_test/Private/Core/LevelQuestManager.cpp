// LevelQuestManager.cpp - ���� ����
#include "Core/LevelQuestManager.h"

ALevelQuestManager::ALevelQuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentLevel = "Level_01"; // �⺻ ���� ����
}

void ALevelQuestManager::StartLevel(const FString& LevelID)
{
    if (!CanStartLevel(LevelID)) return;

    CurrentLevel = LevelID;
}

void ALevelQuestManager::CompleteCurrentLevel()
{
    if (CurrentLevel.IsEmpty()) return;

    CompletedLevels.AddUnique(CurrentLevel);
}

bool ALevelQuestManager::IsLevelCompleted(const FString& LevelID)
{
    return CompletedLevels.Contains(LevelID);
}

bool ALevelQuestManager::CanStartLevel(const FString& LevelID)
{
    if (!LevelDataTable) return false;

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*LevelID, "");
    if (!LevelData) return false;

    // ���� ������ ������ ���� ����
    if (LevelData->PrerequisiteLevel.IsEmpty()) return true;

    // ���� ���� �Ϸ������� ���� ����
    return IsLevelCompleted(LevelData->PrerequisiteLevel);
}

FString ALevelQuestManager::GetCurrentLevelDialogue()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->LumiDialogueID : "";
}