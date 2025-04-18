#include "HamoniaGameMode.h"
#include "HamoniaCharacter.h"
#include "HamoniaPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AHamoniaGameMode::AHamoniaGameMode()
{
    DefaultPawnClass = AHamoniaCharacter::StaticClass();
    PlayerControllerClass = AHamoniaPlayerController::StaticClass();
}

void AHamoniaGameMode::StartPlay()
{
    Super::StartPlay();

    UE_LOG(LogTemp, Display, TEXT("HamoniaGameMode StartPlay"));

    // PlayerStart À§Ä¡ µð¹ö±ë
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