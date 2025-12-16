#include "Interaction/UStrokeGrid.h"
#include "Interaction/UStrokeCell.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"

UStrokeGrid::UStrokeGrid(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CurrentPlayerPosition = FIntPoint(0, 0);
    GameState = EStrokeGameState::Ready;
    bEditMode = false;
    CurrentTeleportID = 1;
    CurrentStageNumber = 1;
    CurrentStageRowName = TEXT("Stage_01");
    bEnforceRGBOrder = false;
    ClearMessage = TEXT("Clear!");
    ClearMessageDisplayTime = 2.0f;

    MaxGridWidth = 800.0f;
    MaxGridHeight = 600.0f;
    MinCellSize = 40.0f;
    CellPadding = 2.0f;
    CurrentCellSize = 80.0f;

    EditorGridSize = FIntPoint(5, 5);
    EditorStartPosition = FIntPoint(2, 4);
    EditorGoalPosition = FIntPoint(2, 0);
    EditorRequiredPoints = {
        FIntPoint(1, 2),
        FIntPoint(2, 2),
        FIntPoint(3, 2)
    };
    EditorWallPositions = {
        FIntPoint(0, 1),
        FIntPoint(4, 1)
    };

    CurrentPathLineColor = DefaultPathLineColor;
    SetupDefaultPuzzle();
}

void UStrokeGrid::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);

    if (ResetButton)
    {
        ResetButton->OnClicked.AddDynamic(this, &UStrokeGrid::OnResetClicked);
    }

    if (StatusText)
    {
        StatusText->SetVisibility(ESlateVisibility::Hidden);
    }

    if (StageDataTable && CurrentStageNumber > 0)
    {
        LoadStageFromDataTable(CurrentStageNumber);
    }
    else
    {
        ApplyEditorSettings();
    }
}

