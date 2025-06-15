// MazeDisplay.cpp
#include "Gameplay/MazeDisplay.h"
#include "Gameplay/GridMazeManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMazeDisplay::AMazeDisplay()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create scene root
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    InitializeComponents();
}

void AMazeDisplay::BeginPlay()
{
    Super::BeginPlay();

    SetupDisplayStyle();

    if (bAutoFindManager && !ConnectedManager)
    {
        FindConnectedManager();
    }

    SetDisplayActive(false);
}

void AMazeDisplay::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsActive && ConnectedManager)
    {
        EMazeState CurrentState = ConnectedManager->GetCurrentState();
        float TimeRemaining = ConnectedManager->GetTimeRemaining();
        int32 AttemptCount = ConnectedManager->GetAttemptCount();

        UpdateDisplay(TimeRemaining, AttemptCount, CurrentState);
    }

    if (bIsBlinking)
    {
        UpdateBlinking(DeltaTime);
    }

    if (bIsPulsing)
    {
        UpdatePulse(DeltaTime);
    }
}

void AMazeDisplay::InitializeComponents()
{
    // Frame mesh
    FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
    FrameMesh->SetupAttachment(SceneRoot);

    // Screen mesh
    ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
    ScreenMesh->SetupAttachment(FrameMesh);
    ScreenMesh->SetRelativeLocation(FVector(1.0f, 0.0f, 0.0f)); // 약간 앞으로

    // Time text
    TimeText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TimeText"));
    TimeText->SetupAttachment(ScreenMesh);
    TimeText->SetRelativeLocation(FVector(2.0f, 0.0f, 20.0f));
    TimeText->SetTextRenderColor(FColor::Green);
    TimeText->SetHorizontalAlignment(EHTA_Center);
    TimeText->SetVerticalAlignment(EVRTA_TextCenter);
    TimeText->SetWorldSize(TimeTextSize);

    // Status text
    StatusText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusText"));
    StatusText->SetupAttachment(ScreenMesh);
    StatusText->SetRelativeLocation(FVector(2.0f, 0.0f, 0.0f));
    StatusText->SetTextRenderColor(FColor::Green);
    StatusText->SetHorizontalAlignment(EHTA_Center);
    StatusText->SetVerticalAlignment(EVRTA_TextCenter);
    StatusText->SetWorldSize(StatusTextSize);

    // Info text
    InfoText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InfoText"));
    InfoText->SetupAttachment(ScreenMesh);
    InfoText->SetRelativeLocation(FVector(2.0f, 0.0f, -20.0f));
    InfoText->SetTextRenderColor(FColor::Green);
    InfoText->SetHorizontalAlignment(EHTA_Center);
    InfoText->SetVerticalAlignment(EVRTA_TextCenter);
    InfoText->SetWorldSize(InfoTextSize);

    // Screen light
    ScreenLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ScreenLight"));
    ScreenLight->SetupAttachment(ScreenMesh);
    ScreenLight->SetRelativeLocation(FVector(10.0f, 0.0f, 0.0f));
    ScreenLight->SetIntensity(ScreenLightIntensity);
    ScreenLight->SetAttenuationRadius(LightRadius);
    ScreenLight->SetLightColor(NormalColor);

    // Back light
    BackLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BackLight"));
    BackLight->SetupAttachment(FrameMesh);
    BackLight->SetRelativeLocation(FVector(-10.0f, 0.0f, 0.0f));
    BackLight->SetIntensity(BackLightIntensity);
    BackLight->SetAttenuationRadius(LightRadius * 0.5f);
    BackLight->SetLightColor(InactiveColor);
}

void AMazeDisplay::SetupDisplayStyle()
{
    // Create dynamic materials
    if (ScreenMesh && ScreenMesh->GetMaterial(0))
    {
        ScreenMaterial = ScreenMesh->CreateDynamicMaterialInstance(0);
    }

    if (FrameMesh && FrameMesh->GetMaterial(0))
    {
        FrameMaterial = FrameMesh->CreateDynamicMaterialInstance(0);
    }

    // Apply display style settings
    switch (DisplayStyle)
    {
    case EDisplayStyle::Modern:
        // 모던 LCD 스타일
        NormalColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
        if (DisplayFont == nullptr)
        {
            // 기본 폰트 사용
        }
        break;

    case EDisplayStyle::Retro:
        // 레트로 LED 스타일
        NormalColor = FLinearColor(1.0f, 0.2f, 0.0f, 1.0f);
        TimeTextSize *= 1.2f;
        StatusTextSize *= 1.2f;
        InfoTextSize *= 1.2f;
        break;

    case EDisplayStyle::Industrial:
        // 산업용 스타일
        NormalColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
        break;

    case EDisplayStyle::Hologram:
        // 홀로그램 스타일
        NormalColor = FLinearColor(0.0f, 0.8f, 1.0f, 0.8f);
        bEnablePulse = true;
        break;
    }

    // Apply font if set
    if (DisplayFont)
    {
        TimeText->SetFont(DisplayFont);
        StatusText->SetFont(DisplayFont);
        InfoText->SetFont(DisplayFont);
    }
}

