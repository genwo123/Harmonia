// StrokeGameTypes.h
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
    BluePoint   UMETA(DisplayName = "Blue Point")
};

// 게임 상태 열거형
UENUM(BlueprintType)
enum class EStrokeGameState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Playing     UMETA(DisplayName = "Playing"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed")
};

// 개별 셀 정보 구조체
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeCell
{
    GENERATED_BODY()

    // 기본 속성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    EStrokeCellType CellType = EStrokeCellType::Empty;

    // 방문 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsVisited = false;

    // 순서 시스템 (추후 확장용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
    int32 RequiredOrder = 0; // 0=순서없음, 1,2,3=순서대로

    // 생성자
    FStrokeCell()
    {
        CellType = EStrokeCellType::Empty;
        bIsVisited = false;
        RequiredOrder = 0;
    }

    // 편의 함수들
    bool IsWall() const { return CellType == EStrokeCellType::Wall; }
    bool IsEmpty() const { return CellType == EStrokeCellType::Empty; }
    bool IsStart() const { return CellType == EStrokeCellType::Start; }
    bool IsGoal() const { return CellType == EStrokeCellType::Goal; }
    bool IsRGBPoint() const {
        return CellType == EStrokeCellType::RedPoint ||
            CellType == EStrokeCellType::GreenPoint ||
            CellType == EStrokeCellType::BluePoint;
    }
};

// 맵 전체 데이터 구조체
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeMapData
{
    GENERATED_BODY()

    // 격자 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "3", ClampMax = "10"))
    int32 Rows = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "3", ClampMax = "10"))
    int32 Columns = 5;

    // 셀 데이터 배열 (크기: Rows * Columns)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    TArray<FStrokeCell> Cells;

    // 시작/도착 지점 (자동으로 찾거나 수동 설정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FIntPoint StartPos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FIntPoint GoalPos;

    // 게임 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bRequireOrder = false; // true시 RGB 순서대로 방문 필요

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FString MapName = "Untitled Map";

    // 생성자
    FStrokeMapData()
    {
        Rows = 5;
        Columns = 5;
        StartPos = FIntPoint(0, 0);
        GoalPos = FIntPoint(4, 4);
        bRequireOrder = false;
        MapName = "Untitled Map";

        // 기본 빈 셀들로 초기화
        InitializeEmptyCells();
    }

    // 편의 함수들
    void InitializeEmptyCells()
    {
        Cells.Empty();
        Cells.SetNum(Rows * Columns);
        for (int32 i = 0; i < Cells.Num(); i++)
        {
            Cells[i] = FStrokeCell();
        }
    }

    // 2D 좌표를 1D 인덱스로 변환
    int32 GetIndex(int32 Row, int32 Col) const
    {
        if (Row < 0 || Row >= Rows || Col < 0 || Col >= Columns)
        {
            return -1; // 유효하지 않은 인덱스
        }
        return Row * Columns + Col;
    }

    // 1D 인덱스를 2D 좌표로 변환
    FIntPoint GetCoords(int32 Index) const
    {
        if (Index < 0 || Index >= Cells.Num())
        {
            return FIntPoint(-1, -1); // 유효하지 않은 좌표
        }
        return FIntPoint(Index / Columns, Index % Columns);
    }

    // 특정 좌표의 셀 가져오기
    FStrokeCell* GetCell(int32 Row, int32 Col)
    {
        int32 Index = GetIndex(Row, Col);
        return (Index >= 0) ? &Cells[Index] : nullptr;
    }

    // 특정 좌표의 셀 가져오기 (const)
    const FStrokeCell* GetCell(int32 Row, int32 Col) const
    {
        int32 Index = GetIndex(Row, Col);
        return (Index >= 0) ? &Cells[Index] : nullptr;
    }

    // 인접한 셀인지 확인 (상하좌우만)
    bool AreAdjacent(FIntPoint From, FIntPoint To) const
    {
        int32 DeltaX = FMath::Abs(From.X - To.X);
        int32 DeltaY = FMath::Abs(From.Y - To.Y);
        return (DeltaX == 1 && DeltaY == 0) || (DeltaX == 0 && DeltaY == 1);
    }

    // 맵 유효성 검사
    bool IsValidMap() const
    {
        // 시작점과 도착점이 유효한지 확인
        if (StartPos.X < 0 || StartPos.X >= Rows || StartPos.Y < 0 || StartPos.Y >= Columns)
            return false;
        if (GoalPos.X < 0 || GoalPos.X >= Rows || GoalPos.Y < 0 || GoalPos.Y >= Columns)
            return false;

        // RGB 포인트가 각각 최소 1개씩 있는지 확인
        bool bHasRed = false, bHasGreen = false, bHasBlue = false;
        for (const FStrokeCell& Cell : Cells)
        {
            if (Cell.CellType == EStrokeCellType::RedPoint) bHasRed = true;
            if (Cell.CellType == EStrokeCellType::GreenPoint) bHasGreen = true;
            if (Cell.CellType == EStrokeCellType::BluePoint) bHasBlue = true;
        }

        return bHasRed && bHasGreen && bHasBlue;
    }
};

// 게임 진행 상태 구조체
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeGameProgress
{
    GENERATED_BODY()

    // 현재 상태
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    EStrokeGameState GameState = EStrokeGameState::Ready;

    // 현재 위치
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    FIntPoint CurrentPos;

    // 이동 경로
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    TArray<FIntPoint> MovementPath;

    // RGB 수집 상태
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    bool bRedCollected = false;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    bool bGreenCollected = false;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    bool bBlueCollected = false;

    // 순서 모드용
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    int32 NextRequiredOrder = 1;

    // 생성자
    FStrokeGameProgress()
    {
        GameState = EStrokeGameState::Ready;
        CurrentPos = FIntPoint(-1, -1);
        bRedCollected = false;
        bGreenCollected = false;
        bBlueCollected = false;
        NextRequiredOrder = 1;
    }

    // 편의 함수들
    bool AllRGBCollected() const
    {
        return bRedCollected && bGreenCollected && bBlueCollected;
    }

    void Reset()
    {
        GameState = EStrokeGameState::Ready;
        CurrentPos = FIntPoint(-1, -1);
        MovementPath.Empty();
        bRedCollected = false;
        bGreenCollected = false;
        bBlueCollected = false;
        NextRequiredOrder = 1;
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
    FLinearColor LineColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float LineThickness = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowGrid = true;

    // 게임플레이 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAllowDiagonalMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowPath = true;

    // 생성자
    FStrokeGameSettings()
    {
        CellSize = 50.0f;
        CellSpacing = 2.0f;
        LineColor = FLinearColor::White;
        LineThickness = 3.0f;
        bShowGrid = true;
        bAllowDiagonalMovement = false;
        bShowPath = true;
    }
};