FReply UStrokeGrid::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (bEditMode || GameState != EStrokeGameState::Playing)
    {
        return FReply::Unhandled();
    }

    FKey PressedKey = InKeyEvent.GetKey();

    if (PressedKey == EKeys::W || PressedKey == EKeys::Up)
    {
        MovePlayer(FIntPoint(-1, 0));
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::S || PressedKey == EKeys::Down)
    {
        MovePlayer(FIntPoint(1, 0));
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::A || PressedKey == EKeys::Left)
    {
        MovePlayer(FIntPoint(0, -1));
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::D || PressedKey == EKeys::Right)
    {
        MovePlayer(FIntPoint(0, 1));
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::R)
    {
        ResetGame();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UStrokeGrid::LoadStageFromDataTable(int32 StageNumber)
{
    if (!StageDataTable)
    {
        return;
    }

    FString RowName = GenerateRowNameFromStage(StageNumber);
    LoadStageFromRowName(*RowName);
}

void UStrokeGrid::LoadStageFromRowName(FName RowName)
{
    if (!StageDataTable)
    {
        return;
    }

    FStrokeStageData* StageData = StageDataTable->FindRow<FStrokeStageData>(RowName, TEXT(""));

    if (StageData)
    {
        CurrentStageRowName = RowName;

        FString RowString = RowName.ToString();
        if (RowString.StartsWith(TEXT("Stage_")))
        {
            FString NumberPart = RowString.RightChop(6);
            CurrentStageNumber = FCString::Atoi(*NumberPart);
        }

        LoadStageDataFromTable(*StageData);
    }
}

void UStrokeGrid::LoadStageDataFromTable(const FStrokeStageData& StageData)
{
    EditorGridSize = FIntPoint(StageData.GridWidth, StageData.GridHeight);
    EditorStartPosition = StageData.StartPosition;
    EditorGoalPosition = StageData.GoalPosition;
    EditorRequiredPoints = StageData.RequiredPoints;
    EditorWallPositions = StageData.WallPositions;
    EditorTeleportPortals = StageData.TeleportPortals;

    CurrentPuzzle.PuzzleName = StageData.StageName;
    CurrentPuzzle.GridSize = FIntPoint(StageData.GridWidth, StageData.GridHeight);
    CurrentPuzzle.StartPosition = StageData.StartPosition;
    CurrentPuzzle.GoalPosition = StageData.GoalPosition;
    CurrentPuzzle.RequiredPoints = StageData.RequiredPoints;
    CurrentPuzzle.WallPositions = StageData.WallPositions;
    CurrentPuzzle.TeleportPortals = StageData.TeleportPortals;

    if (StageNameText)
    {
        
        StageNameText->SetText(FText::FromString(StageData.StageName));
    }

    ApplyEditorSettings();

    if (!bEditMode)
    {
        ResetGame();
    }
}

void UStrokeGrid::PreviewStageInEditor()
{
    if (StageDataTable)
    {
        LoadStageFromDataTable(CurrentStageNumber);
    }
}

void UStrokeGrid::SaveCurrentToDataTable()
{
    if (!StageDataTable)
    {
        return;
    }

    FStrokeStageData NewStageData;
    NewStageData.StageName = FString::Printf(TEXT("Stage %d"), CurrentStageNumber);
    NewStageData.GridWidth = EditorGridSize.X;
    NewStageData.GridHeight = EditorGridSize.Y;
    NewStageData.StartPosition = EditorStartPosition;
    NewStageData.GoalPosition = EditorGoalPosition;
    NewStageData.RequiredPoints = EditorRequiredPoints;
    NewStageData.WallPositions = EditorWallPositions;
    NewStageData.TeleportPortals = EditorTeleportPortals;
}

FString UStrokeGrid::GenerateRowNameFromStage(int32 StageNumber) const
{
    return FString::Printf(TEXT("Stage_%02d"), StageNumber);
}

void UStrokeGrid::ApplyEditorSettings()
{
    ValidatePositions();

    CurrentPuzzle.GridSize = EditorGridSize;
    CurrentPuzzle.StartPosition = EditorStartPosition;
    CurrentPuzzle.GoalPosition = EditorGoalPosition;
    CurrentPuzzle.RequiredPoints = EditorRequiredPoints;
    CurrentPuzzle.WallPositions = EditorWallPositions;
    CurrentPuzzle.TeleportPortals = EditorTeleportPortals;

    InitializeGrid(CurrentPuzzle);
}

void UStrokeGrid::ResizeGrid()
{
    ClampPositionsToGrid();

    if (!IsPositionValid(EditorStartPosition) || !IsPositionValid(EditorGoalPosition))
    {
        AutoPlaceStartGoal();
    }

    ApplyEditorSettings();
}

void UStrokeGrid::ValidatePositions()
{
    TSet<FIntPoint> UsedPositions;

    UsedPositions.Add(EditorStartPosition);

    if (EditorGoalPosition == EditorStartPosition)
    {
        if (EditorGoalPosition.Y > 0)
            EditorGoalPosition.Y--;
        else
            EditorGoalPosition.Y++;
    }
    UsedPositions.Add(EditorGoalPosition);

    for (int32 i = EditorRequiredPoints.Num() - 1; i >= 0; i--)
    {
        if (UsedPositions.Contains(EditorRequiredPoints[i]))
        {
            EditorRequiredPoints.RemoveAt(i);
        }
        else
        {
            UsedPositions.Add(EditorRequiredPoints[i]);
        }
    }

    for (int32 i = EditorWallPositions.Num() - 1; i >= 0; i--)
    {
        if (UsedPositions.Contains(EditorWallPositions[i]))
        {
            EditorWallPositions.RemoveAt(i);
        }
    }

    for (int32 i = EditorTeleportPortals.Num() - 1; i >= 0; i--)
    {
        if (UsedPositions.Contains(EditorTeleportPortals[i].PortalA))
        {
            EditorTeleportPortals[i].PortalA = FIntPoint(-1, -1);
        }
        if (UsedPositions.Contains(EditorTeleportPortals[i].PortalB))
        {
            EditorTeleportPortals[i].PortalB = FIntPoint(-1, -1);
        }

        if (EditorTeleportPortals[i].PortalA.X == -1 && EditorTeleportPortals[i].PortalB.X == -1)
        {
            EditorTeleportPortals.RemoveAt(i);
        }
    }
}

void UStrokeGrid::TestPuzzle()
{
    bEditMode = false;
    ApplyEditorSettings();
    ResetGame();
}

void UStrokeGrid::AutoPlaceRGBPoints()
{
    EditorRequiredPoints.Empty();

    int32 MiddleY = EditorGridSize.Y / 2;

    if (EditorGridSize.X >= 3)
    {
        EditorRequiredPoints.Add(FIntPoint(1, MiddleY));
        EditorRequiredPoints.Add(FIntPoint(EditorGridSize.X / 2, MiddleY));
        EditorRequiredPoints.Add(FIntPoint(EditorGridSize.X - 2, MiddleY));
    }

    ApplyEditorSettings();
}

void UStrokeGrid::ClearAllTeleportPortals()
{
    EditorTeleportPortals.Empty();
    ApplyEditorSettings();
}

void UStrokeGrid::AutoCompleteTeleportPairs()
{
    for (int32 i = EditorTeleportPortals.Num() - 1; i >= 0; i--)
    {
        FTeleportPortal& Portal = EditorTeleportPortals[i];

        if (Portal.PortalA.X == -1 || Portal.PortalB.X == -1)
        {
            for (int32 Y = 0; Y < EditorGridSize.Y; Y++)
            {
                for (int32 X = 0; X < EditorGridSize.X; X++)
                {
                    FIntPoint TestPos(X, Y);
                    if (IsPositionValid(TestPos) && GetCellTypeAtPosition(TestPos) == EStrokeCellType::Empty)
                    {
                        if (Portal.PortalA.X == -1)
                        {
                            Portal.PortalA = TestPos;
                            break;
                        }
                        else if (Portal.PortalB.X == -1)
                        {
                            Portal.PortalB = TestPos;
                            break;
                        }
                    }
                }
                if (Portal.PortalA.X != -1 && Portal.PortalB.X != -1) break;
            }

            if (Portal.PortalA.X == -1 || Portal.PortalB.X == -1)
            {
                EditorTeleportPortals.RemoveAt(i);
            }
        }
    }

    ApplyEditorSettings();
}

void UStrokeGrid::OnCellEditClicked(FIntPoint Position)
{
    if (!bEditMode) return;

    switch (CurrentEditType)
    {
    case EStrokeCellType::Start:
        EditorStartPosition = Position;
        break;

    case EStrokeCellType::Goal:
        EditorGoalPosition = Position;
        break;

    case EStrokeCellType::RedPoint:
    case EStrokeCellType::GreenPoint:
    case EStrokeCellType::BluePoint:
        if (EditorRequiredPoints.Contains(Position))
        {
            EditorRequiredPoints.Remove(Position);
        }
        else
        {
            EditorRequiredPoints.Add(Position);
        }
        break;

    case EStrokeCellType::Wall:
        if (EditorWallPositions.Contains(Position))
        {
            EditorWallPositions.Remove(Position);
        }
        else
        {
            EditorWallPositions.Add(Position);
        }
        break;

    case EStrokeCellType::Empty:
        EditorRequiredPoints.Remove(Position);
        EditorWallPositions.Remove(Position);
        RemoveFromAllTeleportPortals(Position);
        break;

    default:
        ToggleTeleportPortal(Position, CurrentTeleportID);
        break;
    }

    ApplyEditorSettings();
}

void UStrokeGrid::InitializeGrid(const FStrokePuzzleData& PuzzleData)
{
    CurrentPuzzle = PuzzleData;
    ClearGrid();
    CreateCells();

    if (!bEditMode)
    {
        ResetGame();
    }
}

void UStrokeGrid::CreateCells()
{
    if (!GridPanel)
    {
        return;
    }

    UClass* ClassToUse = StrokeCellWidgetClass ? StrokeCellWidgetClass.Get() : UStrokeCell::StaticClass();
    CellWidgets.Empty();

    for (int32 X = 0; X < CurrentPuzzle.GridSize.X; X++)
    {
        for (int32 Y = 0; Y < CurrentPuzzle.GridSize.Y; Y++)
        {
            UStrokeCell* NewCell = CreateWidget<UStrokeCell>(this, ClassToUse);
            if (!NewCell)
            {
                continue;
            }

            FStrokeCellData CellData;
            CellData.GridPosition = FIntPoint(X, Y);
            CellData.CellType = GetCellTypeAtPosition(FIntPoint(X, Y));
            CellData.bIsVisited = false;

            NewCell->SetCellData(CellData);
            NewCell->ParentGrid = this;
            NewCell->UpdateInteractionState(bEditMode);

            GridPanel->AddChildToUniformGrid(NewCell, X, Y);
            CellWidgets.Add(NewCell);
        }
    }

    UpdateEditorVisuals();
}

void UStrokeGrid::ClearGrid()
{
    if (GridPanel)
    {
        GridPanel->ClearChildren();
    }

    CellWidgets.Empty();
    VisitedPositions.Empty();
    VisitedRequiredPoints.Empty();
}



bool UStrokeGrid::IsValidMove(FIntPoint NewPosition) const
{
    if (NewPosition.X < 0 || NewPosition.X >= CurrentPuzzle.GridSize.X ||
        NewPosition.Y < 0 || NewPosition.Y >= CurrentPuzzle.GridSize.Y)
    {
        return false;
    }

    if (CurrentPuzzle.WallPositions.Contains(NewPosition))
    {
        return false;
    }

    if (VisitedPositions.Contains(NewPosition))
    {
        return false;
    }

    return true;
}

void UStrokeGrid::CheckWinCondition()
{
    if (CurrentPlayerPosition != CurrentPuzzle.GoalPosition)
    {
        return;
    }

    if (!AreAllRequiredPointsVisited())
    {
        return;
    }

    if (bEnforceRGBOrder && !IsRGBOrderCorrect())
    {
        return;
    }

    GameState = EStrokeGameState::Won;
    OnGameWon();
}


void UStrokeGrid::ShowClearMessage()
{
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(ClearMessage));
        StatusText->SetVisibility(ESlateVisibility::Visible);

        if (ClearMessageDisplayTime > 0.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(
                ClearMessageTimerHandle,
                this,
                &UStrokeGrid::HideClearMessage,
                ClearMessageDisplayTime,
                false
            );
        }
    }
}