void AMazeDisplay::SetDisplayActive(bool bActive)
{
    bIsActive = bActive;

    if (bActive)
    {
        OnDisplayActivated();
    }
    else
    {
        OnDisplayDeactivated();
        ClearDisplay();
    }

    UpdateLighting(bActive ? NormalColor : InactiveColor);
}

void AMazeDisplay::UpdateDisplay(float TimeRemaining, int32 AttemptCount, EMazeState MazeState)
{
    if (!bIsActive)
        return;

    // State change detection
    if (MazeState != LastMazeState)
    {
        LastMazeState = MazeState;
        OnMazeStateChanged(MazeState);
    }

    // Time warning detection
    if (TimeRemaining != LastTimeRemaining)
    {
        if (TimeRemaining <= WarningTimeThreshold && TimeRemaining > DangerTimeThreshold)
        {
            OnTimeWarning(TimeRemaining);
        }
        LastTimeRemaining = TimeRemaining;
    }

    UpdateTextContent(TimeRemaining, AttemptCount, MazeState);
    UpdateVisualStyle(TimeRemaining, MazeState);
}

void AMazeDisplay::UpdateTextContent(float TimeRemaining, int32 AttemptCount, EMazeState MazeState)
{
    FString TimeDisplayText;
    FString StatusDisplayText;
    FString InfoDisplayText;

    // Time display based on mode
    switch (DisplayMode)
    {
    case EDisplayMode::TimeOnly:
        TimeDisplayText = FormatTime(TimeRemaining);
        break;

    case EDisplayMode::TimeAndAttempts:
        TimeDisplayText = FormatTime(TimeRemaining);
        InfoDisplayText = FString::Printf(TEXT("ATTEMPTS: %d"), AttemptCount);
        break;

    case EDisplayMode::StatusOnly:
        StatusDisplayText = GetStatusText(MazeState);
        break;

    case EDisplayMode::Full:
        TimeDisplayText = FormatTime(TimeRemaining);
        StatusDisplayText = GetStatusText(MazeState);
        InfoDisplayText = FString::Printf(TEXT("ATTEMPTS: %d"), AttemptCount);
        break;
    }

    // Handle inactive state
    if (!bShowWhenInactive && MazeState == EMazeState::Ready)
    {
        StatusDisplayText = InactiveMessage;
        TimeDisplayText = TEXT("");
        InfoDisplayText = TEXT("");
    }

    // Set text content
    if (TimeText)
    {
        TimeText->SetText(FText::FromString(TimeDisplayText));
    }

    if (StatusText)
    {
        StatusText->SetText(FText::FromString(StatusDisplayText));
    }

    if (InfoText)
    {
        InfoText->SetText(FText::FromString(InfoDisplayText));
    }
}

void AMazeDisplay::UpdateVisualStyle(float TimeRemaining, EMazeState MazeState)
{
    FLinearColor CurrentColor = GetStateColor(TimeRemaining, MazeState);
    UpdateLighting(CurrentColor);

    // Update text colors
    FColor TextColor = CurrentColor.ToFColor(true);
    if (TimeText) TimeText->SetTextRenderColor(TextColor);
    if (StatusText) StatusText->SetTextRenderColor(TextColor);
    if (InfoText) InfoText->SetTextRenderColor(TextColor);

    // Start/stop visual effects based on state
    switch (MazeState)
    {
    case EMazeState::Success:
        if (bEnableBlinking) StartBlinking();
        if (bEnablePulse) StartPulse();
        PlayDisplaySound(SuccessSound);
        break;

    case EMazeState::Failed:
    case EMazeState::TimeOut:
        if (bEnableBlinking) StartBlinking();
        PlayDisplaySound(FailSound);
        break;

    case EMazeState::Running:
        if (TimeRemaining <= DangerTimeThreshold && bEnableBlinking)
        {
            StartBlinking();
        }
        break;

    default:
        StopBlinking();
        StopPulse();
        break;
    }
}

FLinearColor AMazeDisplay::GetStateColor(float TimeRemaining, EMazeState MazeState)
{
    switch (MazeState)
    {
    case EMazeState::Success:
        return SuccessColor;

    case EMazeState::Failed:
    case EMazeState::TimeOut:
        return DangerColor;

    case EMazeState::Running:
        if (TimeRemaining <= DangerTimeThreshold)
            return DangerColor;
        else if (TimeRemaining <= WarningTimeThreshold)
            return WarningColor;
        else
            return NormalColor;

    case EMazeState::Ready:
    default:
        return bShowWhenInactive ? NormalColor : InactiveColor;
    }
}

void AMazeDisplay::UpdateLighting(FLinearColor Color)
{
    if (ScreenLight)
    {
        ScreenLight->SetLightColor(Color);
    }

    if (BackLight)
    {
        BackLight->SetLightColor(Color * 0.3f);
    }

    // Update material emissive color
    if (ScreenMaterial)
    {
        ScreenMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
    }
}

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

