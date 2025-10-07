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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 GridRows = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    int32 GridColumns = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    TSubclassOf<AGridTile> TileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float TileSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float TileThickness = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    float TileSpacing = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    float PuzzleTimeLimit = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    bool bAutoResetOnFail = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    float FailResetDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer Settings")
    bool bContinueTimeOnFail = true;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual Path", meta = (TitleProperty = "ToString"))
    TArray<FIntPoint> CorrectPath;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manual Path")
    FIntPoint StartPosition = FIntPoint(0, 0);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manual Path")
    FIntPoint GoalPosition = FIntPoint(3, 3);

    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> CompletedSteps;

    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> FailedSteps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress Tracking")
    bool bKeepProgressOnFail = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor WaitingColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor ReadyColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor CorrectColor = FLinearColor(0.2f, 0.2f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color Settings")
    FLinearColor WrongColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* CorrectStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* WrongStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* SuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    USoundBase* FailSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Connection")
    AMazeDisplay* ConnectedDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display Connection")
    bool bAutoFindDisplay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPreviewInEditor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPathInPreview = true;

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

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPuzzleStateChanged OnPuzzleStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimerUpdate OnTimerUpdate;

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzleWithCountdown();

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

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void OnTileStep(AGridTile* SteppedTile, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    AGridTile* GetTileAt(int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void CreateGrid();

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void ClearGrid();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorCreateGrid();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorClearGrid();

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void CreateStartingFloor();

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void SetStartingFloorColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    FVector GetStartingFloorLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void RespawnPlayerToStart(AActor* Player);

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
    void RestoreProgressColors();

    UFUNCTION(BlueprintCallable, Category = "Path Validation")
    bool ValidateCorrectPath();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Path Information")
    int32 GetPathLength() const { return CorrectPath.Num(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Path Information")
    FIntPoint GetPathStepAt(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetGridSize(int32 NewRows, int32 NewColumns);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTimeLimit(float NewTimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetCorrectPath(const TArray<FIntPoint>& NewPath);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetFailResetDelay(float NewDelay);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTileThickness(float NewThickness);

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
    FString GetProgressText() const;

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
    void SetAllTilesWaiting();
    void SetAllTilesReady();
    void ResetToStartPosition();
    void UpdateTileThickness();

    bool bGamePaused = false;
    FTimerHandle ResetTimer;
};