void UStrokeGrid::HideClearMessage()
{
    if (StatusText)
    {
        StatusText->SetVisibility(ESlateVisibility::Hidden);
    }
}



FIntPoint UStrokeGrid::CheckTeleport(FIntPoint Position)
{
    for (const FTeleportPortal& Portal : CurrentPuzzle.TeleportPortals)
    {
        if (Portal.PortalA == Position && Portal.PortalB.X != -1)
        {
            return Portal.PortalB;
        }
        else if (Portal.PortalB == Position && Portal.PortalA.X != -1)
        {
            return Portal.PortalA;
        }
    }
    return Position;
}

UStrokeCell* UStrokeGrid::GetCellAtPosition(FIntPoint Position) const
{
    int32 Index = GetCellIndex(Position);
    if (Index >= 0 && Index < CellWidgets.Num())
    {
        return CellWidgets[Index];
    }
    return nullptr;
}

int32 UStrokeGrid::GetCellIndex(FIntPoint Position) const
{
    if (Position.X < 0 || Position.X >= CurrentPuzzle.GridSize.X ||
        Position.Y < 0 || Position.Y >= CurrentPuzzle.GridSize.Y)
    {
        return -1;
    }

    return Position.X * CurrentPuzzle.GridSize.Y + Position.Y;
}