FString AMazeDisplay::GetStatusText(EMazeState MazeState)
{
    switch (MazeState)
    {
    case EMazeState::Ready:
        return InactiveMessage;
    case EMazeState::Running:
        return TEXT("ACTIVE");
    case EMazeState::Success:
        return SuccessMessage;
    case EMazeState::Failed:
        return FailMessage;
    case EMazeState::TimeOut:
        return TEXT("TIME OUT!");
    default:
        return TEXT("");
    }
}

FString AMazeDisplay::GetInfoText(int32 AttemptCount, EMazeState MazeState)
{
    if (MazeState == EMazeState::Ready && AttemptCount == 0)
    {
        return FreePlayMessage;
    }
    return FString::Printf(TEXT("ATTEMPTS: %d"), AttemptCount);
}

void AMazeDisplay::ShowMessage(const FString& Message, FLinearColor Color, float Duration)
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(Message));
        StatusText->SetTextRenderColor(Color.ToFColor(true));
    }

    UpdateLighting(Color);

    // Clear message after duration
    GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle, [this]()
        {
            if (ConnectedManager)
            {
                UpdateDisplay(
                    ConnectedManager->GetTimeRemaining(),
                    ConnectedManager->GetAttemptCount(),
                    ConnectedManager->GetCurrentState()
                );
            }
        }, Duration, false);
}

void AMazeDisplay::ClearDisplay()
{
    if (TimeText) TimeText->SetText(FText::FromString(TEXT("")));
    if (StatusText) StatusText->SetText(FText::FromString(TEXT("")));
    if (InfoText) InfoText->SetText(FText::FromString(TEXT("")));

    UpdateLighting(InactiveColor);
    StopBlinking();
    StopPulse();
}

void AMazeDisplay::ConnectToManager(AGridMazeManager* Manager)
{
    if (ConnectedManager)
    {
        DisconnectFromManager();
    }

    ConnectedManager = Manager;

    if (ConnectedManager)
    {
        // Bind to manager events
        ConnectedManager->OnMazeDataUpdate.AddDynamic(this, &AMazeDisplay::OnMazeDataReceived);
        ConnectedManager->OnMazeComplete.AddDynamic(this, &AMazeDisplay::OnMazeCompleted);

        UE_LOG(LogTemp, Log, TEXT("MazeDisplay connected to manager"));
    }
}

void AMazeDisplay::DisconnectFromManager()
{
    if (ConnectedManager)
    {
        ConnectedManager->OnMazeDataUpdate.RemoveDynamic(this, &AMazeDisplay::OnMazeDataReceived);
        ConnectedManager->OnMazeComplete.RemoveDynamic(this, &AMazeDisplay::OnMazeCompleted);
        ConnectedManager = nullptr;

        UE_LOG(LogTemp, Log, TEXT("MazeDisplay disconnected from manager"));
    }
}

void AMazeDisplay::StartBlinking()
{
    if (!bEnableBlinking || bIsBlinking)
        return;

    bIsBlinking = true;
    BlinkTimer = 0.0f;
}

void AMazeDisplay::StopBlinking()
{
    if (bIsBlinking)
    {
        bIsBlinking = false;
        BlinkTimer = 0.0f;
    }
}

void AMazeDisplay::StartPulse()
{
    if (!bEnablePulse || bIsPulsing)
        return;

    bIsPulsing = true;
    PulseTimer = 0.0f;
}

void AMazeDisplay::StopPulse()
{
    if (bIsPulsing)
    {
        bIsPulsing = false;
        PulseTimer = 0.0f;
    }
}

void AMazeDisplay::UpdateBlinking(float DeltaTime)
{
    BlinkTimer += DeltaTime;

    float BlinkFactor = FMath::Sin(BlinkTimer * BlinkSpeed * PI) * 0.5f + 0.5f;
    float CurrentIntensity = FMath::Lerp(ScreenLightIntensity * 0.2f, ScreenLightIntensity, BlinkFactor);

    if (ScreenLight)
    {
        ScreenLight->SetIntensity(CurrentIntensity);
    }
}

void AMazeDisplay::UpdatePulse(float DeltaTime)
{
    PulseTimer += DeltaTime;

    float PulseFactor = FMath::Sin(PulseTimer * PulseSpeed * PI) * 0.3f + 0.7f;

    if (ScreenMaterial)
    {
        ScreenMaterial->SetScalarParameterValue(TEXT("EmissiveMultiplier"), PulseFactor);
    }
}

void AMazeDisplay::FindConnectedManager()
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

void AMazeDisplay::PlayDisplaySound(USoundBase* Sound)
{
    if (bPlaySounds && Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), SoundVolume);
    }
}

void AMazeDisplay::OnMazeDataReceived(float TimeRemaining, int32 AttemptCount, EMazeState CurrentState)
{
    if (bIsActive)
    {
        UpdateDisplay(TimeRemaining, AttemptCount, CurrentState);
    }
}

void AMazeDisplay::OnMazeCompleted(bool bSuccess)
{
    if (bSuccess)
    {
        ShowMessage(SuccessMessage, SuccessColor, 5.0f);
    }
    else
    {
        ShowMessage(FailMessage, DangerColor, 3.0f);
    }
}