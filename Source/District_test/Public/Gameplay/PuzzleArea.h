// PuzzleArea.h - ����ϰ� ������ ����
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PuzzleArea.generated.h"

class APedestal;

// �� ���� ����
UENUM(BlueprintType)
enum class ECellState : uint8
{
    Walkable,       // �ʷϻ� - �̵� ����
    Unwalkable,     // ������ - �̵� �Ұ���/��
    PedestalSlot,   // ����� - ��ħ�� ��ġ ����
    Occupied        // �Ķ��� - ��ħ�밡 ��ġ��
};

// ���� ����
UENUM(BlueprintType)
enum class EGridDirection : uint8
{
    North,
    East,
    South,
    West
};

// �׸��� ��ǥ ����ü
USTRUCT(BlueprintType)
struct FGridCoordinate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinate")
    int32 Row = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordinate")
    int32 Column = 0;

    FGridCoordinate() = default;
    FGridCoordinate(int32 InRow, int32 InColumn) : Row(InRow), Column(InColumn) {}

    bool operator==(const FGridCoordinate& Other) const
    {
        return Row == Other.Row && Column == Other.Column;
    }
};

// �׸��� �� ����ü
USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()

    // �� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    ECellState State = ECellState::Walkable;

    // ���� ��ġ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    FVector WorldLocation = FVector::ZeroVector;

    // ���� ��ġ�� ���� (��ħ��)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    AActor* PlacedActor = nullptr;

    // �ٴ� Ÿ�� �޽�
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    UStaticMeshComponent* TileMesh = nullptr;
};

UCLASS()
class DISTRICT_TEST_API APuzzleArea : public AActor
{
    GENERATED_BODY()

public:
    APuzzleArea();

    // ========== �⺻ �׸��� ���� ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridRows = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridColumns = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float CellSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float GridHeight = 0.0f;

    // ========== ����� ǥ�� ���� ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugGrid = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowGridInGame = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bForceDebugDisplay = true;

    // ========== �� ���� ���� ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor WalkableColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor UnwalkableColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor PedestalSlotColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor OccupiedColor = FLinearColor::Blue;

    // ========== �ٴ� Ÿ�� ���� ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Tiles")
    bool bShowTileMeshes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Tiles")
    UStaticMesh* DefaultTileMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Tiles")
    FVector TileScale = FVector(3.0f, 3.0f, 0.4f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Tiles")
    UMaterialInterface* WalkableMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Tiles")
    UMaterialInterface* UnwalkableMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Tiles")
    UMaterialInterface* PedestalSlotMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Tiles")
    UMaterialInterface* OccupiedMaterial;

    // ========== ���� �� ���� ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual Wall Setup",
        meta = (TitleProperty = "Row,Column"))
    TArray<FGridCoordinate> BlockedCells;

    // ========== ��ħ�� ���� ==========BlockedCells
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    TSubclassOf<AActor> PedestalClass;

    // ========== �׸��� ������ ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Data")
    TArray<FGridCell> Grid;


    // �ڵ� ���� Ȱ��ȭ ���� (�⺻��: false)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Blocking")
    bool bEnableAutoBlocking = false;

    // �ڵ� ���� �Լ� (����� �� �Լ�, ���߿� ����)
    UFUNCTION(BlueprintCallable, Category = "Wall Blocking")
    void ApplyAutoBlocking();

    // ���� ���� �Լ� (����)
    UFUNCTION(BlueprintCallable, Category = "Wall Blocking")
    void ApplyManualBlocking();

    // ========== �ٽ� ��� �Լ��� ==========

    // �׸��� �ʱ�ȭ
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void InitializeGrid();

    // �� ���� ����/��ȸ
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetCellState(int32 Row, int32 Column, ECellState NewState);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    ECellState GetCellState(int32 Row, int32 Column) const;

    // ��ǥ ��ȯ
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GetWorldLocationFromGridIndex(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsValidIndex(int32 Row, int32 Column) const;

    // ��ħ�� ����
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* GetActorAtCell(int32 Row, int32 Column) const;

    // �� ���� ����
    UFUNCTION(BlueprintCallable, Category = "Wall Setup")
    void ApplyBlockedCells();

    UFUNCTION(BlueprintCallable, Category = "Wall Setup")
    void ClearBlockedCells();

    // �ð�ȭ ������Ʈ
    UFUNCTION(BlueprintCallable, Category = "Visual")
    void UpdateCellVisuals();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void DrawDebugGrid(float Duration = 0.0f);

    int32 GetIndexFrom2DCoord(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Tile System")
    void CreateTileMeshes();

    UFUNCTION(BlueprintCallable, Category = "Tile System")
    void ClearTileMeshes();
 
protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    // ��Ʈ ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* AreaBox;

    
    void CreateTileAtCell(int32 Row, int32 Column);
    void UpdateTileMaterial(int32 Row, int32 Column);
    UMaterialInterface* GetMaterialForCellState(ECellState State);

private:
    // ���� ���� �Լ���
    
    void DrawCellsInEditor();
};