EStrokeCellType UStrokeGrid::GetCellTypeAtPosition(FIntPoint Position) const
{
    if (Position == CurrentPuzzle.StartPosition)
    {
        return EStrokeCellType::Start;
    }

    if (Position == CurrentPuzzle.GoalPosition)
    {
        return EStrokeCellType::Goal;
    }

    if (CurrentPuzzle.RequiredPoints.Contains(Position))
    {
        int32 Index = CurrentPuzzle.RequiredPoints.Find(Position);
        switch (Index % 3)
        {
        case 0: return EStrokeCellType::RedPoint;
        case 1: return EStrokeCellType::GreenPoint;
        case 2: return EStrokeCellType::BluePoint;
        }
    }

    if (CurrentPuzzle.WallPositions.Contains(Position))
    {
        return EStrokeCellType::Wall;
    }

    for (const FTeleportPortal& Portal : CurrentPuzzle.TeleportPortals)
    {
        if (Portal.PortalA == Position || Portal.PortalB == Position)
        {
            return EStrokeCellType::Empty;
        }
    }

    return EStrokeCellType::Empty;
}

bool UStrokeGrid::AreAllRequiredPointsVisited() const
{
    for (const FIntPoint& RequiredPoint : CurrentPuzzle.RequiredPoints)
    {
        if (!VisitedRequiredPoints.Contains(RequiredPoint))
        {
            return false;
        }
    }

    return true;
}

