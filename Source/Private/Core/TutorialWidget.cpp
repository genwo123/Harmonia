#include "Core/TutorialWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UTutorialWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UTutorialWidget::ShowMessage(const FText& Message, float Duration)
{
    if (TutorialText)
    {
        TutorialText->SetText(Message);
        SetVisibility(ESlateVisibility::Visible);
    }

    if (Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            HideTimerHandle,
            this,
            &UTutorialWidget::HideMessage,
            Duration,
            false
        );
    }
}

void UTutorialWidget::HideMessage()
{
    SetVisibility(ESlateVisibility::Hidden);
    GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
}