// GridMazeManager.h - 모든 문제점 해결 완전판
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/TimerHandle.h"
#include "GridMazeManager.generated.h"

// Forward Declarations
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
    // ====== Basic Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 GridSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    TSubclassOf<AGridTile> TileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float TileSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float TileSpacing = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    float PuzzleTimeLimit = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    bool bAutoResetOnFail = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    float FailResetDelay = 3.0f;  // 실패 후 리스폰 시간 조절 가능

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    bool bContinueTimeOnFail = true;  // 실패해도 시간 계속 흐르게

    // ====== Starting Floor Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor")
    bool bUseStartingFloor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor",
        meta = (EditCondition = "bUseStartingFloor"))
    FVector StartingFloorPosition = FVector(-400, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor",
        meta = (EditCondition = "bUseStartingFloor"))
    FVector StartingFloorSize = FVector(200.0f, 200.0f, 20.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Floor",
        meta = (EditCondition = "bUseStartingFloor"))
    FLinearColor StartingFloorColor = FLinearColor(0.1f, 0.8f, 0.1f, 1.0f);

    // Starting Floor Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Starting Floor")
    UStaticMeshComponent* StartingFloorMesh;

    // ====== Manual Path Setting (FIntPoint로 변경!) ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual Path",
        meta = (TitleProperty = "ToString"))
    TArray<FIntPoint> CorrectPath;  // FVector2D에서 FIntPoint로 변경!

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manual Path")
    FIntPoint StartPosition = FIntPoint(0, 0);  // FVector2D에서 FIntPoint로 변경!

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manual Path")
    FIntPoint GoalPosition = FIntPoint(3, 3);   // FVector2D에서 FIntPoint로 변경!

    // ====== Progress Tracking (색상 유지 시스템) ======
    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> CompletedSteps;  // 성공한 스텝들 기록

    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> FailedSteps;     // 실패한 스텝들 기록

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Tracking")
    bool bKeepProgressOnFail = true;   // 실패해도 진행 기록 유지

    // ====== Color Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor WaitingColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f); // Gray

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor ReadyColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // Green

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor CorrectColor = FLinearColor(0.2f, 0.2f, 1.0f, 1.0f); // Blue

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor WrongColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // Red

    // ====== Sound Settings ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* CorrectStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* WrongStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* SuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* FailSound;

    // ====== Display Connection ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Connection")
    AMazeDisplay* ConnectedDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Connection")
    bool bAutoFindDisplay = true;

    // ====== Editor Preview ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPreviewInEditor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPathInPreview = true;

    // ====== Current State ======
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EPuzzleState CurrentState = EPuzzleState::Ready;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float TimeRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 CurrentPathIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    TArray<AGridTile*> GridTiles;

public:

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void CompleteReset();

    // ====== Blueprint Events ======
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPuzzleStateChanged OnPuzzleStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimerUpdate OnTimerUpdate;

    // ====== Puzzle Control ======
    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzleWithCountdown();  // 카운트다운과 함께 시작

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void ResetPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void CompletePuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void FailPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void PausePuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void ResumePuzzle();

    // ====== Tile Management ======
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void OnTileStep(AGridTile* SteppedTile, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    AGridTile* GetTileAt(int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void CreateGrid();

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void ClearGrid();

    // ====== Editor Tools ======
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorCreateGrid();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorClearGrid();

    // ====== Starting Floor Functions ======
    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void CreateStartingFloor();

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void SetStartingFloorColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    FVector GetStartingFloorLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void RespawnPlayerToStart(AActor* Player);

    // ====== Progress Tracking Functions ======
    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void MarkStepAsCompleted(const FIntPoint& Position);

    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void MarkStepAsFailed(const FIntPoint& Position);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progress Tracking")
    bool IsStepCompleted(const FIntPoint& Position) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progress Tracking")
    bool IsStepFailed(const FIntPoint& Position) const;

    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void ClearProgressHistory();

    UFUNCTION(BlueprintCallable, Category = "Progress Tracking")
    void RestoreProgressColors();  // 저장된 색상 복원

    // ====== Path Validation ======
    UFUNCTION(BlueprintCallable, Category = "Path Validation")
    bool ValidateCorrectPath();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Path Information")
    int32 GetPathLength() const { return CorrectPath.Num(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Path Information")
    FIntPoint GetPathStepAt(int32 Index) const;  // FVector2D에서 FIntPoint로 변경!

    // ====== Settings Functions ======
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetGridSize(int32 NewSize);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTimeLimit(float NewTimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetCorrectPath(const TArray<FIntPoint>& NewPath);  // FVector2D에서 FIntPoint로 변경!

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetFailResetDelay(float NewDelay);

    // ====== Information Query ======
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    EPuzzleState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    float GetTimeRemaining() const { return TimeRemaining; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    int32 GetCurrentPathIndex() const { return CurrentPathIndex; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsGameActive() const { return CurrentState == EPuzzleState::Playing; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    float GetProgress() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FString GetProgressText() const;  // 진행도 텍스트 반환

    // ====== Blueprint Events (Implementable in BP) ======
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

    // ====== Blueprint Overrideable Functions ======
    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    bool ShouldStartPuzzle(AGridTile* FirstTile);
    virtual bool ShouldStartPuzzle_Implementation(AGridTile* FirstTile) { return true; }

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    void OnCustomTileStep(AGridTile* Tile, bool bIsCorrect);
    virtual void OnCustomTileStep_Implementation(AGridTile* Tile, bool bIsCorrect) {}

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    void CustomResetLogic();
    virtual void CustomResetLogic_Implementation() {}



private:
    // Internal functions
    void UpdateTimer(float DeltaTime);
    void SetPuzzleState(EPuzzleState NewState);
    void ConnectToDisplay();
    void CreateTilesInternal();
    void ClearGridTiles();
    void UpdateTilePositions();
    void ShowPathPreview();
    FVector CalculateTilePosition(int32 X, int32 Y);
    bool IsValidPosition(int32 X, int32 Y);
    bool IsCorrectNextStep(int32 X, int32 Y);
    void PlaySound(USoundBase* Sound);
    void ApplyTileColors();
    void DestroyAllTiles();

    // Pathfinding specific
    void SetAllTilesWaiting();
    void SetAllTilesReady();
    void ResetToStartPosition();

    // Timer related
    bool bGamePaused = false;
    FTimerHandle ResetTimer;
};