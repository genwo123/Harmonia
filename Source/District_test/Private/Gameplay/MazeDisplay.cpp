// MazeDisplay.cpp
#include "Gameplay/MazeDisplay.h"
#include "Gameplay/GridMazeManager.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMazeDisplay::AMazeDisplay()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
    DisplayMesh->SetupAttachment(RootSceneComponent);

    TimeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimeText"));
    TimeText->SetupAttachment(DisplayMesh);
    TimeText->SetRelativeLocation(FVector(2.0f, 0.0f, 30.0f));
    TimeText->SetTextRenderColor(FColor::White);
    TimeText->SetHorizontalAlignment(EHTA_Center);
    TimeText->SetVerticalAlignment(EVRTA_TextCenter);
    TimeText->SetWorldSize(TimeTextSize);

    ProgressText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ProgressText"));
    ProgressText->SetupAttachment(DisplayMesh);
    ProgressText->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
    ProgressText->SetTextRenderColor(ProgressColor.ToFColor(true));
    ProgressText->SetHorizontalAlignment(EHTA_Center);
    ProgressText->SetVerticalAlignment(EVRTA_TextCenter);
    ProgressText->SetWorldSize(ProgressTextSize);

    StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
    StatusText->SetupAttachment(DisplayMesh);
    StatusText->SetRelativeLocation(FVector(2.0f, 0.0f, -30.0f));
    StatusText->SetTextRenderColor(FColor::White);
    StatusText->SetHorizontalAlignment(EHTA_Center);
    StatusText->SetVerticalAlignment(EVRTA_TextCenter);
    StatusText->SetWorldSize(StatusTextSize);

    DisplayLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DisplayLight"));
    DisplayLight->SetupAttachment(DisplayMesh);
    DisplayLight->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
    DisplayLight->SetIntensity(1000.0f);
    DisplayLight->SetAttenuationRadius(200.0f);
    DisplayLight->SetLightColor(ReadyColor);
}

void AMazeDisplay::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoConnectToManager)
    {
        AutoConnectToManager();
    }
    else if (ManualManager)
    {
        ConnectToManager(ManualManager);
    }

    SetDisplayState(EDisplayState::Ready);
    SetupProgressText();
    ShowMessage(ReadyMessage, ReadyColor);
}

void AMazeDisplay::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsBlinking)
    {
        BlinkTimer += DeltaTime;
        float BlinkFactor = FMath::Sin(BlinkTimer * BlinkSpeed * PI) * 0.5f + 0.5f;

        if (DisplayLight)
        {
            float CurrentIntensity = FMath::Lerp(200.0f, 1000.0f, BlinkFactor);
            DisplayLight->SetIntensity(CurrentIntensity);
        }
    }
}

// ============ 디스플레이 제어 ============

void AMazeDisplay::SetDisplayState(EDisplayState NewState)
{
    if (CurrentDisplayState != NewState)
    {
        CurrentDisplayState = NewState;

        FString Message;
        FLinearColor Color = FLinearColor::White; // 기본값 초기화 추가

        switch (NewState)
        {
        case EDisplayState::Ready:
            Message = ReadyMessage;
            Color = ReadyColor;
            StopBlinking();
            StopCountdown();
            break;

        case EDisplayState::Countdown:
            Message = CountdownReadyMessage;
            Color = CountdownColor;
            StopBlinking();
            if (bEnableCountdown)
            {
                StartCountdown();
            }
            break;

        case EDisplayState::Playing:
            Message = PlayingMessage;
            Color = PlayingColor;
            StopBlinking();
            StopCountdown();
            break;

        case EDisplayState::Success:
            Message = SuccessMessage;
            Color = SuccessColor;
            if (bEnableSuccessAnimation)
            {
                StartBlinking();
                GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, [this]()
                    {
                        StopBlinking();
                    }, SuccessAnimationDuration, false);
            }
            break;

        case EDisplayState::Failed:
            Message = FailedMessage;
            Color = FailedColor;
            if (bEnableFailAnimation)
            {
                StartBlinking();
                GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, [this]()
                    {
                        StopBlinking();
                    }, FailAnimationDuration, false);
            }
            break;

        case EDisplayState::Custom:
            // Custom 상태에서는 메시지/색상 변경 없음
            break;
        }

        if (NewState != EDisplayState::Custom)
        {
            ShowMessage(Message, Color);
        }

        OnDisplayStateChanged_Event.Broadcast(NewState);
        OnDisplayStateChanged(NewState);
    }
}


