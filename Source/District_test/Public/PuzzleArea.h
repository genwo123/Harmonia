// PuzzleArea.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PuzzleArea.generated.h"

// �� ���� ����
UENUM(BlueprintType)
enum class ECellState : uint8
{
    Walkable,       // �ʷϻ� - �̵� ����
    Unwalkable,     // ������ - �̵� �Ұ���
    PedestalSlot    // ����� - ��ħ�� ��ġ ����
};

// ���� ���� (������/�ſ� ���� � ���)
UENUM(BlueprintType)
enum class EGridDirection : uint8
{
    North,
    East,
    South,
    West
};

// �׸��� �� ����ü
USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()

    // �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    ECellState State;

    // ���� ��ġ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    FVector WorldLocation;

    // ���� ��ġ�� ���� (��ħ�� �Ǵ� ���� ���)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    AActor* PlacedActor;

    // �ʱ�ȭ
    FGridCell()
    {
        State = ECellState::Walkable;
        WorldLocation = FVector::ZeroVector;
        PlacedActor = nullptr;
    }
};

UCLASS()
class DISTRICT_TEST_API APuzzleArea : public AActor
{
    GENERATED_BODY()

public:
    APuzzleArea();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaTime) override;

    // ��Ʈ ������Ʈ�� ����� �ڽ�
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* AreaBox;

    // Ÿ�� �޽� �ν��Ͻ� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* WalkableTiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* UnwalkableTiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent* PedestalSlotTiles;

public:
    // �⺻ �Ӽ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridRows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridColumns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float CellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float GridHeight;

    // �׸��� ����� ǥ�� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugGrid;

    // Ÿ�� �޽� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* WalkableTileMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* UnwalkableTileMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* PedestalSlotTileMesh;

    // Ÿ�� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor WalkableColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor UnwalkableColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor PedestalSlotColor;

    // ��ħ�� Ŭ���� (������ �� ���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    TSubclassOf<AActor> PedestalClass;

    // �׸��� ������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Data")
    TArray<FGridCell> Grid;

    // �׸��� �ʱ�ȭ �Լ�
    UFUNCTION(BlueprintCallable, Category = "Grid Setup")
    void InitializeGrid();

    // Ÿ�� �޽� ������Ʈ
    UFUNCTION(BlueprintCallable, Category = "Grid Setup")
    void UpdateTileMeshes();

    // �� ���� ����
    UFUNCTION(BlueprintCallable, Category = "Grid Setup")
    void SetCellState(int32 Row, int32 Column, ECellState NewState);

    // �� ���� ��������
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    ECellState GetCellState(int32 Row, int32 Column) const;

    // ���� ��ġ���� �� ���� ��������
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    ECellState GetCellStateFromWorldLocation(const FVector& WorldLocation) const;

    // ���� ��ġ���� �׸��� �ε��� ��������
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const;

    // �׸��� �ε������� ���� ��ġ ��������
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    FVector GetWorldLocationFromGridIndex(int32 Row, int32 Column) const;

    // ��ȿ�� �ε������� Ȯ��
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool IsValidIndex(int32 Row, int32 Column) const;

    // ���ڿ� ��ġ ����(���߱�)
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    FVector SnapToGrid(const FVector& WorldLocation) const;

    // ��ħ�� ��ġ �������� Ȯ��
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool CanPlacePedestalAt(int32 Row, int32 Column) const;

    // ��ħ�� ��ġ
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* PlacePedestal(int32 Row, int32 Column);

    // ��� ��ħ�� �ڵ� ��ġ (����� ����)
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void PlaceAllPedestals();

    // Ư�� ���� ��ġ�� ���� ��������
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    AActor* GetActorAtCell(int32 Row, int32 Column) const;

    // Ư�� ���� ���� ��ġ
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool PlaceActorAtCell(AActor* Actor, int32 Row, int32 Column);

    // ���� ��ġ�� ���� ��ġ (���� ����� ���� ����)
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool PlaceActorAtWorldLocation(AActor* Actor, const FVector& WorldLocation);

    // ������ �� ��������
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool GetAdjacentCell(int32 Row, int32 Column, EGridDirection Direction, int32& OutRow, int32& OutColumn) const;

    // ����׿� ���� �׸���
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugGrid(float Duration = 0.0f);

    // ����� ��� ���
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugMode();

    // �׸��� ���� ������ ��������
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    FVector GetGridSize() const;

    // ��� �� ���� ���� (���̺��)
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    TArray<uint8> SaveCellStates() const;

    // ��� �� ���� �ε�
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    void LoadCellStates(const TArray<uint8>& CellStates);

    // ���� ���� (��� ��ġ�� ���� ����)
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void ResetPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void UpdateGridSizeFromActorScale();

    // Ư�� ũ��� �׸��� ����
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetGridSize(float Width, float Height);

private:
    // �׸��� �ε��� <-> 1D �迭 �ε��� ��ȯ ���� �Լ�
    int32 GetIndexFrom2DCoord(int32 Row, int32 Column) const;
    void Get2DCoordFromIndex(int32 Index, int32& OutRow, int32& OutColumn) const;
};