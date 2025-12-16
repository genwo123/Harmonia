// MazeDisplay.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "MazeDisplay.generated.h"

class AGridMazeManager;

UENUM(BlueprintType)
enum class EDisplayState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Countdown   UMETA(DisplayName = "Countdown"),
    Playing     UMETA(DisplayName = "Playing"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed"),
    Custom      UMETA(DisplayName = "Custom")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisplayStateChangedDelegate, EDisplayState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownStepDelegate, int32, RemainingSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownFinishedDelegate);

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API AMazeDisplay : public AActor
{
    GENERATED_BODY()

public:
    AMazeDisplay();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ============ 컴포넌트 ============

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* DisplayMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UTextRenderComponent* TimeText;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UTextRenderComponent* ProgressText;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UTextRenderComponent* StatusText;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UPointLightComponent* DisplayLight;

public:
    // ============ 색상 설정 (BP에서 편집 가능) ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|States")
    FLinearColor ReadyColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);  // 초록

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|States")
    FLinearColor CountdownColor = FLinearColor(1.0f, 1.0f, 0.2f, 1.0f);  // 노랑

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|States")
    FLinearColor PlayingColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);  // 파랑

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|States")
    FLinearColor SuccessColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);  // 밝은 초록

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|States")
    FLinearColor FailedColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);  // 빨강

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|Time Warning")
    FLinearColor WarningColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);  // 주황

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|Time Warning")
    FLinearColor DangerColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);  // 빨강

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Colors|Progress")
    FLinearColor ProgressColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);  // 흰색

    // ============ 텍스트 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Size")
    float TimeTextSize = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Size")
    float ProgressTextSize = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Size")
    float StatusTextSize = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Messages")
    FString ReadyMessage = TEXT("READY");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Messages")
    FString CountdownReadyMessage = TEXT("GET READY");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Messages")
    FString CountdownStartMessage = TEXT("START!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Messages")
    FString PlayingMessage = TEXT("Playing...");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Messages")
    FString SuccessMessage = TEXT("SUCCESS!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Text|Messages")
    FString FailedMessage = TEXT("FAILED!");

    // ============ 표시 옵션 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Options")
    bool bShowTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Options")
    bool bShowProgress = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Options")
    bool bShowStatus = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Options")
    bool bShowMilliseconds = false;

    // ============ 카운트다운 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Countdown Settings")
    bool bEnableCountdown = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Countdown Settings")
    int32 CountdownDuration = 3;

    // ============ 애니메이션 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableSuccessAnimation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SuccessAnimationDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableFailAnimation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FailAnimationDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bBlinkOnWarning = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BlinkSpeed = 4.0f;

    // ============ 경고 임계값 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Thresholds")
    float WarningTimeThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warning Thresholds")
    float DangerTimeThreshold = 10.0f;

    // ============ 매니저 연결 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    AGridMazeManager* ManualManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    bool bAutoConnectToManager = true;

    UPROPERTY(BlueprintReadOnly, Category = "Connection")
    AGridMazeManager* ConnectedManager;

    // ============ 현재 상태 ============

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDisplayState CurrentDisplayState = EDisplayState::Ready;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentProgress = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 MaxProgress = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsCountingDown = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentCountdown = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsBlinking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bWarningTriggered = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bDangerTriggered = false;

