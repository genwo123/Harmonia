#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PuzzleArea.generated.h"

class APedestal;

UENUM(BlueprintType)
enum class ECellState : uint8
{
    Walkable,
    Unwalkable,
    PedestalSlot,
    Occupied
};

UENUM(BlueprintType)
enum class EGridDirection : uint8
{
    North,
    East,
    South,
    West
};

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

USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    ECellState State = ECellState::Walkable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    AActor* PlacedActor = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    UStaticMeshComponent* TileMesh = nullptr;
};

UCLASS()
class DISTRICT_TEST_API APuzzleArea : public AActor
{
    GENERATED_BODY()

public:
    APuzzleArea();

    // Components - AreaBox는 독립적으로 존재
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* GridRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* AreaBox;

    // Area Box Settings - 퍼즐 그리드와 완전히 독립적
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area Box")
    FVector AreaBoxExtent = FVector(750.0f, 750.0f, 200.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area Box")
    FVector AreaBoxOffset = FVector::ZeroVector;

    // Grid Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridRows = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridColumns = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float CellSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float GridHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugGrid = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowGridInGame = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowGridLinesInEditor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor WalkableColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor UnwalkableColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor PedestalSlotColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor OccupiedColor = FLinearColor::Blue;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual Wall Setup", meta = (TitleProperty = "Row,Column"))
    TArray<FGridCoordinate> BlockedCells;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    TSubclassOf<AActor> PedestalClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Data")
    TArray<FGridCell> Grid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Blocking")
    bool bEnableAutoBlocking = false;

    UFUNCTION(BlueprintCallable, Category = "Wall Blocking")
    void ApplyAutoBlocking();

    UFUNCTION(BlueprintCallable, Category = "Wall Blocking")
    void ApplyManualBlocking();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void InitializeGrid();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetCellState(int32 Row, int32 Column, ECellState NewState);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    ECellState GetCellState(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GetWorldLocationFromGridIndex(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsValidIndex(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* GetActorAtCell(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Wall Setup")
    void ApplyBlockedCells();

    UFUNCTION(BlueprintCallable, Category = "Wall Setup")
    void ClearBlockedCells();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void UpdateCellVisuals();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void DrawDebugGrid(float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Area Box")
    void SetBoxExtent(FVector NewExtent);

    UFUNCTION(BlueprintPure, Category = "Area Box")
    FVector GetBoxExtent() const;

    UFUNCTION(BlueprintCallable, Category = "Area Box")
    void UpdateAreaBoxTransform();

    int32 GetIndexFrom2DCoord(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Tile System")
    void CreateTileMeshes();

    UFUNCTION(BlueprintCallable, Category = "Tile System")
    void ClearTileMeshes();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    void CreateTileAtCell(int32 Row, int32 Column);
    void UpdateTileMaterial(int32 Row, int32 Column);
    UMaterialInterface* GetMaterialForCellState(ECellState State);

private:
    void DrawCellsInEditor();
};