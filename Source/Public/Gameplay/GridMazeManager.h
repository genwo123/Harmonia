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

USTRUCT(BlueprintType)
struct FPathData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FIntPoint> PathPoints;

    FPathData() {}
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
    UFUNCTION(BlueprintPure, Category = "Preview")
    float GetPreviewDuration() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Size")
    int32 GridRows = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Size")
    int32 GridColumns = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    TSubclassOf<AGridTile> TileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    float TileSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    float TileThickness = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings|Tile")
    float TileSpacing = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Time")
    float PuzzleTimeLimit = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Preview")
    bool bEnablePreview = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Preview")
    float PreviewDuration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Preview")
    float TileLightDelay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Correct Display")
    float CorrectDisplayDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Fail")
    bool bAutoResetOnFail = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Fail")
    float FailResetDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Fail")
    bool bContinueTimeOnFail = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings|Progress")
    bool bKeepProgressOnFail = true;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings|Random")
    bool bUseRandomPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings|Random")
    TArray<FPathData> PredefinedPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings", meta = (TitleProperty = "ToString"))
    TArray<FIntPoint> CorrectPath;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Settings")
    FIntPoint StartPosition = FIntPoint(0, 0);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Settings")
    FIntPoint GoalPosition = FIntPoint(6, 4);

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

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsShowingPreview = false;

    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> CompletedSteps;

    UPROPERTY(BlueprintReadOnly, Category = "Progress Tracking")
    TArray<FIntPoint> FailedSteps;

public:
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

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void StartPuzzleWithCountdown();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void ResetPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle Control")
    void CompleteReset();

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

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void SetAllTilesInactive();

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void SetAllTilesReady();

    UFUNCTION(BlueprintCallable, Category = "Preview")
    void ShowPathPreviewSequence();

    UFUNCTION(BlueprintCallable, Category = "Preview")
    void StopPreview();

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void CreateStartingFloor();

    UFUNCTION(BlueprintCallable, Category = "Starting Floor")
    void SetStartingFloorColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Starting Floor")
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

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetPuzzleColors(FLinearColor Inactive, FLinearColor Ready, FLinearColor Correct, FLinearColor Wrong);

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

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    int32 GetPathLength() const { return CorrectPath.Num(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FIntPoint GetPathStepAt(int32 Index) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsPuzzleCompleted() const { return CurrentState == EPuzzleState::Success; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsPuzzleFailed() const { return CurrentState == EPuzzleState::Failed; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorCreateGrid();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void EditorClearGrid();

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
    void SelectRandomPath();

    FTimerHandle WrongTileTimer;
    bool bGamePaused = false;
    FTimerHandle ResetTimer;
    FTimerHandle PreviewTimerHandle;
    FTimerHandle CorrectDisplayTimer;
    int32 SelectedPathIndex = -1;
};