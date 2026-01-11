#include "Gameplay/WarningMessageComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UWarningMessageComponent::UWarningMessageComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MessageDataTable = nullptr;
    DefaultDisplayDuration = 3.0f;
    DefaultTextColor = FLinearColor::Yellow;
}

void UWarningMessageComponent::ShowMessage(EWarningMessageType MessageType)
{
    if (!MessageDataTable)
    {
        ShowCustomMessage(
            FText::FromString(TEXT("Warning Message")),
            DefaultDisplayDuration,
            DefaultTextColor
        );
        return;
    }

    FString RowName = StaticEnum<EWarningMessageType>()->GetNameStringByValue((int64)MessageType);
    FWarningMessageData* MessageData = MessageDataTable->FindRow<FWarningMessageData>(FName(*RowName), TEXT(""));

    if (MessageData)
    {
        ShowCustomMessage(MessageData->MessageText, MessageData->DisplayDuration, MessageData->TextColor);
    }
    else
    {
        ShowCustomMessage(
            FText::FromString(TEXT("Message Not Found")),
            DefaultDisplayDuration,
            DefaultTextColor
        );
    }
}

void UWarningMessageComponent::ShowMessageWithCustomSettings(
    EWarningMessageType MessageType,
    float OverrideDuration,
    FLinearColor OverrideColor)
{
    if (!MessageDataTable)
    {
        ShowCustomMessage(
            FText::FromString(TEXT("Warning Message")),
            OverrideDuration > 0 ? OverrideDuration : DefaultDisplayDuration,
            OverrideColor.A > 0 ? OverrideColor : DefaultTextColor
        );
        return;
    }

    FString RowName = StaticEnum<EWarningMessageType>()->GetNameStringByValue((int64)MessageType);
    FWarningMessageData* MessageData = MessageDataTable->FindRow<FWarningMessageData>(FName(*RowName), TEXT(""));

    if (MessageData)
    {
        float FinalDuration = OverrideDuration > 0 ? OverrideDuration : MessageData->DisplayDuration;
        FLinearColor FinalColor = OverrideColor.A > 0 ? OverrideColor : MessageData->TextColor;
        ShowCustomMessage(MessageData->MessageText, FinalDuration, FinalColor);
    }
    else
    {
        ShowCustomMessage(
            FText::FromString(TEXT("Message Not Found")),
            OverrideDuration > 0 ? OverrideDuration : DefaultDisplayDuration,
            OverrideColor.A > 0 ? OverrideColor : DefaultTextColor
        );
    }
}

void UWarningMessageComponent::ShowCustomMessage(const FText& Message, float Duration, FLinearColor Color)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (World->GetTimerManager().IsTimerActive(MessageTimerHandle))
    {
        World->GetTimerManager().ClearTimer(MessageTimerHandle);
    }

    OnWarningMessageReceived.Broadcast(Message, Duration, Color);

    World->GetTimerManager().SetTimer(
        MessageTimerHandle,
        this,
        &UWarningMessageComponent::HideMessage,
        Duration,
        false
    );
}

void UWarningMessageComponent::HideMessage()
{
    OnWarningMessageReceived.Broadcast(FText::GetEmpty(), 0.0f, FLinearColor::White);
}