// PuzzleArea.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "PuzzleArea.generated.h"

class APedestal; // 전방 선언 추가

// 셀 상태 정의
UENUM(BlueprintType)
enum class ECellState : uint8
{
    Walkable,       // 초록색 - 이동 가능
    Unwalkable,     // 빨간색 - 이동 불가능
    PedestalSlot,   // 노란색 - 받침대 배치 가능
    Occupied        // 파란색 - 이미 받침대가 설치됨
};

// 방향 정의 (레이저/거울 방향 등에 사용)
UENUM(BlueprintType)
enum class EGridDirection : uint8
{
    North,
    East,
    South,
    West
};

// 그리드 셀 구조체
USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()

    // 셀 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cell")
    ECellState State;

    // 월드 위치
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    FVector WorldLocation;

    // 셀에 배치된 액터 (받침대 또는 퍼즐 요소)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cell")
    AActor* PlacedActor;

    // 초기화
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

    // 루트 컴포넌트로 사용할 박스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* AreaBox;

public:
    // 기본 속성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridRows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    int32 GridColumns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float CellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float GridHeight;

    // 그리드 디버그 표시 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugGrid;

    // 게임에서 그리드 라인 표시 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowGridInGame;

    // 색상 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor WalkableColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor UnwalkableColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor PedestalSlotColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor OccupiedColor;

    // 받침대 클래스 (생성할 때 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    TSubclassOf<AActor> PedestalClass;

    // 그리드 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Data")
    TArray<FGridCell> Grid;

    // 그리드 초기화 함수
    UFUNCTION(BlueprintCallable, Category = "Grid Setup")
    void InitializeGrid();

    // 셀 시각화 함수 (UpdateTileMeshes 대체)
    UFUNCTION(BlueprintCallable, Category = "Grid Setup")
    void UpdateCellVisuals();

    // 셀 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Grid Setup")
    void SetCellState(int32 Row, int32 Column, ECellState NewState);

    // 셀 상태 가져오기
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    ECellState GetCellState(int32 Row, int32 Column) const;

    // 월드 위치에서 셀 상태 가져오기
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    ECellState GetCellStateFromWorldLocation(const FVector& WorldLocation) const;

    // 월드 위치에서 그리드 인덱스 가져오기
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const;

    // 그리드 인덱스에서 월드 위치 가져오기
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    FVector GetWorldLocationFromGridIndex(int32 Row, int32 Column) const;

    // 유효한 인덱스인지 확인
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool IsValidIndex(int32 Row, int32 Column) const;

    // 격자에 위치 스냅(맞추기)
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    FVector SnapToGrid(const FVector& WorldLocation) const;

    // 받침대 배치 가능한지 확인
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool CanPlacePedestalAt(int32 Row, int32 Column) const;

    // 받침대 배치
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* PlacePedestal(int32 Row, int32 Column);

    // 모든 받침대 자동 배치 (노란색 셀에)
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void PlaceAllPedestals();

    // 특정 셀에 배치된 액터 가져오기
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    AActor* GetActorAtCell(int32 Row, int32 Column) const;

    // 특정 셀에 액터 배치
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool PlaceActorAtCell(AActor* Actor, int32 Row, int32 Column);

    // 월드 위치에 액터 배치 (가장 가까운 셀로 스냅)
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool PlaceActorAtWorldLocation(AActor* Actor, const FVector& WorldLocation);

    // 인접한 셀 가져오기
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    bool GetAdjacentCell(int32 Row, int32 Column, EGridDirection Direction, int32& OutRow, int32& OutColumn) const;

    // 디버그용 격자 그리기
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugGrid(float Duration = 0.0f);

    // 디버그 모드 토글
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ToggleDebugMode();

    // 그리드 영역 사이즈 가져오기
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    FVector GetGridSize() const;

    // 현재 그리드 최대 크기 얻기 (블루프린트에서 사용)
    UFUNCTION(BlueprintCallable, Category = "Grid Utility")
    void GetGridDimensions(int32& OutRows, int32& OutColumns) const;

    // 모든 셀 상태 저장 (세이브용)
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    TArray<uint8> SaveCellStates() const;

    // 모든 셀 상태 로드
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    void LoadCellStates(const TArray<uint8>& CellStates);

    // 퍼즐 리셋 (모든 배치된 액터 제거)
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void ResetPuzzle();

    // 그리드 상태 리셋 함수 (디버깅용)
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ResetGridState();

    // 그리드 크기 업데이트 (액터 스케일에 따라)
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void UpdateGridSizeFromActorScale();

    // 특정 크기로 그리드 조정
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetGridSize(float Width, float Height);

    // 받침대 등록 함수
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column);

    // 블루프린트에서 특정 그리드 좌표에 받침대 생성
    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    APedestal* CreatePedestalAtGridPosition(TSubclassOf<APedestal> InPedestalClass, int32 Row, int32 Column);

    UFUNCTION(BlueprintCallable, Category = "Grid Setup")
    void RefreshCellVisuals();

    // 게임에서 셀 색상 표시 (선택적)
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawCellsInGame(bool bEnable);

    int32 GetIndexFrom2DCoord(int32 Row, int32 Column) const;
    void Get2DCoordFromIndex(int32 Index, int32& OutRow, int32& OutColumn) const;

private:
    // 그리드 인덱스 <-> 1D 배열 인덱스 변환 헬퍼 함수

    // 에디터에서만 셀 색상 표시
    void DrawCellsInEditor();

    bool IsDebugLinePresent(UWorld* World);
};