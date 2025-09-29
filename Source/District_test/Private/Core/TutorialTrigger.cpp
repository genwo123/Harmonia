#include "Core/TutorialTrigger.h"
#include "Core/TutorialWidget.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATutorialTrigger::ATutorialTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->SetGenerateOverlapEvents(true);

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    bIsOneTimeUse = true;
    bIsActive = true;
    bShowDebugMesh = true;
    bHasTriggered = false;
    CurrentMessageIndex = 0;
    TutorialWidget = nullptr;

    FTutorialMessage DefaultMessage;
    DefaultMessage.MessageText = FText::FromString(TEXT("Test Trigger message"));
    DefaultMessage.DisplayDuration = 3.0f;
    TutorialMessages.Add(DefaultMessage);
}

void ATutorialTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATutorialTrigger::OnTriggerBeginOverlap);

    if (VisualMesh)
    {
        VisualMesh->SetVisibility(bShowDebugMesh);
    }

    GetOrCreateTutorialWidget();
}

void ATutorialTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!PlayerCharacter)
        return;

    if (!bIsActive || (bIsOneTimeUse && bHasTriggered))
        return;

    if (TutorialMessages.Num() == 0)
        return;

    StartTutorial();
}

void ATutorialTrigger::StartTutorial()
{
    if (bHasTriggered && bIsOneTimeUse)
        return;

    bHasTriggered = true;
    CurrentMessageIndex = 0;

    ShowCurrentMessage();
}

void ATutorialTrigger::ShowCurrentMessage()
{
    if (CurrentMessageIndex >= TutorialMessages.Num())
    {
        EndTutorial();
        return;
    }

    const FTutorialMessage& CurrentMessage = TutorialMessages[CurrentMessageIndex];

    UTutorialWidget* Widget = Cast<UTutorialWidget>(GetOrCreateTutorialWidget());
    if (Widget)
    {
        Widget->ShowMessage(CurrentMessage.MessageText, CurrentMessage.DisplayDuration);
    }

    if (CurrentMessage.DisplayDuration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            MessageTimerHandle,
            [this]()
            {
                CurrentMessageIndex++;
                ShowNextMessage();
            },
            CurrentMessage.DisplayDuration,
            false
        );
    }
}

void ATutorialTrigger::ShowNextMessage()
{
    ShowCurrentMessage();
}

void ATutorialTrigger::HideCurrentMessage()
{
    GetWorld()->GetTimerManager().ClearTimer(MessageTimerHandle);

    if (TutorialWidget)
    {
        UTutorialWidget* Widget = Cast<UTutorialWidget>(TutorialWidget);
        if (Widget)
        {
            Widget->HideMessage();
        }
    }
}

void ATutorialTrigger::EndTutorial()
{
    HideCurrentMessage();

    if (!bIsOneTimeUse)
    {
        CurrentMessageIndex = 0;
        bHasTriggered = false;
    }
}

void ATutorialTrigger::ResetTrigger()
{
    bHasTriggered = false;
    CurrentMessageIndex = 0;
    HideCurrentMessage();
}

UUserWidget* ATutorialTrigger::GetOrCreateTutorialWidget()
{
    if (!TutorialWidget && TutorialWidgetClass)
    {
        TutorialWidget = CreateWidget<UUserWidget>(GetWorld(), TutorialWidgetClass);
        if (TutorialWidget)
        {
            TutorialWidget->AddToViewport(100);
            TutorialWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    return TutorialWidget;
}