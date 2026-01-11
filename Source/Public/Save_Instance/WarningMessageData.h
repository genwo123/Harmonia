#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WarningMessageData.generated.h"

UENUM(BlueprintType)
enum class EWarningMessageType : uint8
{
    AlreadyHoldingObject    UMETA(DisplayName = "AlreadyHoldingObject"),
    InventoryFull           UMETA(DisplayName = "InventoryFull"),
    RequiredItem            UMETA(DisplayName = "RequiredItem"),
    NoteOpened              UMETA(DisplayName = "NoteOpened"),
    PuzzleLocked            UMETA(DisplayName = "PuzzleLocked"),
    DoorLocked              UMETA(DisplayName = "DoorLocked"),
    Custom                  UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FWarningMessageData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    EWarningMessageType MessageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FText MessageText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLinearColor TextColor;

    FWarningMessageData()
        : MessageType(EWarningMessageType::Custom)
        , MessageText(FText::FromString(TEXT("Warning Message")))
        , DisplayDuration(3.0f)
        , TextColor(FLinearColor::Yellow)
    {
    }
};