bool UStrokeGrid::IsRGBOrderCorrect() const
{
    if (VisitedRequiredPoints.Num() < 3)
    {
        return false;
    }

    if (GetCellTypeAtPosition(VisitedRequiredPoints[0]) != EStrokeCellType::RedPoint)
    {
        return false;
    }

    if (GetCellTypeAtPosition(VisitedRequiredPoints[1]) != EStrokeCellType::GreenPoint)
    {
        return false;
    }

    if (GetCellTypeAtPosition(VisitedRequiredPoints[2]) != EStrokeCellType::BluePoint)
    {
        return false;
    }

    return true;
}

void UStrokeGrid::UpdatePathDisplay()
{
    if (!bShowPath) return;

    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell && Cell->CellData.GridPosition != CurrentPlayerPosition)
        {
            Cell->UpdatePathConnections(VisitedPositions, CurrentPlayerPosition);
        }
        else if (Cell)
        {
            Cell->ConnectedDirections.Empty();
            Cell->DrawPathLines();
        }
    }
}

void UStrokeGrid::UpdatePathColor()
{
    if (VisitedRequiredPoints.Num() == 0)
    {
        CurrentPathLineColor = DefaultPathLineColor;
        return;
    }

    FIntPoint LastVisitedPoint = VisitedRequiredPoints.Last();
    EStrokeCellType LastType = GetCellTypeAtPosition(LastVisitedPoint);

    switch (LastType)
    {
    case EStrokeCellType::RedPoint:
        CurrentPathLineColor = RedPointColor;
        break;
    case EStrokeCellType::GreenPoint:
        CurrentPathLineColor = GreenPointColor;
        break;
    case EStrokeCellType::BluePoint:
        CurrentPathLineColor = BluePointColor;
        break;
    default:
        CurrentPathLineColor = DefaultPathLineColor;
        break;
    }
}

void UStrokeGrid::UpdateStatusText()
{
}

void UStrokeGrid::UpdatePlayerVisual()
{
    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell)
        {
            bool bShouldHavePlayer = (Cell->CellData.GridPosition == CurrentPlayerPosition);
            Cell->SetPlayerPresence(bShouldHavePlayer);
        }
    }
}

void UStrokeGrid::UpdateEditorVisuals()
{
    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell)
        {
            Cell->UpdateDebugDisplay(bShowGridNumbers);
        }
    }
}

bool UStrokeGrid::IsPositionValid(FIntPoint Position) const
{
    return Position.X >= 0 && Position.X < EditorGridSize.X &&
        Position.Y >= 0 && Position.Y < EditorGridSize.Y;
}

