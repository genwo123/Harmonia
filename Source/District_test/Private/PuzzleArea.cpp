// PuzzleArea.cpp
#include "PuzzleArea.h"
#include "DrawDebugHelpers.h"

APuzzleArea::APuzzleArea()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본값 설정
    GridRows = 10;
    GridColumns = 10;
    CellSize = 400.0f; // 4미터
    GridHeight = 0.0f;
    bShowDebugGrid = true;

    // 색상 기본값
    WalkableColor = FLinearColor::Green;
    UnwalkableColor = FLinearColor::Red;
    PedestalSlotColor = FLinearColor::Yellow;

    // 컴포넌트 생성 및 설정
    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    SetRootComponent(AreaBox);
    AreaBox->SetCollisionProfileName(TEXT("BlockAll"));
    AreaBox->SetBoxExtent(FVector(GridRows * CellSize * 0.5f, GridColumns * CellSize * 0.5f, 50.0f));

    // 인스턴스드 스태틱 메시 컴포넌트 생성
    WalkableTiles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WalkableTiles"));
    WalkableTiles->SetupAttachment(AreaBox);

    UnwalkableTiles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("UnwalkableTiles"));
    UnwalkableTiles->SetupAttachment(AreaBox);

    PedestalSlotTiles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PedestalSlotTiles"));
    PedestalSlotTiles->SetupAttachment(AreaBox);
}

void APuzzleArea::BeginPlay()
{
    Super::BeginPlay();

    if (Grid.Num() == 0)
    {
        InitializeGrid();
    }
}

void APuzzleArea::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UpdateGridSizeFromActorScale();
}

void APuzzleArea::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 게임 중에도 항상 그리드 표시
    if (bShowDebugGrid && GetWorld())
    {
        // 이전에 그려진 디버그 라인 모두 지우기
        FlushPersistentDebugLines(GetWorld());

        // 새로운 그리드 라인 그리기
        DrawDebugGrid(-1.0f); // -1.0f로 설정하여 영구 표시
    }
}

void APuzzleArea::InitializeGrid()
{
    // 기존 그리드 초기화
    Grid.Empty();
    Grid.SetNum(GridRows * GridColumns);

    // 각 셀 정보 설정 - 간격 없이 배치
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);

            if (Index >= 0 && Index < Grid.Num())
            {
                // 월드 위치 계산 - 간격을 0으로 하여 셀이 서로 붙도록 함
                FVector CellLocation = GetWorldLocationFromGridIndex(Row, Column);

                // 셀 정보 설정
                Grid[Index].WorldLocation = CellLocation;
                Grid[Index].State = ECellState::Walkable; // 기본 이동 가능 상태
                Grid[Index].PlacedActor = nullptr;
            }
        }
    }
}

void APuzzleArea::UpdateTileMeshes()
{
    // 기존 인스턴스 초기화
    WalkableTiles->ClearInstances();
    UnwalkableTiles->ClearInstances();
    PedestalSlotTiles->ClearInstances();

    // 각 타일 메시 설정
    WalkableTiles->SetStaticMesh(WalkableTileMesh);
    UnwalkableTiles->SetStaticMesh(UnwalkableTileMesh);
    PedestalSlotTiles->SetStaticMesh(PedestalSlotTileMesh);

    // 타일 메시 크기 조정 (메시의 기본 크기에 따라 스케일을 조정해야 함)
    // 예: 기본 메시가 100x100인 경우 4.0 스케일을 적용
    float TileScale = CellSize / 100.0f; // 메시의 기본 크기에 따라 조정 필요

    // 셀 별로 적절한 메시 인스턴스 추가
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        const FGridCell& Cell = Grid[i];

        // 셀 중앙에 정확히 배치하고 스케일 적용
        FTransform TileTransform(
            FQuat::Identity,
            Cell.WorldLocation,
            FVector(TileScale, TileScale, 1.0f) // 높이는 그대로 유지하거나 필요에 따라 조정
        );

        switch (Cell.State)
        {
        case ECellState::Walkable:
            if (WalkableTileMesh)
            {
                WalkableTiles->AddInstance(TileTransform);
            }
            break;
        case ECellState::Unwalkable:
            if (UnwalkableTileMesh)
            {
                UnwalkableTiles->AddInstance(TileTransform);
            }
            break;
        case ECellState::PedestalSlot:
            if (PedestalSlotTileMesh)
            {
                PedestalSlotTiles->AddInstance(TileTransform);
            }
            break;
        }
    }
}

void APuzzleArea::SetCellState(int32 Row, int32 Column, ECellState NewState)
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);
        Grid[Index].State = NewState;

        // 인스턴스드 메시 업데이트
        UpdateTileMeshes();
    }
}

ECellState APuzzleArea::GetCellState(int32 Row, int32 Column) const
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);
        return Grid[Index].State;
    }
    return ECellState::Unwalkable; // 범위 벗어나면 이동 불가 반환
}