void AMazeDisplay::UpdateTime(float TimeRemaining)
{
    CurrentTime = TimeRemaining;

    if (bShowTime && TimeText)
    {
        FString TimeString = FormatTime(TimeRemaining);
        TimeText->SetText(FText::FromString(TimeString));
    }

    if (ConnectedManager)
    {
        float TotalTime = ConnectedManager->PuzzleTimeLimit;
        HandleTimeWarnings(TimeRemaining, TotalTime);

        FLinearColor TimeColor = GetTimeColor(TimeRemaining, TotalTime);
        UpdateTextColors(TimeColor);
        UpdateLightColor(TimeColor);
    }

    CustomUpdateDisplay(TimeRemaining, ConnectedManager ? ConnectedManager->GetCurrentState() : EPuzzleState::Ready);
}

void AMazeDisplay::UpdateProgress(int32 Current, int32 Max)
{
    CurrentProgress = Current;
    MaxProgress = Max;

    if (bShowProgress && ProgressText)
    {
        FString ProgressString = FormatProgress(Current, Max);
        ProgressText->SetText(FText::FromString(ProgressString));
        ProgressText->SetTextRenderColor(ProgressColor.ToFColor(true));
    }
}

void AMazeDisplay::ShowMessage(const FString& Message, FLinearColor Color)
{
    if (bShowStatus && StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
        StatusText->SetTextRenderColor(Color.ToFColor(true));
    }

    UpdateLightColor(Color);
}

void AMazeDisplay::ShowCustomMessage(const FString& Message)
{
    if (bShowStatus && StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
    }
}

void AMazeDisplay::ClearDisplay()
{
    if (TimeText)
    {
        TimeText->SetText(FText::FromString(TEXT("")));
    }

    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("")));
    }

    if (ProgressText)
    {
        ProgressText->SetText(FText::FromString(TEXT("")));
    }

    StopBlinking();
    StopCountdown();
    UpdateLightColor(FLinearColor::Black);
}

// ============ 카운트다운 제어 ============

void AMazeDisplay::StartCountdown()
{
    if (!bEnableCountdown) return;

    bIsCountingDown = true;
    CurrentCountdown = CountdownDuration;

    UpdateCountdown();

    GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, [this]()
        {
            UpdateCountdown();
        }, 1.0f, true);

    UE_LOG(LogTemp, Warning, TEXT("MazeDisplay: Countdown started from %d"), CountdownDuration);
}

void AMazeDisplay::StopCountdown()
{
    if (bIsCountingDown)
    {
        bIsCountingDown = false;
        GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
        UE_LOG(LogTemp, Warning, TEXT("MazeDisplay: Countdown stopped"));
    }
}

