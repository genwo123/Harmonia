#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StrokeGameTypes.generated.h"

// 셀 타입 열거형
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

// 게임 상태 열거형
UENUM(BlueprintType)
enum class EStrokeGameState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Playing     UMETA(DisplayName = "Playing"),
    Won         UMETA(DisplayName = "Won"),
    Lost        UMETA(DisplayName = "Lost")
};

// 텔레포트 포털 구조체
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

    // 포털이 완성되었는지 확인
    bool IsComplete() const
    {
        return PortalA.X != -1 && PortalB.X != -1;
    }

    // 포털이 비어있는지 확인
    bool IsEmpty() const
    {
        return PortalA.X == -1 && PortalB.X == -1;
    }

    // 특정 위치가 이 포털에 포함되는지 확인
    bool Contains(FIntPoint Position) const
    {
        return PortalA == Position || PortalB == Position;
    }

    // 입구 위치를 주면 출구 위치 반환
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
        return Entry; // 텔레포트 불가능
    }
};

// 셀 데이터 구조체
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

    // 편의 함수들
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

// 퍼즐 데이터 구조체 (DataTable용)
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
    float TimeLimit = 0.0f; // 0 = 무제한

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

    // 편의 함수들
    bool IsValidPuzzle() const
    {
        // 기본 유효성 검사
        if (GridSize.X < 3 || GridSize.Y < 3) return false;
        if (!IsPositionInGrid(StartPosition)) return false;
        if (!IsPositionInGrid(GoalPosition)) return false;
        if (StartPosition == GoalPosition) return false;

        // 필수 포인트가 최소 1개는 있어야 함
        if (RequiredPoints.Num() == 0) return false;

        // 모든 위치가 그리드 내에 있는지 확인
        for (const FIntPoint& Point : RequiredPoints)
        {
            if (!IsPositionInGrid(Point)) return false;
        }

        for (const FIntPoint& Wall : WallPositions)
        {
            if (!IsPositionInGrid(Wall)) return false;
        }

        // 텔레포트 포털 유효성 확인
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
        // 간단한 추정: 시작점 -> 모든 필수점 -> 골점
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

// 게임 진행 상태 구조체
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

// 게임 설정 구조체
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeGameSettings
{
    GENERATED_BODY()

    // 시각적 설정
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

    // 게임플레이 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAllowDiagonalMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowHints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float MovementSpeed = 1.0f;

    // 색상 설정
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