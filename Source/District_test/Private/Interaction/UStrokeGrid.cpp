#include "Interaction/UStrokeGrid.h"
#include "Interaction/UStrokeCell.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"

UStrokeGrid::UStrokeGrid(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CurrentPlayerPosition = FIntPoint(0, 0);
    GameState = EStrokeGameState::Ready;
    bEditMode = false;
    CurrentTeleportID = 1;

    // ������ �⺻��
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

    ApplyEditorSettings();
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
        MovePlayer(FIntPoint(-1, 0)); // ���� = X ����
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::S || PressedKey == EKeys::Down)
    {
        MovePlayer(FIntPoint(1, 0));  // �Ʒ��� = X ����
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::A || PressedKey == EKeys::Left)
    {
        MovePlayer(FIntPoint(0, -1)); // ���� = Y ����
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::D || PressedKey == EKeys::Right)
    {
        MovePlayer(FIntPoint(0, 1));  // ������ = Y ����
        return FReply::Handled();
    }
    else if (PressedKey == EKeys::R)
    {
        ResetGame();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
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

    UE_LOG(LogTemp, Log, TEXT("Applied editor settings - Grid: %dx%d"),
        EditorGridSize.X, EditorGridSize.Y);
}
void UStrokeGrid::ResizeGrid()
{
    ClampPositionsToGrid();

    if (!IsPositionValid(EditorStartPosition) || !IsPositionValid(EditorGoalPosition))
    {
        AutoPlaceStartGoal();
    }

    ApplyEditorSettings();

    UE_LOG(LogTemp, Log, TEXT("Grid resized to: %dx%d"), EditorGridSize.X, EditorGridSize.Y);
}

void UStrokeGrid::ValidatePositions()
{
    TSet<FIntPoint> UsedPositions;

    // ������ �߰�
    UsedPositions.Add(EditorStartPosition);

    // �� ������ �������� ������ ����
    if (EditorGoalPosition == EditorStartPosition)
    {
        if (EditorGoalPosition.Y > 0)
            EditorGoalPosition.Y--;
        else
            EditorGoalPosition.Y++;
    }
    UsedPositions.Add(EditorGoalPosition);

    // RGB ����Ʈ �ߺ� ����
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

    // �� �ߺ� ���� (������, ��, RGB�� ��ġ�� �͸�)
    for (int32 i = EditorWallPositions.Num() - 1; i >= 0; i--)
    {
        if (UsedPositions.Contains(EditorWallPositions[i]))
        {
            EditorWallPositions.RemoveAt(i);
        }
    }

    // �ڷ���Ʈ ���� �ߺ� ����
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

        // �� ���� ����
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

    UE_LOG(LogTemp, Log, TEXT("Testing puzzle..."));
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
    UE_LOG(LogTemp, Log, TEXT("Auto-placed RGB points"));
}

void UStrokeGrid::ClearAllTeleportPortals()
{
    EditorTeleportPortals.Empty();
    ApplyEditorSettings();
    UE_LOG(LogTemp, Log, TEXT("Cleared all teleport portals"));
}

void UStrokeGrid::AutoCompleteTeleportPairs()
{
    for (int32 i = EditorTeleportPortals.Num() - 1; i >= 0; i--)
    {
        FTeleportPortal& Portal = EditorTeleportPortals[i];

        // �̿ϼ� ���� �� ã��
        if (Portal.PortalA.X == -1 || Portal.PortalB.X == -1)
        {
            // �� ������ �ڵ� ��ġ
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

            // ������ �̿ϼ��̸� ����
            if (Portal.PortalA.X == -1 || Portal.PortalB.X == -1)
            {
                EditorTeleportPortals.RemoveAt(i);
            }
        }
    }

    ApplyEditorSettings();
    UE_LOG(LogTemp, Log, TEXT("Auto-completed teleport pairs"));
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
        // �ڷ���Ʈ ���� ó�� (��Ÿ Ÿ�Ե�)
        ToggleTeleportPortal(Position, CurrentTeleportID);
        break;
    }

    ApplyEditorSettings();
}

// ========== ���� �Լ��� ==========

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
        UE_LOG(LogTemp, Error, TEXT("GridPanel is NULL! Cannot create cells."));
        return;
    }

    UClass* ClassToUse = StrokeCellWidgetClass ? StrokeCellWidgetClass.Get() : UStrokeCell::StaticClass();
    CellWidgets.Empty();

    // X�� Y ���� ���� �ٲٱ� - X�� ���� (�� �켱)
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

            // GridPanel ��ġ�� Row=X, Column=Y�� ����
            GridPanel->AddChildToUniformGrid(NewCell, X, Y);
            CellWidgets.Add(NewCell);
        }
    }

    UpdateEditorVisuals();
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

    // �ڷ���Ʈ ������ ��� Empty�� ó�� (������ ���� ó��)
    for (const FTeleportPortal& Portal : CurrentPuzzle.TeleportPortals)
    {
        if (Portal.PortalA == Position || Portal.PortalB == Position)
        {
            return EStrokeCellType::Empty;
        }
    }

    return EStrokeCellType::Empty;
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

