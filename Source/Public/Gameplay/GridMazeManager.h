// GridMazeManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/TimerHandle.h"
#include "GridMazeManager.generated.h"

class AGridTile;
class AMazeDisplay;

UENUM(BlueprintType)
enum class EPuzzleState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Playing     UMETA(DisplayName = "Playing"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPuzzleStateChanged, EPuzzleState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdate, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressUpdate, int32, CurrentStep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleFailed);

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API AGridMazeManager : public AActor
{
    GENERATED_BODY()

public:
    AGridMazeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Destroyed() override;

#if WITH_EDITOR
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    // ============ 그리드 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Size")
    int32 GridRows = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Size")
    int32 GridColumns = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    TSubclassOf<AGridTile> TileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    float TileSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    float TileThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    float TileSpacing = 50.0f;

    // ============ 퍼즐 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Time")
    float PuzzleTimeLimit = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Preview")
    bool bEnablePreview = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Preview")
    float PreviewDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Preview")
    float TileLightDelay = 0.5f;  // 각 타일이 켜지는 간격

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Correct Display")
    float CorrectDisplayDuration = 1.0f;  // 초록색 표시 시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Fail")
    bool bAutoResetOnFail = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Fail")
    float FailResetDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Fail")
    bool bContinueTimeOnFail = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Progress")
    bool bKeepProgressOnFail = true;

    // ============ 시작 위치 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor")
    bool bUseStartingFloor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor", meta = (EditCondition = "bUseStartingFloor"))
    FVector StartingFloorPosition = FVector(-400, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor", meta = (EditCondition = "bUseStartingFloor"))
    FVector StartingFloorSize = FVector(200.0f, 200.0f, 20.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor", meta = (EditCondition = "bUseStartingFloor"))
    FLinearColor StartingFloorColor = FLinearColor(0.1f, 0.8f, 0.1f, 1.0f);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Starting Floor")
    UStaticMeshComponent* StartingFloorMesh;

    // ============ 경로 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings", meta = (TitleProperty = "ToString"))
    TArray<FIntPoint> CorrectPath;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Settings")
    FIntPoint StartPosition = FIntPoint(0, 0);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Settings")
    FIntPoint GoalPosition = FIntPoint(3, 3);

    // ============ 색상 설정 (타일에 적용) ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor InactiveColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor PreviewColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor ReadyColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor FirstStepColor = FLinearColor(0.3f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor CorrectColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor WrongColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

    // ============ 사운드 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* CorrectStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* WrongStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* SuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* FailSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    float SoundVolume = 1.0f;

    // ============ 디스플레이 연결 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Connection")
    AMazeDisplay* ConnectedDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Connection")
    bool bAutoFindDisplay = true;

    // ============ 에디터 프리뷰 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPreviewInEditor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPathInPreview = true;

    // ============ 현재 상태 ============

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EPuzzleState CurrentState = EPuzzleState::Ready;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float TimeRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentPathIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    TArray<AGridTile*> GridTiles;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsShowingPreview = false;

    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> CompletedSteps;

    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> FailedSteps;

public:
    // ============ 이벤트 델리게이트 ============

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPuzzleStateChanged OnPuzzleStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimerUpdate OnTimerUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnProgressUpdate OnProgressUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPuzzleStarted OnPuzzleStarted_Event;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPuzzleCompleted OnPuzzleCompleted_Event;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPuzzleFailed OnPuzzleFailed_Event;

    // ============ 퍼즐 제어 함수 ============

    // 퍼즐 시작
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzle();

    // 카운트다운과 함께 시작
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzleWithCountdown();

    // 퍼즐 리셋
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void ResetPuzzle();

    // 완전 리셋 (진행도 포함)
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void CompleteReset();

    // 퍼즐 성공 처리
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void CompletePuzzle();

    // 퍼즐 실패 처리
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void FailPuzzle();

    // 퍼즐 일시정지
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void PausePuzzle();

    // 퍼즐 재개
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void ResumePuzzle();

    // ============ 타일 관리 ============

    // 타일 스텝 이벤트 처리
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void OnTileStep(AGridTile* SteppedTile, AActor* Player);

    // 특정 위치의 타일 가져오기
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    AGridTile* GetTileAt(int32 X, int32 Y);

    // 그리드 생성
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void CreateGrid();

    // 그리드 제거
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void ClearGrid();

    // 모든 타일 비활성화
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void SetAllTilesInactive();

    // 모든 타일 준비 상태로
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void SetAllTilesReady();

    // ============ 미리보기 제어 ============

    // 경로 미리보기 시작
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void ShowPathPreviewSequence();

    // 미리보기 중지
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void StopPreview();

    // ============ 시작 위치 제어 ============

    // 시작 바닥 생성
    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void CreateStartingFloor();

    // 시작 바닥 색상 변경
    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void SetStartingFloorColor(FLinearColor NewColor);

    // 시작 위치 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Starting Floor")
    FVector GetStartingFloorLocation() const;

    // 플레이어를 시작 위치로 이동
    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void RespawnPlayerToStart(AActor* Player);

    // ============ 진행도 추적 ============

    // 완료된 스텝 표시
    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void MarkStepAsCompleted(const FIntPoint& Position);

    // 실패한 스텝 표시
    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void MarkStepAsFailed(const FIntPoint& Position);

    // 스텝 완료 여부 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progress Tracking")
    bool IsStepCompleted(const FIntPoint& Position) const;

    // 스텝 실패 여부 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progress Tracking")
    bool IsStepFailed(const FIntPoint& Position) const;

    // 진행도 기록 초기화
    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void ClearProgressHistory();

    // 진행도 색상 복원
    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void RestoreProgressColors();

    // ============ 경로 검증 ============

    // 정답 경로 검증
    UFUNCTION(BlueprintCallable, Category = "Path Validation")
    bool ValidateCorrectPath();

    // ============ 설정 변경 ============

    // 그리드 크기 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetGridSize(int32 NewRows, int32 NewColumns);

    // 제한 시간 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTimeLimit(float NewTimeLimit);

    // 정답 경로 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetCorrectPath(const TArray<FIntPoint>& NewPath);

    // 실패 후 리셋 지연 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetFailResetDelay(float NewDelay);

    // 타일 두께 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTileThickness(float NewThickness);

    // 색상 일괄 설정
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetPuzzleColors(FLinearColor Inactive, FLinearColor Ready, FLinearColor Correct, FLinearColor Wrong);

    // ============ 정보 조회 ============

    // 현재 상태 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    EPuzzleState GetCurrentState() const { return CurrentState; }

    // 남은 시간 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    float GetTimeRemaining() const { return TimeRemaining; }

    // 현재 진행 스텝 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    int32 GetCurrentPathIndex() const { return CurrentPathIndex; }

    // 퍼즐 진행 중인지 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsGameActive() const { return CurrentState == EPuzzleState::Playing; }

    // 진행도 비율 가져오기 (0.0 ~ 1.0)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    float GetProgress() const;

    // 진행도 텍스트 가져오기 "(3/10)" 형식
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FString GetProgressText() const;

    // 경로 길이 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    int32 GetPathLength() const { return CorrectPath.Num(); }

    // 특정 인덱스의 경로 스텝 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FIntPoint GetPathStepAt(int32 Index) const;

    // 퍼즐 완료 여부
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsPuzzleCompleted() const { return CurrentState == EPuzzleState::Success; }

    // 퍼즐 실패 여부
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsPuzzleFailed() const { return CurrentState == EPuzzleState::Failed; }

    // ============ 에디터 도구 ============

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorCreateGrid();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorClearGrid();

    // ============ BP 이벤트들 ============

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnPuzzleStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnPuzzleCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnPuzzleFailed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnCorrectStep(AGridTile* Tile);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnWrongStep(AGridTile* Tile);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnTimeWarning(float TimeLeft);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnPreviewStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnPreviewFinished();

    // ============ BP 커스터마이즈 가능 함수들 ============

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    bool ShouldStartPuzzle(AGridTile* FirstTile);
    virtual bool ShouldStartPuzzle_Implementation(AGridTile* FirstTile) { return true; }

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    void OnCustomTileStep(AGridTile* Tile, bool bIsCorrect);
    virtual void OnCustomTileStep_Implementation(AGridTile* Tile, bool bIsCorrect) {}

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    void CustomResetLogic();
    virtual void CustomResetLogic_Implementation() {}

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    void OnPlayerRespawned(AActor* Player);
    virtual void OnPlayerRespawned_Implementation(AActor* Player) {}

private:
    void UpdateTimer(float DeltaTime);
    void SetPuzzleState(EPuzzleState NewState);
    void ConnectToDisplay();
    void CreateTilesInternal();
    void ClearGridTiles();
    void UpdateTilePositions();
    void ShowNextPreviewTile(int32 Index);
    void UpdateTileThickness();
    FVector CalculateTilePosition(int32 X, int32 Y);
    bool IsValidPosition(int32 X, int32 Y);
    bool IsCorrectNextStep(int32 X, int32 Y);
    void PlaySound(USoundBase* Sound);
    void ApplyTileColors();
    void DestroyAllTiles();
    void ResetToStartPosition();

    FTimerHandle WrongTileTimer;

    bool bGamePaused = false;
    FTimerHandle ResetTimer;
    FTimerHandle PreviewTimerHandle;
    FTimerHandle CorrectDisplayTimer;
};