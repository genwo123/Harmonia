// LevelQuestManager.cpp - 최소한 수정 버전
#include "Core/LevelQuestManager.h"

ALevelQuestManager::ALevelQuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentLevel = "Level_01"; // 기본 시작 레벨

    // 기본 데이터 테이블 경로 설정 (선택사항)
    static ConstructorHelpers::FObjectFinder<UDataTable> QuestDTObject(TEXT("/Game/Hamonia/H_DataTable/DT_LevelQuest"));
    if (QuestDTObject.Succeeded())
    {
        LevelDataTable = QuestDTObject.Object;
    }
}

void ALevelQuestManager::BeginPlay()
{
    Super::BeginPlay();

    FString LevelToStart;

    if (bAutoDetectLevel)
    {
        // 자동 맵 이름 감지
        UWorld* World = GetWorld();
        if (World)
        {
            LevelToStart = World->GetMapName();
            LevelToStart.RemoveFromStart(World->StreamingLevelsPrefix);
        }
    }
    else
    {
        // 기본 레벨 사용
        LevelToStart = CurrentLevel;
    }

    // 자동으로 해당 맵 퀘스트 시작
    StartLevel(LevelToStart);

    UE_LOG(LogTemp, Warning, TEXT("Quest Manager started with level: %s"), *LevelToStart);
}

void ALevelQuestManager::StartLevel(const FString& LevelID)
{
    if (!LevelDataTable) return;
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*LevelID, "");
    if (!LevelData) return;

    CurrentLevel = LevelID;

    // SubStepCompletionStatus 초기화
    SubStepCompletionStatus.SetNum(LevelData->SubSteps.Num());
    for (int32 i = 0; i < SubStepCompletionStatus.Num(); i++)
    {
        SubStepCompletionStatus[i] = false;
    }
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

// 현재 레벨 이름 반환
FString ALevelQuestManager::GetCurrentLevelName()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->LevelName : "";
}

// 현재 레벨 완료 여부
bool ALevelQuestManager::IsCurrentLevelCompleted()
{
    return IsLevelCompleted(CurrentLevel);
}

// 현재 레벨에 소목표가 있는지 체크
bool ALevelQuestManager::HasSubSteps()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return false;
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData && LevelData->SubSteps.Num() > 0;
}

// 특정 소목표 완료
void ALevelQuestManager::CompleteSubStep(int32 StepIndex)
{
    if (SubStepCompletionStatus.IsValidIndex(StepIndex))
    {
        SubStepCompletionStatus[StepIndex] = true;

        // 모든 소목표 완료 시 레벨 완료
        bool bAllCompleted = true;
        for (bool bCompleted : SubStepCompletionStatus)
        {
            if (!bCompleted)
            {
                bAllCompleted = false;
                break;
            }
        }

        if (bAllCompleted)
        {
            CompleteCurrentLevel();
        }
    }
}

// 특정 소목표 완료 여부 확인
bool ALevelQuestManager::IsSubStepCompleted(int32 StepIndex)
{
    if (SubStepCompletionStatus.IsValidIndex(StepIndex))
    {
        return SubStepCompletionStatus[StepIndex];
    }
    return false;
}

// 모든 소목표 완료 상태 반환
TArray<bool> ALevelQuestManager::GetAllSubStepStatus()
{
    return SubStepCompletionStatus;
}

// 총 소목표 개수 반환
int32 ALevelQuestManager::GetSubStepCount()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return 0;
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->SubSteps.Num() : 0;
}

// 모든 소목표 텍스트 반환
TArray<FString> ALevelQuestManager::GetAllSubStepTexts()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return {};
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->SubSteps : TArray<FString>();
}

FString ALevelQuestManager::GetCurrentMainObjective()
{
    if (!LevelDataTable || CurrentLevel.IsEmpty()) return "";
    FLevelInfo* LevelData = LevelDataTable->FindRow<FLevelInfo>(*CurrentLevel, "");
    return LevelData ? LevelData->MainObjective : "";
}

int32 ALevelQuestManager::GetCurrentSubStep()
{
    // 완료되지 않은 첫 번째 단계를 반환
    for (int32 i = 0; i < SubStepCompletionStatus.Num(); i++)
    {
        if (!SubStepCompletionStatus[i])
        {
            return i;  // 첫 번째 미완료 단계 반환
        }
    }

    // 모든 단계가 완료되었으면 마지막 인덱스 반환
    return FMath::Max(0, SubStepCompletionStatus.Num() - 1);
}