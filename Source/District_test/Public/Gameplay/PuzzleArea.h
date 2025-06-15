// PuzzleArea.h - 깔끔하게 정리된 버전
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PuzzleArea.generated.h"

class APedestal;

// 셀 상태 정의
UENUM(BlueprintType)
enum class ECellState : uint8
{
    Walkable,       // 초록색 - 이동 가능
    Unwalkable,     // 빨간색 - 이동 불가능/벽
    PedestalSlot,   // 노란색 - 받침대 배치 가능
    Occupied        // 파란색 - 받침대가 설치됨
};

// 방향 정의
UENUM(BlueprintType)
enum class EGridDirection : uint8
{
    North,
    East,
    South,
    West
};

// 그리드 좌표 구조체
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

// 그리드 셀 구조체
USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()

    // 셀 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    ECellState State = ECellState::Walkable;

    // 월드 위치
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    FVector WorldLocation = FVector::ZeroVector;

    // 셀에 배치된 액터 (받침대)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    AActor* PlacedActor = nullptr;

    // 바닥 타일 메시
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    UStaticMeshComponent* TileMesh = nullptr;
};

UCLASS()
class DISTRICT_TEST_API APuzzleArea : public AActor
{
    GENERATED_BODY()

public:
    APuzzleArea();

    // ========== 기본 그리드 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridRows = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridColumns = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float CellSize = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float GridHeight = 0.0f;

    // ========== 디버그 표시 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugGrid = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowGridInGame = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bForceDebugDisplay = true;

    // ========== 셀 색상 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor WalkableColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor UnwalkableColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor PedestalSlotColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell Colors")
    FLinearColor OccupiedColor = FLinearColor::Blue;

    // ========== 바닥 타일 설정 ==========
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

    // ========== 수동 벽 설정 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manual Wall Setup",
        meta = (TitleProperty = "Row,Column"))
    TArray<FGridCoordinate> BlockedCells;

    // ========== 받침대 설정 ==========BlockedCells
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    TSubclassOf<AActor> PedestalClass;

    // ========== 그리드 데이터 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Data")
    TArray<FGridCell> Grid;


    // 자동 차단 활성화 여부 (기본값: false)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Blocking")
    bool bEnableAutoBlocking = false;

    // 자동 차단 함수 (현재는 빈 함수, 나중에 구현)
    UFUNCTION(BlueprintCallable, Category = "Wall Blocking")
    void ApplyAutoBlocking();

    // 수동 차단 함수 (기존)
    UFUNCTION(BlueprintCallable, Category = "Wall Blocking")
    void ApplyManualBlocking();

    // ========== 핵심 기능 함수들 ==========

    // 그리드 초기화
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void InitializeGrid();

    // 셀 상태 설정/조회
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetCellState(int32 Row, int32 Column, ECellState NewState);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    ECellState GetCellState(int32 Row, int32 Column) const;

    // 좌표 변환
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GetWorldLocationFromGridIndex(int32 Row, int32 Column) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool IsValidIndex(int32 Row, int32 Column) const;

    // 받침대 관리
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* GetActorAtCell(int32 Row, int32 Column) const;

    // 벽 설정 관리
    UFUNCTION(BlueprintCallable, Category = "Wall Setup")
    void ApplyBlockedCells();

    UFUNCTION(BlueprintCallable, Category = "Wall Setup")
    void ClearBlockedCells();

    // 시각화 업데이트
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

    // 루트 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* AreaBox;

    
    void CreateTileAtCell(int32 Row, int32 Column);
    void UpdateTileMaterial(int32 Row, int32 Column);
    UMaterialInterface* GetMaterialForCellState(ECellState State);

private:
    // 내부 헬퍼 함수들
    
    void DrawCellsInEditor();
};