#include "Core/HamoniaGameMode.h"
#include "Character/HamoniaCharacter.h"
#include "Core/HamoniaPlayerController.h"
#include "Core/LevelQuestManager.h" // 추가
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AHamoniaGameMode::AHamoniaGameMode()
{
    DefaultPawnClass = AHamoniaCharacter::StaticClass();
    PlayerControllerClass = AHamoniaPlayerController::StaticClass();

    // 퀘스트 매니저 초기화
    QuestManager = nullptr;
}

void AHamoniaGameMode::StartPlay()
{
    Super::StartPlay();
    UE_LOG(LogTemp, Display, TEXT("HamoniaGameMode StartPlay"));

    // BP에서 퀘스트 매니저를 생성하므로 C++ 생성 코드 주석 처리
    UE_LOG(LogTemp, Warning, TEXT("=== Quest Manager will be created by BP ==="));

    // PlayerStart debugging (기존 코드)
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        APlayerStart* PlayerStart = *It;
        UE_LOG(LogTemp, Display, TEXT("Found PlayerStart at location: %s"), *PlayerStart->GetActorLocation().ToString());
    }
}

AActor* AHamoniaGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    AActor* FoundPlayerStart = Super::FindPlayerStart_Implementation(Player, IncomingName);
    if (FoundPlayerStart)
    {
        UE_LOG(LogTemp, Display, TEXT("Using PlayerStart: %s at location: %s"),
            *FoundPlayerStart->GetName(),
            *FoundPlayerStart->GetActorLocation().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found!"));
    }
    return FoundPlayerStart;
}

// === 퀘스트 시스템 함수들 ===

void AHamoniaGameMode::CompleteLevel()
{
    if (QuestManager)
    {
        QuestManager->CompleteCurrentLevel();
       
    }
}

void AHamoniaGameMode::StartNewLevel(const FString& LevelID)
{
    if (QuestManager)
    {
        QuestManager->StartLevel(LevelID);
        
    }
}

bool AHamoniaGameMode::IsLevelDone(const FString& LevelID)
{
    if (QuestManager)
    {
        return QuestManager->IsLevelCompleted(LevelID);
    }
    return false;
}