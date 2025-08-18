#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FPlayerSaveData
{
    GENERATED_BODY()

    // 위치 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FString CurrentLevel = TEXT("Level_Main_1");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FRotator PlayerRotation = FRotator::ZeroRotator;

    // 인벤토리 시스템
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FString> InventoryItems; // ["Key_Red", "Tool_Flashlight", "Note_Tutorial"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FString, int32> ItemQuantities; // 아이템별 수량 {"Key_Red": 1, "Coin": 5}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FString CurrentEquippedItem; // 현재 들고 있는 아이템

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySize = 20; // 최대 인벤토리 크기

    // 플레이어 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    bool bHasFlashlight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    bool bCanRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    float MovementSpeed = 400.0f;

    // 생성자
    FPlayerSaveData()
    {
        CurrentLevel = TEXT("Level_Main_1");
        PlayerLocation = FVector::ZeroVector;
        PlayerRotation = FRotator::ZeroRotator;
        CurrentEquippedItem = TEXT("");
        MaxInventorySize = 20;
        bHasFlashlight = false;
        bCanRun = true;
        MovementSpeed = 400.0f;
    }
};
