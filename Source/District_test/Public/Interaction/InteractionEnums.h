// InteractionEnums.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractionEnums.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    Default     UMETA(DisplayName = "Default"),
    Pickup      UMETA(DisplayName = "Pickup"),
    Open        UMETA(DisplayName = "Open"),
    Read        UMETA(DisplayName = "Read"),
    Activate    UMETA(DisplayName = "Activate"),
    Place       UMETA(DisplayName = "Place"),
    Rotate      UMETA(DisplayName = "Rotate"),
    Push        UMETA(DisplayName = "Push"),  
    Use         UMETA(DisplayName = "Use")
};