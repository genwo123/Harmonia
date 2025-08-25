#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StrokeGameTypes.generated.h"

// �� Ÿ�� ������
UENUM(BlueprintType)
enum class EStrokeCellType : uint8
{
    Empty       UMETA(DisplayName = "Empty"),
    Wall        UMETA(DisplayName = "Wall"),
    Start       UMETA(DisplayName = "Start"),
    Goal        UMETA(DisplayName = "Goal"),
    RedPoint    UMETA(DisplayName = "Red Point"),
    GreenPoint  UMETA(DisplayName = "Green Point"),
    BluePoint   UMETA(DisplayName = "Blue Point"),
    Visited     UMETA(DisplayName = "Visited")
};

// ���� ���� ������
UENUM(BlueprintType)
enum class EStrokeGameState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Playing     UMETA(DisplayName = "Playing"),
    Won         UMETA(DisplayName = "Won"),
    Lost        UMETA(DisplayName = "Lost")
};

// �ڷ���Ʈ ���� ����ü
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FTeleportPortal
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
    FIntPoint PortalA = FIntPoint(-1, -1);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
    FIntPoint PortalB = FIntPoint(-1, -1);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
    int32 PortalID = 1;

    FTeleportPortal()
    {
        PortalA = FIntPoint(-1, -1);
        PortalB = FIntPoint(-1, -1);
        PortalID = 1;
    }

    FTeleportPortal(int32 ID)
    {
        PortalA = FIntPoint(-1, -1);
        PortalB = FIntPoint(-1, -1);
        PortalID = ID;
    }

    // ������ �ϼ��Ǿ����� Ȯ��
    bool IsComplete() const
    {
        return PortalA.X != -1 && PortalB.X != -1;
    }

    // ������ ����ִ��� Ȯ��
    bool IsEmpty() const
    {
        return PortalA.X == -1 && PortalB.X == -1;
    }

    // Ư�� ��ġ�� �� ���п� ���ԵǴ��� Ȯ��
    bool Contains(FIntPoint Position) const
    {
        return PortalA == Position || PortalB == Position;
    }

    // �Ա� ��ġ�� �ָ� �ⱸ ��ġ ��ȯ
    FIntPoint GetDestination(FIntPoint Entry) const
    {
        if (Entry == PortalA && PortalB.X != -1)
        {
            return PortalB;
        }
        else if (Entry == PortalB && PortalA.X != -1)
        {
            return PortalA;
        }
        return Entry; // �ڷ���Ʈ �Ұ���
    }
};

// �� ������ ����ü
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeCellData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    EStrokeCellType CellType = EStrokeCellType::Empty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    bool bIsVisited = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    FIntPoint GridPosition = FIntPoint(0, 0);

    FStrokeCellData()
    {
        CellType = EStrokeCellType::Empty;
        bIsVisited = false;
        GridPosition = FIntPoint(0, 0);
    }

    FStrokeCellData(EStrokeCellType InType, FIntPoint InPosition)
    {
        CellType = InType;
        bIsVisited = false;
        GridPosition = InPosition;
    }

    // ���� �Լ���
    bool IsSpecialCell() const
    {
        return CellType != EStrokeCellType::Empty && CellType != EStrokeCellType::Visited;
    }

    bool IsRequiredPoint() const
    {
        return CellType == EStrokeCellType::RedPoint ||
            CellType == EStrokeCellType::GreenPoint ||
            CellType == EStrokeCellType::BluePoint;
    }

    bool IsBlockingCell() const
    {
        return CellType == EStrokeCellType::Wall;
    }
};

