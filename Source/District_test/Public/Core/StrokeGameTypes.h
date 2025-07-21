// StrokeGameTypes.h
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
    BluePoint   UMETA(DisplayName = "Blue Point")
};

// ���� ���� ������
UENUM(BlueprintType)
enum class EStrokeGameState : uint8
{
    Ready       UMETA(DisplayName = "Ready"),
    Playing     UMETA(DisplayName = "Playing"),
    Success     UMETA(DisplayName = "Success"),
    Failed      UMETA(DisplayName = "Failed")
};

// ���� �� ���� ����ü
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeCell
{
    GENERATED_BODY()

    // �⺻ �Ӽ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    EStrokeCellType CellType = EStrokeCellType::Empty;

    // �湮 ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsVisited = false;

    // ���� �ý��� (���� Ȯ���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
    int32 RequiredOrder = 0; // 0=��������, 1,2,3=�������

    // ������
    FStrokeCell()
    {
        CellType = EStrokeCellType::Empty;
        bIsVisited = false;
        RequiredOrder = 0;
    }

    // ���� �Լ���
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

// �� ��ü ������ ����ü
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeMapData
{
    GENERATED_BODY()

    // ���� ũ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "3", ClampMax = "10"))
    int32 Rows = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map", meta = (ClampMin = "3", ClampMax = "10"))
    int32 Columns = 5;

    // �� ������ �迭 (ũ��: Rows * Columns)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    TArray<FStrokeCell> Cells;

    // ����/���� ���� (�ڵ����� ã�ų� ���� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FIntPoint StartPos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
    FIntPoint GoalPos;

    // ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bRequireOrder = false; // true�� RGB ������� �湮 �ʿ�

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FString MapName = "Untitled Map";

    // ������
    FStrokeMapData()
    {
        Rows = 5;
        Columns = 5;
        StartPos = FIntPoint(0, 0);
        GoalPos = FIntPoint(4, 4);
        bRequireOrder = false;
        MapName = "Untitled Map";

        // �⺻ �� ����� �ʱ�ȭ
        InitializeEmptyCells();
    }

    // ���� �Լ���
    void InitializeEmptyCells()
    {
        Cells.Empty();
        Cells.SetNum(Rows * Columns);
        for (int32 i = 0; i < Cells.Num(); i++)
        {
            Cells[i] = FStrokeCell();
        }
    }

    // 2D ��ǥ�� 1D �ε����� ��ȯ
    int32 GetIndex(int32 Row, int32 Col) const
    {
        if (Row < 0 || Row >= Rows || Col < 0 || Col >= Columns)
        {
            return -1; // ��ȿ���� ���� �ε���
        }
        return Row * Columns + Col;
    }

    // 1D �ε����� 2D ��ǥ�� ��ȯ
    FIntPoint GetCoords(int32 Index) const
    {
        if (Index < 0 || Index >= Cells.Num())
        {
            return FIntPoint(-1, -1); // ��ȿ���� ���� ��ǥ
        }
        return FIntPoint(Index / Columns, Index % Columns);
    }

    // Ư�� ��ǥ�� �� ��������
    FStrokeCell* GetCell(int32 Row, int32 Col)
    {
        int32 Index = GetIndex(Row, Col);
        return (Index >= 0) ? &Cells[Index] : nullptr;
    }

    // Ư�� ��ǥ�� �� �������� (const)
    const FStrokeCell* GetCell(int32 Row, int32 Col) const
    {
        int32 Index = GetIndex(Row, Col);
        return (Index >= 0) ? &Cells[Index] : nullptr;
    }

    // ������ ������ Ȯ�� (�����¿츸)
    bool AreAdjacent(FIntPoint From, FIntPoint To) const
    {
        int32 DeltaX = FMath::Abs(From.X - To.X);
        int32 DeltaY = FMath::Abs(From.Y - To.Y);
        return (DeltaX == 1 && DeltaY == 0) || (DeltaX == 0 && DeltaY == 1);
    }

    // �� ��ȿ�� �˻�
    bool IsValidMap() const
    {
        // �������� �������� ��ȿ���� Ȯ��
        if (StartPos.X < 0 || StartPos.X >= Rows || StartPos.Y < 0 || StartPos.Y >= Columns)
            return false;
        if (GoalPos.X < 0 || GoalPos.X >= Rows || GoalPos.Y < 0 || GoalPos.Y >= Columns)
            return false;

        // RGB ����Ʈ�� ���� �ּ� 1���� �ִ��� Ȯ��
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

// ���� ���� ���� ����ü
USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FStrokeGameProgress
{
    GENERATED_BODY()

    // ���� ����
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    EStrokeGameState GameState = EStrokeGameState::Ready;

    // ���� ��ġ
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    FIntPoint CurrentPos;

    // �̵� ���
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    TArray<FIntPoint> MovementPath;

    // RGB ���� ����
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    bool bRedCollected = false;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    bool bGreenCollected = false;

    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    bool bBlueCollected = false;

    // ���� ����
    UPROPERTY(BlueprintReadWrite, Category = "Progress")
    int32 NextRequiredOrder = 1;

    // ������
    FStrokeGameProgress()
    {
        GameState = EStrokeGameState::Ready;
        CurrentPos = FIntPoint(-1, -1);
        bRedCollected = false;
        bGreenCollected = false;
        bBlueCollected = false;
        NextRequiredOrder = 1;
    }

    // ���� �Լ���
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
    FLinearColor LineColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float LineThickness = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bShowGrid = true;

    // �����÷��� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bAllowDiagonalMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bShowPath = true;

    // ������
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