ECellState APuzzleArea::GetCellStateFromWorldLocation(const FVector& WorldLocation) const
{
    int32 Row, Column;
    if (GetGridIndexFromWorldLocation(WorldLocation, Row, Column))
    {
        return GetCellState(Row, Column);
    }
    return ECellState::Unwalkable;
}

FVector APuzzleArea::GetWorldLocationFromGridIndex(int32 Row, int32 Column) const
{
    // 그리드 원점은 액터 중앙으로부터 -GridSize/2 위치에 있음
    FVector LocalLocation(
        (Row + 0.5f) * CellSize - (GridRows * CellSize * 0.5f),
        (Column + 0.5f) * CellSize - (GridColumns * CellSize * 0.5f),
        GridHeight
    );

    // 월드 좌표로 변환
    return GetActorTransform().TransformPosition(LocalLocation);
}

bool APuzzleArea::GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const
{
    // 액터 기준 로컬 좌표로 변환
    FVector LocalLocation = GetActorTransform().InverseTransformPosition(WorldLocation);

    // 그리드 원점 오프셋 조정 (그리드는 중앙이 원점이므로)
    LocalLocation.X += (GridRows * CellSize * 0.5f);
    LocalLocation.Y += (GridColumns * CellSize * 0.5f);

    // 그리드 좌표 계산
    OutRow = FMath::FloorToInt(LocalLocation.X / CellSize);
    OutColumn = FMath::FloorToInt(LocalLocation.Y / CellSize);

    // 유효한 인덱스인지 확인
    return IsValidIndex(OutRow, OutColumn);
}

bool APuzzleArea::IsValidIndex(int32 Row, int32 Column) const
{
    return Row >= 0 && Row < GridRows && Column >= 0 && Column < GridColumns;
}

FVector APuzzleArea::SnapToGrid(const FVector& WorldLocation) const
{
    int32 Row, Column;
    if (GetGridIndexFromWorldLocation(WorldLocation, Row, Column))
    {
        return GetWorldLocationFromGridIndex(Row, Column);
    }
    return WorldLocation;
}

bool APuzzleArea::CanPlacePedestalAt(int32 Row, int32 Column) const
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);
        return Grid[Index].State == ECellState::PedestalSlot && Grid[Index].PlacedActor == nullptr;
    }
    return false;
}

AActor* APuzzleArea::PlacePedestal(int32 Row, int32 Column)
{
    if (!CanPlacePedestalAt(Row, Column) || !PedestalClass)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // 받침대 스폰 위치
    FVector Location = GetWorldLocationFromGridIndex(Row, Column);
    FRotator Rotation = FRotator::ZeroRotator;

    // 받침대 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = this;

    AActor* Pedestal = World->SpawnActor<AActor>(PedestalClass, Location, Rotation, SpawnParams);

    if (Pedestal)
    {
        // 스폰된 받침대를 그리드에 등록
        int32 Index = GetIndexFrom2DCoord(Row, Column);
        Grid[Index].PlacedActor = Pedestal;
    }

    return Pedestal;
}

void APuzzleArea::PlaceAllPedestals()
{
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            if (CanPlacePedestalAt(Row, Column))
            {
                PlacePedestal(Row, Column);
            }
        }
    }
}

AActor* APuzzleArea::GetActorAtCell(int32 Row, int32 Column) const
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);
        return Grid[Index].PlacedActor;
    }
    return nullptr;
}

bool APuzzleArea::PlaceActorAtCell(AActor* Actor, int32 Row, int32 Column)
{
    if (!IsValid(Actor) || !IsValidIndex(Row, Column))
    {
        return false;
    }

    int32 Index = GetIndexFrom2DCoord(Row, Column);

    // 셀이 비어있어야만 배치 가능
    if (Grid[Index].PlacedActor != nullptr && Grid[Index].PlacedActor != Actor)
    {
        return false;
    }

    // 액터 위치 설정 - 정확히 셀 중앙에 배치
    FVector Location = Grid[Index].WorldLocation;
    Actor->SetActorLocation(Location);

    // 그리드에 액터 등록
    Grid[Index].PlacedActor = Actor;

    return true;
}

bool APuzzleArea::PlaceActorAtWorldLocation(AActor* Actor, const FVector& WorldLocation)
{
    int32 Row, Column;
    if (GetGridIndexFromWorldLocation(WorldLocation, Row, Column))
    {
        return PlaceActorAtCell(Actor, Row, Column);
    }
    return false;
}

