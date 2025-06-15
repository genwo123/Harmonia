// GridMazeManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/TimerHandle.h"
#include "GridMazeManager.generated.h"

// Forward Declarations
class AGridTile;
class AMazeDisplay;
class USoundBase;
class UWorld;

UENUM(BlueprintType)
enum class EMazeState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),       // 시작 대기
    Running     UMETA(DisplayName = "Running"),     // 진행 중
    Success     UMETA(DisplayName = "Success"),     // 성공
    Failed      UMETA(DisplayName = "Failed"),      // 실패
    TimeOut     UMETA(DisplayName = "Time Out")     // 시간 초과
};

UENUM(BlueprintType)
enum class EMazeDifficulty : uint8
{
    Easy        UMETA(DisplayName = "Easy"),        // 3x3, 쉬운 경로
    Normal      UMETA(DisplayName = "Normal"),      // 5x5, 보통 경로
    Hard        UMETA(DisplayName = "Hard"),        // 7x7, 어려운 경로
    Expert      UMETA(DisplayName = "Expert"),      // 8x8+, 매우 어려운 경로
    Custom      UMETA(DisplayName = "Custom")       // 사용자 정의
};

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMazeDataUpdate, float, TimeRemaining, int32, AttemptCount, EMazeState, CurrentState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMazeComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileActivated, int32, TileX, int32, TileY);

UCLASS()
class DISTRICT_TEST_API AGridMazeManager : public AActor
{
    GENERATED_BODY()

public:
    AGridMazeManager();
    virtual ~AGridMazeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    // OnConstruction - 언리얼 표준 방식
    virtual void OnConstruction(const FTransform& Transform) override;

    // 프로퍼티 변경 감지
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // ====== GRID SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float TileSize = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float TileSpacing = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    TSubclassOf<AGridTile> TileClass;

    // ====== DIFFICULTY SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    EMazeDifficulty Difficulty = EMazeDifficulty::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (EditCondition = "Difficulty == EMazeDifficulty::Custom"))
    int32 CustomGridSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (EditCondition = "Difficulty == EMazeDifficulty::Custom"))
    TArray<FVector2D> CustomCorrectPath;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ====== TIMER SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    bool bHasTimeLimit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bHasTimeLimit"))
    float TimeLimit = 180.0f;



    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bHasTimeLimit"))
    bool bAutoResetOnTimeout = true;

    // ====== DISPLAY SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    AMazeDisplay* ConnectedDisplay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bAutoFindDisplay = true;

    // ====== GAMEPLAY SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAllowDiagonalMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bResetOnWrongStep = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 MaxAttempts = 0; // 0 = 무제한

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowHints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (EditCondition = "bShowHints"))
    int32 MaxHints = 3;

    // ====== EDITOR PREVIEW SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPreviewInEditor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bAutoUpdatePreview = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bShowPathInPreview = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    FLinearColor PreviewPathColor = FLinearColor(0.0f, 0.8f, 1.0f, 0.7f);

    // ====== EDITOR SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Settings")
    bool bCreateGridInEditor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Settings")
    bool bShowPathPreview = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Settings")
    FLinearColor PathPreviewColor = FLinearColor::Blue;

    // ====== AUDIO SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bPlaySounds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (EditCondition = "bPlaySounds"))
    USoundBase* CorrectStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (EditCondition = "bPlaySounds"))
    USoundBase* WrongStepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (EditCondition = "bPlaySounds"))
    USoundBase* SuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (EditCondition = "bPlaySounds"))
    USoundBase* FailureSound;

    // ====== INTERNAL VARIABLES ======
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EMazeState CurrentState = EMazeState::Ready;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 AttemptCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 UsedHints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Settings")
    bool bIsPartOfCorrectPath = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentPathIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Grid")
    TArray<AGridTile*> GridTiles;

    UPROPERTY(BlueprintReadOnly, Category = "Path")
    TArray<FVector2D> CorrectPath;

    UPROPERTY(BlueprintReadOnly, Category = "Path")
    FVector2D StartPosition = FVector2D(0, 0);

    UPROPERTY(BlueprintReadOnly, Category = "Path")
    FVector2D GoalPosition = FVector2D(4, 4);

