#include "Character/UniaWaitSpot.h"
#include "Kismet/GameplayStatics.h"

AUniaWaitSpot::AUniaWaitSpot()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

FVector AUniaWaitSpot::GetWaitLocation() const
{
    return GetActorLocation();
}

bool AUniaWaitSpot::IsValidForLevel(const FString& CurrentLevelName) const
{
    if (LevelName.IsEmpty())
    {
        return true;
    }
    return LevelName.Equals(CurrentLevelName, ESearchCase::IgnoreCase) ||
        CurrentLevelName.Contains(LevelName, ESearchCase::IgnoreCase) ||
        LevelName.Contains(CurrentLevelName, ESearchCase::IgnoreCase);
}