bool APuzzleArea::GetAdjacentCell(int32 Row, int32 Column, EGridDirection Direction, int32& OutRow, int32& OutColumn) const
{
    OutRow = Row;
    OutColumn = Column;

    switch (Direction)
    {
    case EGridDirection::North:
        OutRow--;
        break;
    case EGridDirection::East:
        OutColumn++;
        break;
    case EGridDirection::South:
        OutRow++;
        break;
    case EGridDirection::West:
        OutColumn--;
        break;
    }

    return IsValidIndex(OutRow, OutColumn);
}

void APuzzleArea::DrawDebugGrid(float Duration)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 각 셀의 경계를 표시하는 라인 그리기
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z = GetActorLocation().Z + GridHeight + 1.0f;

    // 그리드 라인 그리기 - 여기서 매개변수 수정
    for (int32 i = 0; i <= GridRows; i++)
    {
        FVector Start = GridOrigin + FVector(i * CellSize, 0.0f, 0.0f);
        FVector End = GridOrigin + FVector(i * CellSize, GridColumns * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, true, Duration, 0, 2.0f);
        
    }

    for (int32 j = 0; j <= GridColumns; j++)
    {
        FVector Start = GridOrigin + FVector(0.0f, j * CellSize, 0.0f);
        FVector End = GridOrigin + FVector(GridRows * CellSize, j * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, true, Duration, 0, 2.0f);
 
    }
}
void APuzzleArea::ToggleDebugMode()
{
    bShowDebugGrid = !bShowDebugGrid;
}

FVector APuzzleArea::GetGridSize() const
{
    return FVector(GridRows * CellSize, GridColumns * CellSize, 0.0f);
}

TArray<uint8> APuzzleArea::SaveCellStates() const
{
    TArray<uint8> SaveData;
    SaveData.SetNum(Grid.Num());

    for (int32 i = 0; i < Grid.Num(); i++)
    {
        SaveData[i] = static_cast<uint8>(Grid[i].State);
    }

    return SaveData;
}

void APuzzleArea::LoadCellStates(const TArray<uint8>& CellStates)
{
    if (CellStates.Num() == GridRows * GridColumns)
    {
        for (int32 i = 0; i < CellStates.Num(); i++)
        {
            Grid[i].State = static_cast<ECellState>(CellStates[i]);
        }

        UpdateTileMeshes();
    }
}

void APuzzleArea::ResetPuzzle()
{
    // 모든 셀에 배치된 액터 제거 (받침대 제외)
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        if (Grid[i].PlacedActor)
        {
            // 받침대인지 확인 로직 필요 (여기서는 간단하게 처리)
            // 실제 게임에서는 받침대와 퍼즐 요소를 구분하는 방법이 필요함
            Grid[i].PlacedActor = nullptr;
        }
    }
}

int32 APuzzleArea::GetIndexFrom2DCoord(int32 Row, int32 Column) const
{
    if (IsValidIndex(Row, Column))
    {
        return Row * GridColumns + Column;
    }
    return -1;
}

void APuzzleArea::Get2DCoordFromIndex(int32 Index, int32& OutRow, int32& OutColumn) const
{
    if (Index >= 0 && Index < Grid.Num())
    {
        OutRow = Index / GridColumns;
        OutColumn = Index % GridColumns;
    }
}

void APuzzleArea::UpdateGridSizeFromActorScale()
{
    // 액터의 현재 스케일 가져오기
    FVector ActorScale = GetActorScale3D();

    // 박스 컴포넌트의 기본 크기 가져오기
    FVector BoxExtent = AreaBox->GetUnscaledBoxExtent();

    // 실제 월드 크기 계산 (기본 박스 크기 * 스케일)
    float WorldWidth = BoxExtent.X * 2.0f * ActorScale.X;
    float WorldHeight = BoxExtent.Y * 2.0f * ActorScale.Y;

    // 셀 크기는 유지하면서 행과 열 수 계산
    GridRows = FMath::Max(1, FMath::FloorToInt(WorldWidth / CellSize));
    GridColumns = FMath::Max(1, FMath::FloorToInt(WorldHeight / CellSize));

    // 그리드 재초기화
    InitializeGrid();
    UpdateTileMeshes();

    // 디버그 그리드 업데이트
    if (bShowDebugGrid && GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        DrawDebugGrid(0.0f);
    }
}

void APuzzleArea::SetGridSize(float Width, float Height)
{
    // 셀 크기는 유지하면서 행과 열 수 계산
    GridRows = FMath::Max(1, FMath::FloorToInt(Width / CellSize));
    GridColumns = FMath::Max(1, FMath::FloorToInt(Height / CellSize));

    // 박스 크기 조정
    FVector NewExtent(Width * 0.5f, Height * 0.5f, AreaBox->GetUnscaledBoxExtent().Z);
    AreaBox->SetBoxExtent(NewExtent);

    // 그리드 재초기화
    InitializeGrid();
    UpdateTileMeshes();

    // 디버그 그리드 업데이트
    if (bShowDebugGrid && GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        DrawDebugGrid(0.0f);
    }
}