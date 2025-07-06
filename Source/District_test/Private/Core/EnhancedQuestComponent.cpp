// EnhancedQuestComponent.cpp
#include "Core/EnhancedQuestComponent.h"
#include "Core/LevelQuestManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

UEnhancedQuestComponent::UEnhancedQuestComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // 기본 데이터 테이블 경로 설정 (프로젝트에 맞게 수정)
    static ConstructorHelpers::FObjectFinder<UDataTable> QuestDTObject(TEXT("/Game/Data/DT_LevelQuests"));
    if (QuestDTObject.Succeeded())
    {
        QuestDataTable = QuestDTObject.Object;
    }
}

void UEnhancedQuestComponent::BeginPlay()
{
    Super::BeginPlay();

    // 현재 레벨명 가져오기
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentLevelName = World->GetMapName();
        CurrentLevelName.RemoveFromStart(World->StreamingLevelsPrefix);
    }

    // Quest Manager 찾기
    GetQuestManager();
}

void UEnhancedQuestComponent::CompleteQuestStep()
{
    // 퀘스트 관련이 아니면 실행하지 않음
    if (!bIsQuestRelated)
    {
        UE_LOG(LogTemp, Warning, TEXT("Object is not quest related. Skipping quest completion."));
        return;
    }

    // 직접 참조된 Quest Manager 사용
    if (QuestManagerRef)
    {
        QuestManagerRef->CompleteSubStep(QuestStepIndex);
        UE_LOG(LogTemp, Warning, TEXT("Quest Step %d completed!"), QuestStepIndex);
        return;
    }

    // 백업: 자동으로 찾기
    if (!QuestManager)
    {
        QuestManager = GetQuestManager();
    }

    if (QuestManager)
    {
        QuestManager->CompleteSubStep(QuestStepIndex);
        UE_LOG(LogTemp, Warning, TEXT("Quest Step %d completed! (Found automatically)"), QuestStepIndex);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Quest Manager not found! Please set QuestManagerRef manually."));
    }
}

bool UEnhancedQuestComponent::IsCurrentStepCompleted()
{
    if (!bIsQuestRelated)
        return false;

    if (!QuestManager)
    {
        QuestManager = GetQuestManager();
    }

    if (QuestManager)
    {
        return QuestManager->IsSubStepCompleted(QuestStepIndex);
    }

    return false;
}

ALevelQuestManager* UEnhancedQuestComponent::GetQuestManager()
{
    if (!QuestManager)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            // 방법 1: GameMode를 통해 Quest Manager 찾기 (추천)
            AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World);
            if (GameMode)
            {
                // BP_HamoniaGameMode에 GetQuestManager 함수가 있다면
                // 여기서 블루프린트 함수 호출로 가져올 수 있음

                // 임시로 태그로 찾는 방법 사용
                TArray<AActor*> FoundActors;
                UGameplayStatics::GetAllActorsOfClassWithTag(World, ALevelQuestManager::StaticClass(),
                    FName("QuestManager"), FoundActors);

                if (FoundActors.Num() > 0)
                {
                    QuestManager = Cast<ALevelQuestManager>(FoundActors[0]);
                }
            }

            // 방법 2: 일반적인 클래스 찾기 (백업)
            if (!QuestManager)
            {
                TArray<AActor*> FoundManagers;
                UGameplayStatics::GetAllActorsOfClass(World, ALevelQuestManager::StaticClass(), FoundManagers);

                if (FoundManagers.Num() > 0)
                {
                    QuestManager = Cast<ALevelQuestManager>(FoundManagers[0]);
                }
            }
        }
    }

    return QuestManager;
}

#if WITH_EDITOR
void UEnhancedQuestComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = PropertyChangedEvent.GetPropertyName();

    // bIsQuestRelated나 QuestStepIndex가 변경되면 정보 업데이트
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UEnhancedQuestComponent, bIsQuestRelated) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(UEnhancedQuestComponent, QuestStepIndex))
    {
        UpdateQuestInfo();
    }
}

void UEnhancedQuestComponent::UpdateQuestInfo()
{
    // 퀘스트 관련이 아니면 정보 초기화
    if (!bIsQuestRelated)
    {
        CurrentLevelInfo = "이 오브젝트는 퀘스트와 관련없습니다.";
        CurrentSubSteps.Empty();
        SelectedStepPreview = "";
        return;
    }

    // 데이터 테이블이 없으면 경고
    if (!QuestDataTable)
    {
        CurrentLevelInfo = "퀘스트 데이터 테이블이 설정되지 않았습니다.";
        return;
    }

    // 에디터에서만 실행되는 안전한 코드
    UWorld* EditorWorld = nullptr;

    // 더 안전한 방법으로 에디터 월드 가져오기
    if (GetOwner())
    {
        EditorWorld = GetOwner()->GetWorld();
    }

    if (EditorWorld)
    {
        CurrentLevelName = EditorWorld->GetMapName();
        CurrentLevelName.RemoveFromStart(EditorWorld->StreamingLevelsPrefix);
    }
    else
    {
        CurrentLevelName = "Unknown";
    }

    // 데이터 테이블에서 현재 레벨 정보 찾기
    FLevelInfo* LevelData = nullptr;
    TArray<FName> RowNames = QuestDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FLevelInfo* Row = QuestDataTable->FindRow<FLevelInfo>(RowName, TEXT(""));
        if (Row && Row->LevelName == CurrentLevelName)
        {
            LevelData = Row;
            break;
        }
    }

    if (LevelData)
    {
        // 레벨 정보 업데이트
        CurrentLevelInfo = FString::Printf(TEXT("레벨: %s\n목표: %s\n설명: %s"),
            *LevelData->LevelName,
            *LevelData->MainObjective,
            *LevelData->Description);

        CurrentSubSteps = LevelData->SubSteps;

        // 선택된 스텝 미리보기
        if (QuestStepIndex >= 0 && QuestStepIndex < CurrentSubSteps.Num())
        {
            SelectedStepPreview = FString::Printf(TEXT("스텝 %d: %s"),
                QuestStepIndex,
                *CurrentSubSteps[QuestStepIndex]);
        }
        else
        {
            SelectedStepPreview = "잘못된 스텝 인덱스입니다.";
        }
    }
    else
    {
        CurrentLevelInfo = FString::Printf(TEXT("레벨 '%s'의 퀘스트 정보를 찾을 수 없습니다."), *CurrentLevelName);
        CurrentSubSteps.Empty();
        SelectedStepPreview = "";
    }
}
#endif