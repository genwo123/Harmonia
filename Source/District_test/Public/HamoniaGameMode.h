#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HamoniaGameMode.generated.h"

UCLASS()
class DISTRICT_TEST_API AHamoniaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHamoniaGameMode();

    virtual void StartPlay() override;
    virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
};