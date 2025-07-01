// LevelQuestManager.cpp - 간단 버전
#include "Core/LevelQuestManager.h"

ALevelQuestManager::ALevelQuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentLevel = "Level_01"; // 기본 시작 레벨
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

    // 선행 레벨이 없으면 시작 가능
    if (LevelData->PrerequisiteLevel.IsEmpty()) return true;

    // 선행 레벨 완료했으면 시작 가능
    return IsLevelCompleted(LevelData->PrerequisiteLevel);
}

FString ALevelQuestManager::GetCurrentLevelDialogue()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";

    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->LumiDialogueID : "";
}