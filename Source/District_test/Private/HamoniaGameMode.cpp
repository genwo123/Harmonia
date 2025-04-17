// HamoniaGameMode.cpp
#include "HamoniaGameMode.h"
#include "HamoniaCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AHamoniaGameMode::AHamoniaGameMode()
{
    // HamoniaCharacter를 기본 폰으로 설정
    DefaultPawnClass = AHamoniaCharacter::StaticClass();
}

void AHamoniaGameMode::StartPlay()
{
    Super::StartPlay();

}

AActor* AHamoniaGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    // 모든 PlayerStart 찾기
    TArray<AActor*> FoundStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundStarts);

    for (AActor* Start : FoundStarts)
    {
        UE_LOG(LogTemp, Warning, TEXT("플레이어 스타트 찾음: %s, 위치: %s"),
            *Start->GetName(),
            *Start->GetActorLocation().ToString());
    }

    // 플레이어 스타트가 있으면 첫 번째 것 사용
    if (FoundStarts.Num() > 0)
    {
        return FoundStarts[0];
    }

    // 없으면 기본 로직 사용
    return Super::FindPlayerStart_Implementation(Player, IncomingName);
}