void AMazeDisplay::UpdateCountdown()
{
    if (!bIsCountingDown) return;

    if (CurrentCountdown > 0)
    {
        FString CountdownText = FString::Printf(TEXT("00:0%d"), CurrentCountdown);
        if (TimeText)
        {
            TimeText->SetText(FText::FromString(CountdownText));
            TimeText->SetTextRenderColor(CountdownColor.ToFColor(true));
        }

        ShowMessage(CountdownReadyMessage, CountdownColor);

        OnCountdownStep_Event.Broadcast(CurrentCountdown);
        OnCountdownStep(CurrentCountdown);

        CurrentCountdown--;

        UE_LOG(LogTemp, Warning, TEXT("MazeDisplay: Countdown %d"), CurrentCountdown + 1);
    }
    else
    {
        StopCountdown();

        ShowMessage(CountdownStartMessage, CountdownColor);
        if (TimeText)
        {
            TimeText->SetText(FText::FromString(CountdownStartMessage));
        }

        OnCountdownFinished_Event.Broadcast();
        OnCountdownFinished();

        if (ConnectedManager)
        {
            GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, [this]()
                {
                    SetDisplayState(EDisplayState::Playing);
                    if (ConnectedManager)
                    {
                        ConnectedManager->StartPuzzle();
                    }
                }, 1.0f, false);
        }

        UE_LOG(LogTemp, Warning, TEXT("MazeDisplay: Countdown finished - START!"));
    }
}

// ============ 애니메이션 제어 ============

void AMazeDisplay::StartBlinking()
{
    if (bBlinkOnWarning)
    {
        bIsBlinking = true;
        BlinkTimer = 0.0f;
    }
}

void AMazeDisplay::StopBlinking()
{
    if (bIsBlinking)
    {
        bIsBlinking = false;
        BlinkTimer = 0.0f;

        if (DisplayLight)
        {
            DisplayLight->SetIntensity(1000.0f);
        }
    }
}

// ============ 매니저 연결 ============

void AMazeDisplay::ConnectToManager(AGridMazeManager* Manager)
{
    if (ConnectedManager)
    {
        DisconnectFromManager();
    }

    ConnectedManager = Manager;

    if (ConnectedManager)
    {
        ConnectedManager->OnPuzzleStateChanged.AddDynamic(this, &AMazeDisplay::OnPuzzleStateChanged);
        ConnectedManager->OnTimerUpdate.AddDynamic(this, &AMazeDisplay::OnTimerUpdate);

        OnConnectedToManager(Manager);
    }
}

void AMazeDisplay::DisconnectFromManager()
{
    if (ConnectedManager)
    {
        ConnectedManager->OnPuzzleStateChanged.RemoveDynamic(this, &AMazeDisplay::OnPuzzleStateChanged);
        ConnectedManager->OnTimerUpdate.RemoveDynamic(this, &AMazeDisplay::OnTimerUpdate);

        OnDisconnectedFromManager();
        ConnectedManager = nullptr;
    }
}

void AMazeDisplay::AutoConnectToManager()
{
    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridMazeManager::StaticClass(), FoundManagers);

    if (FoundManagers.Num() > 0)
    {
        AGridMazeManager* Manager = Cast<AGridMazeManager>(FoundManagers[0]);
        if (Manager)
        {
            ConnectToManager(Manager);
        }
    }
}

// ============ 설정 변경 함수들 ============

void AMazeDisplay::SetTimeTextSize(float NewSize)
{
    TimeTextSize = NewSize;
    if (TimeText)
    {
        TimeText->SetWorldSize(NewSize);
    }
}

void AMazeDisplay::SetStatusTextSize(float NewSize)
{
    StatusTextSize = NewSize;
    if (StatusText)
    {
        StatusText->SetWorldSize(NewSize);
    }
}

void AMazeDisplay::SetProgressTextSize(float NewSize)
{
    ProgressTextSize = NewSize;
    if (ProgressText)
    {
        ProgressText->SetWorldSize(NewSize);
    }
}

void AMazeDisplay::SetDisplayColors(FLinearColor Ready, FLinearColor Playing, FLinearColor Success, FLinearColor Failed)
{
    ReadyColor = Ready;
    PlayingColor = Playing;
    SuccessColor = Success;
    FailedColor = Failed;
}

void AMazeDisplay::SetDisplayMessages(const FString& Ready, const FString& Success, const FString& Failed)
{
    ReadyMessage = Ready;
    SuccessMessage = Success;
    FailedMessage = Failed;
}

// ============ 정보 조회 함수들 ============