public:
    // ====== EVENTS ======
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMazeDataUpdate OnMazeDataUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnMazeComplete OnMazeComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTileActivated OnTileActivated;

    // ====== EDITOR FUNCTIONS ======
    UFUNCTION(BlueprintCallable, Category = "Editor Preview")
    void EditorCreateGrid();

    UFUNCTION(BlueprintCallable, Category = "Editor Preview")
    void EditorClearGrid();

    UFUNCTION(BlueprintCallable, Category = "Editor Preview")
    void EditorGeneratePath();

    UFUNCTION(BlueprintCallable, Category = "Editor Preview")
    void RefreshEditorPreview();

    // ====== PUBLIC FUNCTIONS ======
    UFUNCTION(BlueprintCallable, Category = "Maze Control")
    void StartMaze();

    UFUNCTION(BlueprintCallable, Category = "Maze Control")
    void ResetMaze();

    UFUNCTION(BlueprintCallable, Category = "Maze Control")
    void PauseMaze();

    UFUNCTION(BlueprintCallable, Category = "Maze Control")
    void ResumeMaze();

    UFUNCTION(BlueprintCallable, Category = "Maze Control")
    void UseHint();

    UFUNCTION(BlueprintCallable, Category = "Maze Info")
    EMazeState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Maze Info")
    float GetTimeRemaining() const { return TimeRemaining; }

    UFUNCTION(BlueprintCallable, Category = "Maze Info")
    int32 GetAttemptCount() const { return AttemptCount; }

    UFUNCTION(BlueprintCallable, Category = "Maze Info")
    FVector2D GetPlayerGridPosition() const;

    // ====== TILE INTERACTION ======
    UFUNCTION(BlueprintCallable, Category = "Tile System")
    void OnTileStepped(AGridTile* SteppedTile, AActor* SteppedActor);

    UFUNCTION(BlueprintCallable, Category = "Tile System")
    AGridTile* GetTileAt(int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, Category = "Tile System")
    bool IsValidGridPosition(int32 X, int32 Y);

protected:
    // ====== UNIFIED GRID MANAGEMENT ======
    void CreateGameGrid();
    void CreateEditorGrid();
    void CreateTiles(const FString& TileTag, bool bConnectDelegates);
    void ClearAllTiles(const FString& TileTag = TEXT(""));

    // ====== LEGACY FUNCTIONS (호환성을 위해 유지) ======
    void CreateGrid();
    void ClearGrid();

    // ====== EDITOR HELPER FUNCTIONS ======
    void CreateEditorPreviewGrid();
    void UpdatePathVisualization();
    void ShowPreviewPath();
    bool IsInEditorMode() const;

    // ====== INTERNAL FUNCTIONS ======
    void GenerateCorrectPath();
    void SetupDifficultySettings();

    FVector CalculateTileWorldPosition(int32 X, int32 Y);
    int32 CoordinateToIndex(int32 X, int32 Y);
    FVector2D IndexToCoordinate(int32 Index);

    void UpdateTimer(float DeltaTime);
    void BroadcastMazeData();
    void SetMazeState(EMazeState NewState);

    bool IsCorrectNextStep(int32 X, int32 Y);
    void HandleCorrectStep(AGridTile* Tile);
    void HandleWrongStep(AGridTile* Tile);
    void HandleMazeSuccess();
    void HandleMazeFailure();

    void PlaySound(USoundBase* Sound);
    void FindConnectedDisplay();

    UFUNCTION(BlueprintCallable, Category = "Manual Control")
    void ManualCreateTiles();

    UFUNCTION(BlueprintCallable, Category = "Manual Control")
    void ManualClearAllTiles();

    // ====== BLUEPRINT EVENTS ======
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnMazeStateChanged(EMazeState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnPlayerPositionChanged(int32 NewX, int32 NewY);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnHintUsed(int32 RemainingHints);


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual Path Setup")
    FVector2D ManualStartPosition = FVector2D(0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual Path Setup")
    FVector2D ManualGoalPosition = FVector2D(4, 4);

    UFUNCTION(BlueprintCallable, Category = "Manual Path Setup")
    void SetStartAndGoalTiles();
};