bool UStrokeGrid::MovePlayer(FIntPoint Direction)
{
    

    FIntPoint NewPosition = CurrentPlayerPosition + Direction;



    if (!IsValidMove(NewPosition))
    {
        return false;
    }

    // ���� ��ġ ó��
    UStrokeCell* OldCell = GetCellAtPosition(CurrentPlayerPosition);
    if (OldCell)
    {
        OldCell->SetPlayerPresence(false);
        OldCell->SetVisited(true);
    }

    VisitedPositions.Add(CurrentPlayerPosition);

    // RGB ����Ʈ �湮 üũ
    EStrokeCellType OldCellType = GetCellTypeAtPosition(CurrentPlayerPosition);
    if (OldCellType == EStrokeCellType::RedPoint ||
        OldCellType == EStrokeCellType::GreenPoint ||
        OldCellType == EStrokeCellType::BluePoint)
    {
        if (!VisitedRequiredPoints.Contains(CurrentPlayerPosition))
        {
            VisitedRequiredPoints.Add(CurrentPlayerPosition);
        }
    }

    // �� ��ġ�� �̵�
    CurrentPlayerPosition = NewPosition;

    // �ڷ���Ʈ üũ
    FIntPoint FinalPosition = CheckTeleport(CurrentPlayerPosition);
    if (FinalPosition != CurrentPlayerPosition)
    {
        CurrentPlayerPosition = FinalPosition;
        UE_LOG(LogTemp, Log, TEXT("Teleported from (%d,%d) to (%d,%d)"),
            NewPosition.X, NewPosition.Y, FinalPosition.X, FinalPosition.Y);
    }

    UStrokeCell* NewCell = GetCellAtPosition(CurrentPlayerPosition);
    if (NewCell)
    {
        NewCell->SetPlayerPresence(true);
    }

    // ��� ǥ�� ������Ʈ �߰�
    UpdatePathDisplay();

    CheckWinCondition();
    UpdateStatusText();

    return true;
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

void UStrokeGrid::CheckWinCondition()
{
    if (CurrentPlayerPosition != CurrentPuzzle.GoalPosition)
    {
        return;
    }

    if (!AreAllRequiredPointsVisited())
    {
        UE_LOG(LogTemp, Warning, TEXT("Reached goal but missing required points!"));
        return;
    }

    GameState = EStrokeGameState::Won;
    UpdateStatusText();
    OnGameWon();

    UE_LOG(LogTemp, Log, TEXT("GAME WON! All required points visited and reached goal!"));
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

void UStrokeGrid::OnGameWon()
{
    // Ŭ���� �޽��� ǥ��
    if (StatusText)
    {
        StatusText->SetText(FText::FromString(TEXT("Clear!")));
    }

    // �Է� ��Ȱ��ȭ
    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell && Cell->CellButton)
        {
            Cell->CellButton->SetIsEnabled(false);
        }
    }

    // 3�� �� Blueprint �̺�Ʈ ȣ��
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            OnPuzzleCompleted(); // Blueprint���� ������ �̺�Ʈ
        }, 3.0f, false);
}

void UStrokeGrid::ResetGame()
{
    GameState = EStrokeGameState::Playing;
    CurrentPlayerPosition = CurrentPuzzle.StartPosition;
    VisitedPositions.Empty();
    VisitedRequiredPoints.Empty();

    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell)
        {
            Cell->SetVisited(false);
            Cell->SetPlayerPresence(false);

            if (Cell->CellButton)
            {
                Cell->CellButton->SetIsEnabled(true);
            }
        }
    }

    UStrokeCell* StartCell = GetCellAtPosition(CurrentPlayerPosition);
    if (StartCell)
    {
        StartCell->SetPlayerPresence(true);
    }

    // ��� ǥ�� �ʱ�ȭ
    UpdatePathDisplay();

    UpdateStatusText();
    SetKeyboardFocus();

}


void UStrokeGrid::UpdatePathDisplay()
{
    if (!bShowPath) return;

    // ���� �÷��̾� ��ġ�� �ƴ� ���鸸 ������Ʈ
    for (UStrokeCell* Cell : CellWidgets)
    {
        if (Cell && Cell->CellData.GridPosition != CurrentPlayerPosition)
        {
            Cell->UpdatePathConnections(VisitedPositions, CurrentPlayerPosition);
        }
        else if (Cell)
        {
            // ���� �÷��̾� ��ġ�� ���� ����
            Cell->ConnectedDirections.Empty();
            Cell->DrawPathLines();
        }
    }
}



void UStrokeGrid::UpdateStatusText()
{
    if (!StatusText) return;

    FString StatusString;

    switch (GameState)
    {
    case EStrokeGameState::Ready:
        if (bEditMode)
        {
            StatusString = TEXT("EDIT MODE - Click cells to modify, Test Puzzle to play");
        }
        else
        {
            StatusString = TEXT("Press any key to start");
        }
        break;

    case EStrokeGameState::Playing:
        StatusString = FString::Printf(TEXT("Points: %d/%d | Position: (%d, %d) | WASD to move | R to reset"),
            VisitedRequiredPoints.Num(),
            CurrentPuzzle.RequiredPoints.Num(),
            CurrentPlayerPosition.X,
            CurrentPlayerPosition.Y);
        break;

    case EStrokeGameState::Won:
        StatusString = TEXT("CONGRATULATIONS! Press R to play again");
        break;

    case EStrokeGameState::Lost:
        StatusString = TEXT("Game Over! Press R to try again");
        break;
    }

    StatusText->SetText(FText::FromString(StatusString));
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

// ========== ������ ���� �Լ��� ==========

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

    UE_LOG(LogTemp, Log, TEXT("Auto-placed Start: (%d,%d), Goal: (%d,%d)"),
        EditorStartPosition.X, EditorStartPosition.Y,
        EditorGoalPosition.X, EditorGoalPosition.Y);
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

// ========== �ڷ���Ʈ ���� �Լ��� ==========

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

