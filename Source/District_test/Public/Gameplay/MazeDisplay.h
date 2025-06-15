// MazeDisplay.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "GridMazeManager.h"
#include "MazeDisplay.generated.h"

UENUM(BlueprintType)
enum class EDisplayStyle : uint8
{
    Modern      UMETA(DisplayName = "Modern LCD"),      // 모던 LCD 스타일
    Retro       UMETA(DisplayName = "Retro LED"),       // 레트로 LED 스타일
    Industrial  UMETA(DisplayName = "Industrial"),      // 산업용 스타일
    Hologram    UMETA(DisplayName = "Hologram")         // 홀로그램 스타일
};

UENUM(BlueprintType)
enum class EDisplayMode : uint8
{
    TimeOnly        UMETA(DisplayName = "Time Only"),           // 시간만 표시
    TimeAndAttempts UMETA(DisplayName = "Time + Attempts"),     // 시간 + 시도횟수
    StatusOnly      UMETA(DisplayName = "Status Only"),         // 상태만 표시
    Full            UMETA(DisplayName = "Full Info")            // 모든 정보 표시
};

UCLASS()
class DISTRICT_TEST_API AMazeDisplay : public AActor
{
    GENERATED_BODY()

public:
    AMazeDisplay();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ====== COMPONENTS ======
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FrameMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ScreenMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* TimeText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* InfoText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* ScreenLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* BackLight;

    // ====== MANAGER REFERENCE ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    AGridMazeManager* ConnectedManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    bool bAutoFindManager = true;

    // ====== DISPLAY SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    EDisplayStyle DisplayStyle = EDisplayStyle::Modern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    EDisplayMode DisplayMode = EDisplayMode::TimeOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bShowWhenInactive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString InactiveMessage = TEXT("READY");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString SuccessMessage = TEXT("CLEAR!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString FailMessage = TEXT("TIME OUT!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    FString FreePlayMessage = TEXT("FREE PLAY");

    // ====== VISUAL SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor NormalColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // 초록색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor WarningColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // 노란색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor DangerColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // 빨간색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor SuccessColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // 밝은 초록

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor InactiveColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f); // 회색

    // ====== TIME SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float WarningTimeThreshold = 30.0f; // 30초 남으면 경고

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DangerTimeThreshold = 10.0f; // 10초 남으면 위험

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bShowMilliseconds = false;

    // ====== TEXT SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
    float TimeTextSize = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
    float StatusTextSize = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
    float InfoTextSize = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
    class UFont* DisplayFont;

    // ====== ANIMATION SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bEnableBlinking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlinkSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bEnablePulse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PulseSpeed = 2.0f;

    // ====== LIGHT SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float ScreenLightIntensity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float BackLightIntensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float LightRadius = 200.0f;

    // ====== AUDIO SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bPlaySounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* UpdateSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* WarningSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* SuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* FailSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SoundVolume = 1.0f;

public:
    // ====== PUBLIC FUNCTIONS ======
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void SetDisplayActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void UpdateDisplay(float TimeRemaining, int32 AttemptCount, EMazeState MazeState);

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void ShowMessage(const FString& Message, FLinearColor Color = FLinearColor::White, float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void ClearDisplay();

    UFUNCTION(BlueprintCallable, Category = "Connection")
    void ConnectToManager(AGridMazeManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Connection")
    void DisconnectFromManager();

    // ====== UTILITY FUNCTIONS ======
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString FormatTime(float TimeInSeconds);

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void StartBlinking();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void StopBlinking();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void StartPulse();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void StopPulse();

protected:
    // ====== INTERNAL VARIABLES ======
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsBlinking = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPulsing = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float BlinkTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float PulseTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EMazeState LastMazeState = EMazeState::Ready;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastTimeRemaining = 0.0f;

    // Material instances
    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    UMaterialInstanceDynamic* ScreenMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    UMaterialInstanceDynamic* FrameMaterial;

    // Timer handles
    FTimerHandle BlinkTimerHandle;
    FTimerHandle MessageTimerHandle;

    // ====== INTERNAL FUNCTIONS ======
    void InitializeComponents();
    void SetupDisplayStyle();
    void FindConnectedManager();

    void UpdateTextContent(float TimeRemaining, int32 AttemptCount, EMazeState MazeState);
    void UpdateVisualStyle(float TimeRemaining, EMazeState MazeState);
    void UpdateLighting(FLinearColor Color);

    FLinearColor GetStateColor(float TimeRemaining, EMazeState MazeState);
    void PlayDisplaySound(USoundBase* Sound);

    void UpdateBlinking(float DeltaTime);
    void UpdatePulse(float DeltaTime);

    FString GetDisplayText(float TimeRemaining, int32 AttemptCount, EMazeState MazeState);
    FString GetStatusText(EMazeState MazeState);
    FString GetInfoText(int32 AttemptCount, EMazeState MazeState);

    // ====== MANAGER EVENTS ======
    UFUNCTION()
    void OnMazeDataReceived(float TimeRemaining, int32 AttemptCount, EMazeState CurrentState);

    UFUNCTION()
    void OnMazeCompleted(bool bSuccess);

    // ====== BLUEPRINT EVENTS ======
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDisplayActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnDisplayDeactivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnTimeWarning(float TimeRemaining);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnMazeStateChanged(EMazeState NewState);
};