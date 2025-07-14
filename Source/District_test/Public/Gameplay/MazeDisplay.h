// MazeDisplay.h - 모든 기능 완전 버전
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GridMazeManager.h"
#include "MazeDisplay.generated.h"

UENUM(BlueprintType)
enum class EDisplayState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Countdown   UMETA(DisplayName = "Countdown"),
    Playing     UMETA(DisplayName = "Playing"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Custom      UMETA(DisplayName = "Custom Message")
};

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API AMazeDisplay : public AActor
{
    GENERATED_BODY()

public:
    AMazeDisplay();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ====== Basic Components ======
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DisplayMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UTextRenderComponent* TimeText;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UTextRenderComponent* StatusText;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UTextRenderComponent* ProgressText;  // 새로 추가

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UPointLightComponent* DisplayLight;

    // ====== Connection Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection Settings")
    bool bAutoConnectToManager = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection Settings")
    AGridMazeManager* ManualManager;

    // ====== Display Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings")
    bool bShowTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings")
    bool bShowStatus = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings")
    bool bShowProgress = true;  // 새로 추가

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings")
    bool bShowMilliseconds = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings")
    float TimeTextSize = 32.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings")
    float StatusTextSize = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Settings")
    float ProgressTextSize = 20.0f;  // 새로 추가

    // ====== Countdown Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Countdown Settings")
    bool bEnableCountdown = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Countdown Settings")
    int32 CountdownDuration = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Countdown Settings")
    FString CountdownFormat = TEXT("00:0{0}");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Countdown Settings")
    FLinearColor CountdownColor = FLinearColor::Yellow;

    // ====== Color Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor ReadyColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor PlayingColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor WarningColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor DangerColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor SuccessColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor FailedColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor ProgressColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);  // 사이안 색상

    // ====== Message Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Settings")
    FString ReadyMessage = TEXT("READY");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Settings")
    FString PlayingMessage = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Settings")
    FString SuccessMessage = TEXT("SUCCESS!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Settings")
    FString FailedMessage = TEXT("FAILED!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Settings")
    FString TimeOutMessage = TEXT("TIME OUT!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Settings")
    FString CountdownReadyMessage = TEXT("READY");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message Settings")
    FString CountdownStartMessage = TEXT("START!");

    // ====== Warning Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Settings")
    float WarningTimeThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Settings")
    float DangerTimeThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Settings")
    bool bBlinkOnWarning = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Settings")
    float BlinkSpeed = 2.0f;

    // ====== Animation Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableSuccessAnimation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SuccessAnimationDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableFailAnimation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FailAnimationDuration = 2.0f;

    // ====== Current State ======
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDisplayState CurrentDisplayState = EDisplayState::Ready;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentProgress = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 MaxProgress = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsBlinking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsCountingDown = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentCountdown = 0;

public:
    // ====== Blueprint Callable Functions ======
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void UpdateTime(float TimeRemaining);

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void UpdateProgress(int32 Current, int32 Max);  // 새로 추가

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void ShowMessage(const FString& Message, FLinearColor Color = FLinearColor::White);

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void SetDisplayState(EDisplayState NewState);

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void StartCountdown();  // 새로 추가

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void StopCountdown();   // 새로 추가

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void ClearDisplay();

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void StartBlinking();

    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void StopBlinking();

    // ====== Connection Management ======
    UFUNCTION(BlueprintCallable, Category = "Connection")
    void ConnectToManager(AGridMazeManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Connection")
    void DisconnectFromManager();

    UFUNCTION(BlueprintCallable, Category = "Connection")
    void AutoConnectToManager();

    // ====== Settings Functions ======
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTimeTextSize(float NewSize);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetStatusTextSize(float NewSize);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetProgressTextSize(float NewSize);  // 새로 추가

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetDisplayColors(FLinearColor Ready, FLinearColor Playing, FLinearColor Success, FLinearColor Failed);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetDisplayMessages(const FString& Ready, const FString& Success, const FString& Failed);

    // ====== Information Query ======
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsConnectedToManager() const { return ConnectedManager != nullptr; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    AGridMazeManager* GetConnectedManager() const { return ConnectedManager; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FString FormatTime(float TimeInSeconds);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FString FormatProgress(int32 Current, int32 Max);  // 새로 추가

    // ====== Blueprint Events ======
    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnDisplayStateChanged(EDisplayState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnTimeWarning(float TimeRemaining);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnTimeDanger(float TimeRemaining);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnCountdownStep(int32 CountdownNumber);  // 새로 추가

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnCountdownFinished();  // 새로 추가

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnConnectedToManager(AGridMazeManager* Manager);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnDisconnectedFromManager();

    // ====== Blueprint Overrideable Functions ======
    UFUNCTION(BlueprintNativeEvent, Category = "Custom Display")
    void CustomUpdateDisplay(float TimeRemaining, EPuzzleState PuzzleState);
    virtual void CustomUpdateDisplay_Implementation(float TimeRemaining, EPuzzleState PuzzleState) {}

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Display")
    FString CustomFormatTime(float TimeInSeconds);
    virtual FString CustomFormatTime_Implementation(float TimeInSeconds);

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Display")
    FLinearColor GetTimeColor(float TimeRemaining, float TotalTime);
    virtual FLinearColor GetTimeColor_Implementation(float TimeRemaining, float TotalTime);

protected:
    // Manager event callbacks
    UFUNCTION()
    void OnPuzzleStateChanged(EPuzzleState NewState);

    UFUNCTION()
    void OnTimerUpdate(float TimeRemaining);

private:
    UPROPERTY()
    AGridMazeManager* ConnectedManager;

    // Internal variables
    float BlinkTimer = 0.0f;
    FTimerHandle BlinkTimerHandle;
    FTimerHandle MessageTimerHandle;
    FTimerHandle CountdownTimerHandle;  // 새로 추가

    bool bWarningTriggered = false;
    bool bDangerTriggered = false;

    // Internal functions
    void UpdateVisuals();
    void UpdateTextColors(FLinearColor Color);
    void UpdateLightColor(FLinearColor Color);
    void HandleTimeWarnings(float TimeRemaining, float TotalTime);
    void UpdateCountdown();  // 새로 추가
    void SetupProgressText();  // 새로 추가
};