FString AMazeDisplay::FormatTime(float TimeInSeconds)
{
    if (TimeInSeconds <= 0.0f)
        return TEXT("00:00");

    int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
    int32 Seconds = FMath::FloorToInt(TimeInSeconds) % 60;

    if (bShowMilliseconds)
    {
        int32 Milliseconds = FMath::FloorToInt((TimeInSeconds - FMath::FloorToInt(TimeInSeconds)) * 100.0f);
        return FString::Printf(TEXT("%02d:%02d.%02d"), Minutes, Seconds, Milliseconds);
    }
    else
    {
        return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    }
}

FString AMazeDisplay::FormatProgress(int32 Current, int32 Max)
{
    return FString::Printf(TEXT("(%d/%d)"), Current, Max);
}

// ============ 이벤트 콜백들 ============

void AMazeDisplay::OnPuzzleStateChanged(EPuzzleState NewState)
{
    switch (NewState)
    {
    case EPuzzleState::Ready:
        SetDisplayState(EDisplayState::Ready);
        break;
    case EPuzzleState::Playing:
        SetDisplayState(EDisplayState::Playing);
        break;
    case EPuzzleState::Success:
        SetDisplayState(EDisplayState::Success);
        break;
    case EPuzzleState::Failed:
        SetDisplayState(EDisplayState::Failed);
        break;
    }
}

void AMazeDisplay::OnTimerUpdate(float TimeRemaining)
{
    UpdateTime(TimeRemaining);

    if (ConnectedManager)
    {
        UpdateProgress(ConnectedManager->GetCurrentPathIndex(), ConnectedManager->GetPathLength());
    }
}

// ============ BP 커스터마이즈 가능 함수들 ============

FString AMazeDisplay::CustomFormatTime_Implementation(float TimeInSeconds)
{
    return FormatTime(TimeInSeconds);
}

FLinearColor AMazeDisplay::GetTimeColor_Implementation(float TimeRemaining, float TotalTime)
{
    if (TimeRemaining <= DangerTimeThreshold)
    {
        return DangerColor;
    }
    else if (TimeRemaining <= WarningTimeThreshold)
    {
        return WarningColor;
    }
    else
    {
        return PlayingColor;
    }
}

// ============ 내부 함수들 ============

void AMazeDisplay::UpdateVisuals()
{
    if (TimeText)
    {
        TimeText->SetWorldSize(TimeTextSize);
    }

    if (StatusText)
    {
        StatusText->SetWorldSize(StatusTextSize);
    }

    if (ProgressText)
    {
        ProgressText->SetWorldSize(ProgressTextSize);
    }
}

void AMazeDisplay::UpdateTextColors(FLinearColor Color)
{
    FColor TextColor = Color.ToFColor(true);

    if (TimeText && !bIsCountingDown)
    {
        TimeText->SetTextRenderColor(TextColor);
    }
}

void AMazeDisplay::UpdateLightColor(FLinearColor Color)
{
    if (DisplayLight)
    {
        DisplayLight->SetLightColor(Color);
    }
}

void AMazeDisplay::HandleTimeWarnings(float TimeRemaining, float TotalTime)
{
    if (TimeRemaining <= WarningTimeThreshold && !bWarningTriggered)
    {
        bWarningTriggered = true;
        OnTimeWarning(TimeRemaining);
    }

    if (TimeRemaining <= DangerTimeThreshold && !bDangerTriggered)
    {
        bDangerTriggered = true;
        OnTimeDanger(TimeRemaining);

        if (bBlinkOnWarning)
        {
            StartBlinking();
        }
    }

    if (TimeRemaining > WarningTimeThreshold)
    {
        bWarningTriggered = false;
        bDangerTriggered = false;
        StopBlinking();
    }
}

void AMazeDisplay::SetupProgressText()
{
    if (ProgressText)
    {
        ProgressText->SetText(FText::FromString(TEXT("(0/0)")));
        ProgressText->SetTextRenderColor(ProgressColor.ToFColor(true));
        ProgressText->SetVisibility(bShowProgress);
    }
}