void UStrokeGrid::ClampPositionsToGrid()
{
    EditorStartPosition.X = FMath::Clamp(EditorStartPosition.X, 0, EditorGridSize.X - 1);
    EditorStartPosition.Y = FMath::Clamp(EditorStartPosition.Y, 0, EditorGridSize.Y - 1);

    EditorGoalPosition.X = FMath::Clamp(EditorGoalPosition.X, 0, EditorGridSize.X - 1);
    EditorGoalPosition.Y = FMath::Clamp(EditorGoalPosition.Y, 0, EditorGridSize.Y - 1);

    for (int32 i = EditorRequiredPoints.Num() - 1; i >= 0; i--)
    {
        if (IsPositionValid(EditorRequiredPoints[i]))
        {
            EditorRequiredPoints[i].X = FMath::Clamp(EditorRequiredPoints[i].X, 0, EditorGridSize.X - 1);
            EditorRequiredPoints[i].Y = FMath::Clamp(EditorRequiredPoints[i].Y, 0, EditorGridSize.Y - 1);
        }
        else
        {
            EditorRequiredPoints.RemoveAt(i);
        }
    }

    for (int32 i = EditorWallPositions.Num() - 1; i >= 0; i--)
    {
        if (IsPositionValid(EditorWallPositions[i]))
        {
            EditorWallPositions[i].X = FMath::Clamp(EditorWallPositions[i].X, 0, EditorGridSize.X - 1);
            EditorWallPositions[i].Y = FMath::Clamp(EditorWallPositions[i].Y, 0, EditorGridSize.Y - 1);
        }
        else
        {
            EditorWallPositions.RemoveAt(i);
        }
    }

    for (int32 i = EditorTeleportPortals.Num() - 1; i >= 0; i--)
    {
        bool bPortalAValid = IsPositionValid(EditorTeleportPortals[i].PortalA);
        bool bPortalBValid = IsPositionValid(EditorTeleportPortals[i].PortalB);

        if (!bPortalAValid && !bPortalBValid)
        {
            EditorTeleportPortals.RemoveAt(i);
        }
        else
        {
            if (!bPortalAValid) EditorTeleportPortals[i].PortalA = FIntPoint(-1, -1);
            if (!bPortalBValid) EditorTeleportPortals[i].PortalB = FIntPoint(-1, -1);
        }
    }
}

void UStrokeGrid::AutoPlaceStartGoal()
{
    EditorStartPosition = FIntPoint(EditorGridSize.X / 2, EditorGridSize.Y - 1);
    EditorGoalPosition = FIntPoint(EditorGridSize.X / 2, 0);
}

void UStrokeGrid::SetupDefaultPuzzle()
{
    CurrentPuzzle.PuzzleName = TEXT("Default Puzzle");
    CurrentPuzzle.GridSize = EditorGridSize;
    CurrentPuzzle.StartPosition = EditorStartPosition;
    CurrentPuzzle.GoalPosition = EditorGoalPosition;
    CurrentPuzzle.RequiredPoints = EditorRequiredPoints;
    CurrentPuzzle.WallPositions = EditorWallPositions;
    CurrentPuzzle.TeleportPortals = EditorTeleportPortals;
}

FTeleportPortal* UStrokeGrid::FindTeleportPortal(int32 PortalID)
{
    for (FTeleportPortal& Portal : EditorTeleportPortals)
    {
        if (Portal.PortalID == PortalID)
        {
            return &Portal;
        }
    }
    return nullptr;
}

