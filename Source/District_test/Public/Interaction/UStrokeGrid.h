#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Core/StrokeGameTypes.h"
#include "Interaction/UStrokeCell.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes",
        meta = (DisplayName = "Stroke Cell Widget Class"))
    TSubclassOf<UStrokeCell> StrokeCellWidgetClass;

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

    // ��� ǥ�� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Display")
    bool bShowPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Display")
    FLinearColor PathLineColor = FLinearColor(1.0f, 1.0f, 0.0f, 0.8f);

    // Ŭ���� �̺�Ʈ (Blueprint���� ����)
    UFUNCTION(BlueprintImplementableEvent, Category = "Game Events")
    void OnPuzzleCompleted();

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Widgets")
    class UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget), BlueprintReadWrite, Category = "Widgets")
    class UButton* ResetButton;

    // ������ ��� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode")
    bool bEditMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode", meta = (EditCondition = "bEditMode"))
    EStrokeCellType CurrentEditType = EStrokeCellType::Wall;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode", meta = (EditCondition = "bEditMode"))
    bool bShowGridNumbers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Mode", meta = (EditCondition = "bEditMode"))
    int32 CurrentTeleportID = 1;

    // �ǽð� �׸��� ũ�� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Settings",
        meta = (ClampMin = "3", ClampMax = "20"))
    FIntPoint EditorGridSize = FIntPoint(5, 5);

    // �����Ϳ��� �ٷ� ���� ������ ����Ʈ��
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

    // ���� ������
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

    // ������ �Լ��� - Blueprint������ ȣ�� �����ϰ� ����
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

    // �ֿ� ���� �Լ���
    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    void InitializeGrid(const FStrokePuzzleData& PuzzleData);

    UFUNCTION(BlueprintCallable, Category = "Stroke Game")
    void CreateCells();

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

protected:
    // ���� �Լ���
    EStrokeCellType GetCellTypeAtPosition(FIntPoint Position) const;
    int32 GetCellIndex(FIntPoint Position) const;
    void UpdateStatusText();
    void UpdatePlayerVisual();
    bool AreAllRequiredPointsVisited() const;
    void OnGameWon();

    // ������ ���� �Լ���
    void UpdateEditorVisuals();
    bool IsPositionValid(FIntPoint Position) const;
    void ClampPositionsToGrid();
    void AutoPlaceStartGoal();
    void SetupDefaultPuzzle();

    // �ڷ���Ʈ ���� �Լ���
    FTeleportPortal* FindTeleportPortal(int32 PortalID);
    void ToggleTeleportPortal(FIntPoint Position, int32 PortalID);
    void RemoveFromAllTeleportPortals(FIntPoint Position);

    // ��ư �̺�Ʈ
    UFUNCTION()
    void OnResetClicked();
};