// ���� ������ ����ü (DataTable��)
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokePuzzleData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString PuzzleName = TEXT("Untitled Puzzle");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info")
    FString Description = TEXT("Complete the puzzle by visiting all required points");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid",
        meta = (ClampMin = "3", ClampMax = "20"))
    FIntPoint GridSize = FIntPoint(5, 5);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Points")
    FIntPoint StartPosition = FIntPoint(2, 4);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Points")
    FIntPoint GoalPosition = FIntPoint(2, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Required Points")
    TArray<FIntPoint> RequiredPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
    TArray<FIntPoint> WallPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Features")
    TArray<FTeleportPortal> TeleportPortals;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty",
        meta = (ClampMin = "1", ClampMax = "10"))
    int32 DifficultyLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty",
        meta = (ClampMin = "0"))
    float TimeLimit = 0.0f; // 0 = ������

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAllowRestart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowHints = false;

    FStrokePuzzleData()
    {
        PuzzleName = TEXT("Tutorial Puzzle");
        Description = TEXT("Learn the basics of one-stroke puzzles");
        GridSize = FIntPoint(5, 5);
        StartPosition = FIntPoint(2, 4);
        GoalPosition = FIntPoint(2, 0);
        RequiredPoints = {
            FIntPoint(1, 2),
            FIntPoint(2, 2),
            FIntPoint(3, 2)
        };
        WallPositions = {
            FIntPoint(0, 1),
            FIntPoint(4, 1)
        };
        DifficultyLevel = 1;
        TimeLimit = 0.0f;
        bAllowRestart = true;
        bShowHints = false;
    }

    // ���� �Լ���
    bool IsValidPuzzle() const
    {
        // �⺻ ��ȿ�� �˻�
        if (GridSize.X < 3 || GridSize.Y < 3) return false;
        if (!IsPositionInGrid(StartPosition)) return false;
        if (!IsPositionInGrid(GoalPosition)) return false;
        if (StartPosition == GoalPosition) return false;

        // �ʼ� ����Ʈ�� �ּ� 1���� �־�� ��
        if (RequiredPoints.Num() == 0) return false;

        // ��� ��ġ�� �׸��� ���� �ִ��� Ȯ��
        for (const FIntPoint& Point : RequiredPoints)
        {
            if (!IsPositionInGrid(Point)) return false;
        }

        for (const FIntPoint& Wall : WallPositions)
        {
            if (!IsPositionInGrid(Wall)) return false;
        }

        // �ڷ���Ʈ ���� ��ȿ�� Ȯ��
        for (const FTeleportPortal& Portal : TeleportPortals)
        {
            if (Portal.IsComplete())
            {
                if (!IsPositionInGrid(Portal.PortalA) || !IsPositionInGrid(Portal.PortalB))
                    return false;
            }
        }

        return true;
    }

    bool IsPositionInGrid(FIntPoint Position) const
    {
        return Position.X >= 0 && Position.X < GridSize.X &&
            Position.Y >= 0 && Position.Y < GridSize.Y;
    }

    bool IsPositionBlocked(FIntPoint Position) const
    {
        return WallPositions.Contains(Position);
    }

    bool IsRequiredPosition(FIntPoint Position) const
    {
        return RequiredPoints.Contains(Position);
    }

    FTeleportPortal* FindPortalContaining(FIntPoint Position)
    {
        for (FTeleportPortal& Portal : TeleportPortals)
        {
            if (Portal.Contains(Position))
            {
                return &Portal;
            }
        }
        return nullptr;
    }

    const FTeleportPortal* FindPortalContaining(FIntPoint Position) const
    {
        for (const FTeleportPortal& Portal : TeleportPortals)
        {
            if (Portal.Contains(Position))
            {
                return &Portal;
            }
        }
        return nullptr;
    }

    int32 GetEstimatedMoves() const
    {
        // ������ ����: ������ -> ��� �ʼ��� -> ����
        return RequiredPoints.Num() + 2;
    }

    TArray<FIntPoint> GetAllSpecialPositions() const
    {
        TArray<FIntPoint> SpecialPositions;
        SpecialPositions.Add(StartPosition);
        SpecialPositions.Add(GoalPosition);
        SpecialPositions.Append(RequiredPoints);

        for (const FTeleportPortal& Portal : TeleportPortals)
        {
            if (Portal.PortalA.X != -1) SpecialPositions.Add(Portal.PortalA);
            if (Portal.PortalB.X != -1) SpecialPositions.Add(Portal.PortalB);
        }

        return SpecialPositions;
    }
};

// ���� ���� ���� ����ü
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeGameProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    EStrokeGameState GameState = EStrokeGameState::Ready;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    FIntPoint CurrentPosition = FIntPoint(0, 0);

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    TArray<FIntPoint> MovementPath;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    TArray<FIntPoint> VisitedRequiredPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    float ElapsedTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    int32 MoveCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    int32 TeleportCount = 0;

    FStrokeGameProgress()
    {
        GameState = EStrokeGameState::Ready;
        CurrentPosition = FIntPoint(0, 0);
        ElapsedTime = 0.0f;
        MoveCount = 0;
        TeleportCount = 0;
    }

    void Reset(FIntPoint StartPosition)
    {
        GameState = EStrokeGameState::Ready;
        CurrentPosition = StartPosition;
        MovementPath.Empty();
        VisitedRequiredPoints.Empty();
        ElapsedTime = 0.0f;
        MoveCount = 0;
        TeleportCount = 0;
    }

    bool AllRequiredPointsVisited(const TArray<FIntPoint>& RequiredPoints) const
    {
        for (const FIntPoint& Required : RequiredPoints)
        {
            if (!VisitedRequiredPoints.Contains(Required))
            {
                return false;
            }
        }
        return true;
    }

    float GetCompletionPercentage(const TArray<FIntPoint>& RequiredPoints) const
    {
        if (RequiredPoints.Num() == 0) return 100.0f;
        return (float)VisitedRequiredPoints.Num() / RequiredPoints.Num() * 100.0f;
    }

    void AddMove(FIntPoint NewPosition, bool bWasTeleport = false)
    {
        MovementPath.Add(CurrentPosition);
        CurrentPosition = NewPosition;
        MoveCount++;

        if (bWasTeleport)
        {
            TeleportCount++;
        }
    }
};

// ���� ���� ����ü
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeGameSettings
{
    GENERATED_BODY()

    // �ð��� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float CellSize = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float CellSpacing = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowGrid = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowCoordinates = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bAnimateMovement = true;

    // �����÷��� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAllowDiagonalMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowHints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float MovementSpeed = 1.0f;

    // ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor EmptyColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor WallColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor StartColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor GoalColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor VisitedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor PlayerColor = FLinearColor::Yellow;

    FStrokeGameSettings()
    {
        CellSize = 50.0f;
        CellSpacing = 2.0f;
        bShowGrid = true;
        bShowCoordinates = false;
        bShowPath = true;
        bAnimateMovement = true;
        bAllowDiagonalMovement = false;
        bShowHints = false;
        MovementSpeed = 1.0f;
    }
};