void UStrokeGrid::ToggleTeleportPortal(FIntPoint Position, int32 PortalID)
{
    FTeleportPortal* ExistingPortal = FindTeleportPortal(PortalID);

    if (ExistingPortal)
    {
        if (ExistingPortal->PortalA == Position)
        {
            ExistingPortal->PortalA = FIntPoint(-1, -1);
        }
        else if (ExistingPortal->PortalB == Position)
        {
            ExistingPortal->PortalB = FIntPoint(-1, -1);
        }
        else
        {
            if (ExistingPortal->PortalA.X == -1)
            {
                ExistingPortal->PortalA = Position;
            }
            else if (ExistingPortal->PortalB.X == -1)
            {
                ExistingPortal->PortalB = Position;
            }
            else
            {
                ExistingPortal->PortalA = Position;
            }
        }

        if (ExistingPortal->PortalA.X == -1 && ExistingPortal->PortalB.X == -1)
        {
            EditorTeleportPortals.RemoveAll([PortalID](const FTeleportPortal& Portal)
                {
                    return Portal.PortalID == PortalID;
                });
        }
    }
    else
    {
        FTeleportPortal NewPortal;
        NewPortal.PortalID = PortalID;
        NewPortal.PortalA = Position;
        NewPortal.PortalB = FIntPoint(-1, -1);
        EditorTeleportPortals.Add(NewPortal);
    }
}

void UStrokeGrid::RemoveFromAllTeleportPortals(FIntPoint Position)
{
    for (int32 i = EditorTeleportPortals.Num() - 1; i >= 0; i--)
    {
        FTeleportPortal& Portal = EditorTeleportPortals[i];

        if (Portal.PortalA == Position)
        {
            Portal.PortalA = FIntPoint(-1, -1);
        }
        if (Portal.PortalB == Position)
        {
            Portal.PortalB = FIntPoint(-1, -1);
        }

        if (Portal.PortalA.X == -1 && Portal.PortalB.X == -1)
        {
            EditorTeleportPortals.RemoveAt(i);
        }
    }
}

void UStrokeGrid::OnResetClicked()
{
    if (bEditMode)
    {
        ApplyEditorSettings();
    }
    else
    {
        ResetGame();
    }
}

void UStrokeGrid::CalculateCellSize()
{
    if (!GridPanel || CurrentPuzzle.GridSize.X <= 0 || CurrentPuzzle.GridSize.Y <= 0)
    {
        CurrentCellSize = MinCellSize;
        return;
    }

    float AvailableWidth = MaxGridWidth / CurrentPuzzle.GridSize.X;
    float AvailableHeight = MaxGridHeight / CurrentPuzzle.GridSize.Y;

    CurrentCellSize = FMath::Max(FMath::Min(AvailableWidth, AvailableHeight) - CellPadding, MinCellSize);
}

void UStrokeGrid::ApplyGridLayout()
{
    if (!GridPanel)
    {
        return;
    }

    CalculateCellSize();

    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell)
        {

        }
    }
}

void UStrokeGrid::ResetGame()
{
    GameState = EStrokeGameState::Playing;
    CurrentPlayerPosition = CurrentPuzzle.StartPosition;
    VisitedPositions.Empty();
    VisitedRequiredPoints.Empty();
    CurrentPathLineColor = DefaultPathLineColor;

    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell)
        {
            Cell->SetVisited(false);
            Cell->SetPlayerPresence(false);
            Cell->UpdateInteractionState(bEditMode);
        }
    }

    UStrokeCell* StartCell = GetCellAtPosition(CurrentPlayerPosition);
    if (StartCell)
    {
        StartCell->SetPlayerPresence(true);
    }

    if (StatusText)
    {
        StatusText->SetVisibility(ESlateVisibility::Hidden);
    }

    OnProgressReset();
    UpdatePathDisplay();
    UpdateProgressBar();
    SetKeyboardFocus();
}

void UStrokeGrid::PlaySound(USoundBase* Sound, float Volume)
{
    if (bEnableSounds && Sound && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Sound, Volume);
    }
}

