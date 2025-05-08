#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HamoniaPlayerController.generated.h"

UCLASS()
class DISTRICT_TEST_API AHamoniaPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHamoniaPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
};