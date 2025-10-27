#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FPlayerSaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FString CurrentLevel = TEXT("Level_Main_0");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FRotator PlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FString> InventoryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FString, int32> ItemQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FString CurrentEquippedItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 CurrentSelectedSlot = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    bool bHasFlashlight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    bool bCanRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    float MovementSpeed = 400.0f;

    FPlayerSaveData()
    {
        CurrentLevel = TEXT("Level_Main_0");
        PlayerLocation = FVector::ZeroVector;
        PlayerRotation = FRotator::ZeroRotator;
        CurrentEquippedItem = TEXT("");
        CurrentSelectedSlot = 0;
        MaxInventorySize = 20;
        bHasFlashlight = false;
        bCanRun = true;
        MovementSpeed = 400.0f;
    }
};