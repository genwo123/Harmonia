#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Core/StrokeGameTypes.h"
#include "Interaction/UStrokeCell.h"
#include "Engine/DataTable.h"
#include "UStrokeGrid.generated.h"

UCLASS()
class DISTRICT_TEST_API UStrokeGrid : public UUserWidget
{
    GENERATED_BODY()

public:
    UStrokeGrid(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Widgets")
    class UUniformGridPanel* GridPanel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UStrokeCell> StrokeCellWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Layout")
    float MaxGridWidth = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Layout")
    float MaxGridHeight = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Layout")
    float MinCellSize = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Layout")
    float CellPadding = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Grid Layout")
    float CurrentCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor StartPointColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor GoalPointColor = FLinearColor(0.8f, 0.2f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor RedPointColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor GreenPointColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor BluePointColor = FLinearColor(0.0f, 0.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
    UTexture2D* StartPointImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
    UTexture2D* GoalPointImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
    UTexture2D* WallImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
    UTexture2D* TeleportPortal1Image;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
    UTexture2D* TeleportPortal2Image;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
    UTexture2D* TeleportPortal3Image;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
    UTexture2D* TeleportPortal4Image;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Ground")
    UTexture2D* GroundImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Ground")
    UTexture2D* VisitedGroundImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Points")
    UTexture2D* RedPointImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Points")
    UTexture2D* GreenPointImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Points")
    UTexture2D* BluePointImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Points|Visited")
    UTexture2D* RedPointVisitedImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Points|Visited")
    UTexture2D* GreenPointVisitedImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Points|Visited")
    UTexture2D* BluePointVisitedImage;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Widgets|Progress")
    class UImage* ProgressBar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Progress")
    UTexture2D* ProgressStartImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Progress")
    UTexture2D* ProgressRedImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Progress")
    UTexture2D* ProgressGreenImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Progress")
    UTexture2D* ProgressBlueImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Progress")
    UTexture2D* ProgressGoalImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Display")
    bool bShowPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Display")
    FLinearColor DefaultPathLineColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.8f);

    UPROPERTY(BlueprintReadOnly, Category = "Path Display")
    FLinearColor CurrentPathLineColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear Message")
    FString ClearMessage = TEXT("Clear!");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clear Message")
    float ClearMessageDisplayTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Rules")
    bool bEnforceRGBOrder = false;

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnPuzzleCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnRedPointCollected();

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnGreenPointCollected();

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnBluePointCollected();

    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnProgressReset();

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Widgets")
    class UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Widgets")
    class UTextBlock* StageNameText;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Widgets")
    class UButton* ResetButton;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
    UDataTable* StageDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
    int32 CurrentStageNumber = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Config")
    FName CurrentStageRowName = TEXT("Stage_01");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode")
    bool bEditMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode", meta = (EditCondition = "bEditMode"))
    EStrokeCellType CurrentEditType = EStrokeCellType::Wall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode", meta = (EditCondition = "bEditMode"))
    bool bShowGridNumbers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode", meta = (EditCondition = "bEditMode"))
    int32 CurrentTeleportID = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    FIntPoint EditorGridSize = FIntPoint(5, 5);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    FIntPoint EditorStartPosition = FIntPoint(2, 4);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    FIntPoint EditorGoalPosition = FIntPoint(2, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    TArray<FIntPoint> EditorRequiredPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    TArray<FIntPoint> EditorWallPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings")
    TArray<FTeleportPortal> EditorTeleportPortals;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Game")
    FStrokePuzzleData CurrentPuzzle;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Game")
    TArray<UStrokeCell*> CellWidgets;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Game")
    FIntPoint CurrentPlayerPosition;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Game")
    TArray<FIntPoint> VisitedPositions;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Game")
    TArray<FIntPoint> VisitedRequiredPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Stroke Game")
    EStrokeGameState GameState = EStrokeGameState::Ready;

public:
    UFUNCTION(BlueprintCallable, Category = "Path Display")
    void UpdatePathDisplay();

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void UpdateProgressBar();

    UFUNCTION(BlueprintCallable, Category = "Stage")
    void LoadStageFromDataTable(int32 StageNumber);

    UFUNCTION(BlueprintCallable, Category = "Stage")
    void LoadStageFromRowName(FName RowName);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Stage Editor")
    void PreviewStageInEditor();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Stage Editor")
    void SaveCurrentToDataTable();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ApplyEditorSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ResizeGrid();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ValidatePositions();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void TestPuzzle();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void AutoPlaceRGBPoints();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ClearAllTeleportPortals();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void AutoCompleteTeleportPairs();

    UFUNCTION(BlueprintCallable, Category = "Editor")
    void OnCellEditClicked(FIntPoint Position);

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    void InitializeGrid(const FStrokePuzzleData& PuzzleData);

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    void CreateCells();

    UFUNCTION(BlueprintCallable, Category = "Grid Layout")
    void CalculateCellSize();

    UFUNCTION(BlueprintCallable, Category = "Grid Layout")
    void ApplyGridLayout();

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    void ClearGrid();

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    bool MovePlayer(FIntPoint Direction);

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    bool IsValidMove(FIntPoint NewPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    UStrokeCell* GetCellAtPosition(FIntPoint Position) const;

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    void CheckWinCondition();

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    void ResetGame();

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    FIntPoint CheckTeleport(FIntPoint Position);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Ground|Visited")
    UTexture2D* VisitedGroundRedImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Ground|Visited")
    UTexture2D* VisitedGroundGreenImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images|Ground|Visited")
    UTexture2D* VisitedGroundBlueImage;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Movement")
    USoundBase* MoveSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Movement")
    float MoveSoundVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Points")
    USoundBase* RedPointSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Points")
    USoundBase* GreenPointSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Points")
    USoundBase* BluePointSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Points")
    float PointSoundVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Success")
    USoundBase* PuzzleCompleteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Success")
    float PuzzleCompleteSoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Reset")
    USoundBase* ResetSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Reset")
    float ResetSoundVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Settings")
    bool bEnableSounds = true;


protected:
    EStrokeCellType GetCellTypeAtPosition(FIntPoint Position) const;
    int32 GetCellIndex(FIntPoint Position) const;
    void UpdateStatusText();
    void UpdatePlayerVisual();
    void UpdatePathColor();
    bool AreAllRequiredPointsVisited() const;
    bool IsRGBOrderCorrect() const;
    void OnGameWon();
    void ShowClearMessage();
    void HideClearMessage();

    void UpdateEditorVisuals();
    bool IsPositionValid(FIntPoint Position) const;
    void ClampPositionsToGrid();
    void AutoPlaceStartGoal();
    void SetupDefaultPuzzle();

    void PlaySound(USoundBase* Sound, float Volume = 1.0f);


    FTeleportPortal* FindTeleportPortal(int32 PortalID);
    void ToggleTeleportPortal(FIntPoint Position, int32 PortalID);
    void RemoveFromAllTeleportPortals(FIntPoint Position);

    void LoadStageDataFromTable(const FStrokeStageData& StageData);
    FString GenerateRowNameFromStage(int32 StageNumber) const;

    UFUNCTION()
    void OnResetClicked();

    FTimerHandle ClearMessageTimerHandle;
};