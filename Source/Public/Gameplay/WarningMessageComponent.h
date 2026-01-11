#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Save_Instance/WarningMessageData.h"
#include "WarningMessageComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWarningMessageReceived,
    FText, Message,
    float, Duration,
    FLinearColor, Color);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UWarningMessageComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWarningMessageComponent();

    UPROPERTY(BlueprintAssignable, Category = "Warning Message")
    FOnWarningMessageReceived OnWarningMessageReceived;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Message")
    UDataTable* MessageDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Message|Default Settings")
    float DefaultDisplayDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Message|Default Settings")
    FLinearColor DefaultTextColor = FLinearColor::Yellow;

    UFUNCTION(BlueprintCallable, Category = "Warning Message")
    void ShowMessage(EWarningMessageType MessageType);

    UFUNCTION(BlueprintCallable, Category = "Warning Message")
    void ShowCustomMessage(const FText& Message, float Duration = 3.0f, FLinearColor Color = FLinearColor::Yellow);

    UFUNCTION(BlueprintCallable, Category = "Warning Message")
    void ShowMessageWithCustomSettings(EWarningMessageType MessageType, float OverrideDuration = -1.0f, FLinearColor OverrideColor = FLinearColor(0, 0, 0, 0));

    UFUNCTION(BlueprintCallable, Category = "Warning Message")
    void HideMessage();

protected:
    FTimerHandle MessageTimerHandle;
};