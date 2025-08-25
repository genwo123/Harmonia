#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PlayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FPlayerSaveData
{
    GENERATED_BODY()

    // ��ġ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FString CurrentLevel = TEXT("Level_Main_1");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Location")
    FRotator PlayerRotation = FRotator::ZeroRotator;

    // �κ��丮 �ý���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FString> InventoryItems; // ["Key_Red", "Tool_Flashlight", "Note_Tutorial"]

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FString, int32> ItemQuantities; // �����ۺ� ���� {"Key_Red": 1, "Coin": 5}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FString CurrentEquippedItem; // ���� ��� �ִ� ������

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySize = 20; // �ִ� �κ��丮 ũ��

    // �÷��̾� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    bool bHasFlashlight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    bool bCanRun = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    float MovementSpeed = 400.0f;

    // ������
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
