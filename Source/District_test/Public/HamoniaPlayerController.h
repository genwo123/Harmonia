// HamoniaPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HamoniaPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class DISTRICT_TEST_API AHamoniaPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHamoniaPlayerController();

    // Enhanced Input Mapping Context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    // 입력 우선순위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    int32 InputMappingPriority = 0;

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
};