public:
    // ============ 이벤트 델리게이트 ============

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDisplayStateChangedDelegate OnDisplayStateChanged_Event;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCountdownStepDelegate OnCountdownStep_Event;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCountdownFinishedDelegate OnCountdownFinished_Event;

    // ============ BP에서 호출 가능한 주요 함수들 ============

    // 디스플레이 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void SetDisplayState(EDisplayState NewState);

    // 시간 업데이트
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void UpdateTime(float TimeRemaining);

    // 진행도 업데이트
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void UpdateProgress(int32 Current, int32 Max);

    // 메시지 표시
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void ShowMessage(const FString& Message, FLinearColor Color);

    // 커스텀 메시지 표시 (색상만)
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void ShowCustomMessage(const FString& Message);

    // 디스플레이 초기화
    UFUNCTION(BlueprintCallable, Category = "Display Control")
    void ClearDisplay();

    // ============ 카운트다운 제어 ============

    // 카운트다운 시작
    UFUNCTION(BlueprintCallable, Category = "Countdown")
    void StartCountdown();

    // 카운트다운 중지
    UFUNCTION(BlueprintCallable, Category = "Countdown")
    void StopCountdown();

    // ============ 애니메이션 제어 ============

    // 깜박임 시작
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StartBlinking();

    // 깜박임 중지
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopBlinking();

    // ============ 매니저 연결 ============

    // 매니저 연결
    UFUNCTION(BlueprintCallable, Category = "Connection")
    void ConnectToManager(AGridMazeManager* Manager);

    // 매니저 연결 해제
    UFUNCTION(BlueprintCallable, Category = "Connection")
    void DisconnectFromManager();

    // 자동으로 매니저 찾아서 연결
    UFUNCTION(BlueprintCallable, Category = "Connection")
    void AutoConnectToManager();

    // ============ 설정 변경 함수들 ============

    // 시간 텍스트 크기 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTimeTextSize(float NewSize);

    // 상태 텍스트 크기 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetStatusTextSize(float NewSize);

    // 진행도 텍스트 크기 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetProgressTextSize(float NewSize);

    // 색상 일괄 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetDisplayColors(FLinearColor Ready, FLinearColor Playing, FLinearColor Success, FLinearColor Failed);

    // 메시지 일괄 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetDisplayMessages(const FString& Ready, const FString& Success, const FString& Failed);

    // ============ 정보 조회 함수들 ============

    // 현재 상태 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    EDisplayState GetCurrentState() const { return CurrentDisplayState; }

    // 시간 포맷팅
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FString FormatTime(float TimeInSeconds);

    // 진행도 포맷팅
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FString FormatProgress(int32 Current, int32 Max);

    // 매니저 연결 여부
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsConnectedToManager() const { return ConnectedManager != nullptr; }

    // ============ BP 이벤트들 ============

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnDisplayStateChanged(EDisplayState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnCountdownStep(int32 RemainingSeconds);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnCountdownFinished();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnTimeWarning(float TimeRemaining);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnTimeDanger(float TimeRemaining);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnConnectedToManager(AGridMazeManager* Manager);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnDisconnectedFromManager();

    // ============ BP에서 커스터마이즈 가능한 함수들 ============

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Custom Logic")
    FString CustomFormatTime(float TimeInSeconds);
    virtual FString CustomFormatTime_Implementation(float TimeInSeconds);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Custom Logic")
    FLinearColor GetTimeColor(float TimeRemaining, float TotalTime);
    virtual FLinearColor GetTimeColor_Implementation(float TimeRemaining, float TotalTime);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Custom Logic")
    void CustomUpdateDisplay(float TimeRemaining, EPuzzleState PuzzleState);
    virtual void CustomUpdateDisplay_Implementation(float TimeRemaining, EPuzzleState PuzzleState) {}

protected:
    // ============ 내부 이벤트 콜백들 ============

    UFUNCTION()
    void OnPuzzleStateChanged(EPuzzleState NewState);

    UFUNCTION()
    void OnTimerUpdate(float TimeRemaining);

private:
    void UpdateCountdown();
    void UpdateVisuals();
    void UpdateTextColors(FLinearColor Color);
    void UpdateLightColor(FLinearColor Color);
    void HandleTimeWarnings(float TimeRemaining, float TotalTime);
    void SetupProgressText();

    FTimerHandle CountdownTimerHandle;
    FTimerHandle MessageTimerHandle;
    float BlinkTimer = 0.0f;
};