bool UStrokeGrid::MovePlayer(FIntPoint Direction)
{
    FIntPoint NewPosition = CurrentPlayerPosition + Direction;

    if (!IsValidMove(NewPosition))
    {
        return false;
    }

    PlaySound(MoveSound, MoveSoundVolume);

    UStrokeCell* OldCell = GetCellAtPosition(CurrentPlayerPosition);
    if (OldCell)
    {
        OldCell->SetPlayerPresence(false);
        OldCell->SetVisited(true);
    }

    VisitedPositions.Add(CurrentPlayerPosition);

    EStrokeCellType OldCellType = GetCellTypeAtPosition(CurrentPlayerPosition);
    if (OldCellType == EStrokeCellType::RedPoint ||
        OldCellType == EStrokeCellType::GreenPoint ||
        OldCellType == EStrokeCellType::BluePoint)
    {
        if (!VisitedRequiredPoints.Contains(CurrentPlayerPosition))
        {
            VisitedRequiredPoints.Add(CurrentPlayerPosition);
            UpdatePathColor();

            if (OldCellType == EStrokeCellType::RedPoint)
            {
                PlaySound(RedPointSound, PointSoundVolume);
                OnRedPointCollected();
            }
            else if (OldCellType == EStrokeCellType::GreenPoint)
            {
                PlaySound(GreenPointSound, PointSoundVolume);
                OnGreenPointCollected();
            }
            else if (OldCellType == EStrokeCellType::BluePoint)
            {
                PlaySound(BluePointSound, PointSoundVolume);
                OnBluePointCollected();
            }
        }
    }

    CurrentPlayerPosition = NewPosition;

    FIntPoint FinalPosition = CheckTeleport(CurrentPlayerPosition);
    if (FinalPosition != CurrentPlayerPosition)
    {
        UStrokeCell* TeleportEntryCell = GetCellAtPosition(CurrentPlayerPosition);
        if (TeleportEntryCell)
        {
            TeleportEntryCell->SetVisited(true);
        }

        CurrentPlayerPosition = FinalPosition;

        UStrokeCell* TeleportExitCell = GetCellAtPosition(CurrentPlayerPosition);
        if (TeleportExitCell)
        {
            TeleportExitCell->SetVisited(true);
        }
    }

    UStrokeCell* NewCell = GetCellAtPosition(CurrentPlayerPosition);
    if (NewCell)
    {
        NewCell->SetPlayerPresence(true);
    }

    UpdatePathDisplay();
    UpdateProgressBar();
    CheckWinCondition();

    return true;
}


void UStrokeGrid::OnGameWon()
{

    PlaySound(PuzzleCompleteSound, PuzzleCompleteSoundVolume);

    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell)
        {
            Cell->UpdateInteractionState(false);
        }
    }

    ShowClearMessage();

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            OnPuzzleCompleted();
        }, 1.0f, false);
}


void UStrokeGrid::UpdateProgressBar()
{
    if (!ProgressBar)
    {
        return;
    }

    bool bHasRed = false;
    bool bHasGreen = false;
    bool bHasBlue = false;

    for (const FIntPoint& VisitedPoint : VisitedRequiredPoints)
    {
        EStrokeCellType CellType = GetCellTypeAtPosition(VisitedPoint);

        if (CellType == EStrokeCellType::RedPoint)
        {
            bHasRed = true;
        }
        else if (CellType == EStrokeCellType::GreenPoint)
        {
            bHasGreen = true;
        }
        else if (CellType == EStrokeCellType::BluePoint)
        {
            bHasBlue = true;
        }
    }

    bool bAtGoal = (CurrentPlayerPosition == CurrentPuzzle.GoalPosition);

    if (bHasRed && bHasGreen && bHasBlue && bAtGoal && ProgressGoalImage)
    {
        ProgressBar->SetBrushFromTexture(ProgressGoalImage);
    }
    else if (bHasBlue && ProgressBlueImage)
    {
        ProgressBar->SetBrushFromTexture(ProgressBlueImage);
    }
    else if (bHasGreen && ProgressGreenImage)
    {
        ProgressBar->SetBrushFromTexture(ProgressGreenImage);
    }
    else if (bHasRed && ProgressRedImage)
    {
        ProgressBar->SetBrushFromTexture(ProgressRedImage);
    }
    else if (ProgressStartImage)
    {
        ProgressBar->SetBrushFromTexture(ProgressStartImage);
    }
}