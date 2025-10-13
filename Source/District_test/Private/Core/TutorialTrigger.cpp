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
    MessageGroupIndex = 0;
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

bool ATutorialTrigger::LoadMessageGroupFromDataTable()
{
    if (!TutorialMessageDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("TutorialMessageDataTable NO"));
        return false;
    }

    FString RowName = FString::Printf(TEXT("%d"), MessageGroupIndex);
    FTutorialMessageData* MessageData = TutorialMessageDataTable->FindRow<FTutorialMessageData>(
        FName(*RowName), TEXT("TutorialMessageGroup"));

    if (MessageData)
    {
        CurrentMessageData = *MessageData;

        if (CurrentMessageData.Messages.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("NO Message"), MessageGroupIndex);
            return false;
        }

        while (CurrentMessageData.DisplayDurations.Num() < CurrentMessageData.Messages.Num())
        {
            CurrentMessageData.DisplayDurations.Add(3.0f);
        }

        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No MessageGroup"), MessageGroupIndex);
        return false;
    }
}

void ATutorialTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
    if (!PlayerCharacter)
        return;

    if (!bIsActive || (bIsOneTimeUse && bHasTriggered))
        return;

    StartTutorial();
}

void ATutorialTrigger::StartTutorial()
{
    if (bHasTriggered && bIsOneTimeUse)
        return;

    if (!LoadMessageGroupFromDataTable())
        return;

    bHasTriggered = true;
    CurrentMessageIndex = 0;

    ShowCurrentMessage();
}

void ATutorialTrigger::ShowCurrentMessage()
{
    if (CurrentMessageIndex >= CurrentMessageData.Messages.Num())
    {
        EndTutorial();
        return;
    }

    FText MessageText = CurrentMessageData.Messages[CurrentMessageIndex];
    float Duration = CurrentMessageData.DisplayDurations[CurrentMessageIndex];

    UTutorialWidget* Widget = Cast<UTutorialWidget>(GetOrCreateTutorialWidget());
    if (Widget)
    {
        Widget->ShowMessage(MessageText, Duration);
    }

    if (Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            MessageTimerHandle,
            [this]()
            {
                CurrentMessageIndex++;
                ShowNextMessage();
            },
            Duration,
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