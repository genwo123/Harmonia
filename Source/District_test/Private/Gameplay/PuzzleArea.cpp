// PuzzleArea.cpp - 1부: 생성자 및 기본 함수들
#include "Gameplay/PuzzleArea.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Pedestal.h"

APuzzleArea::APuzzleArea()
{
    PrimaryActorTick.bCanEverTick = false; // Tick 비활성화

    // 기본 그리드 설정
    GridRows = 5;
    GridColumns = 5;
    CellSize = 300.0f;
    GridHeight = 0.0f;

    // 디버그 표시 설정
    bShowDebugGrid = true;
    bShowGridInGame = false;

    // 셀 색상 기본값
    WalkableColor = FLinearColor::Green;
    UnwalkableColor = FLinearColor::Red;
    PedestalSlotColor = FLinearColor::Yellow;
    OccupiedColor = FLinearColor::Blue;

    // 바닥 타일 설정
    bShowTileMeshes = true;
    TileScale = FVector(3.0f, 3.0f, 0.4f);

    // 컴포넌트 생성
    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    SetRootComponent(AreaBox);
    AreaBox->SetCollisionProfileName(TEXT("BlockAll"));
    AreaBox->SetBoxExtent(FVector(GridRows * CellSize * 0.5f, GridColumns * CellSize * 0.5f, 50.0f));

    // 기본 타일 메시 로드
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        DefaultTileMesh = CubeMeshAsset.Object;
    }
}

void APuzzleArea::BeginPlay()
{
    Super::BeginPlay();

    if (Grid.Num() == 0)
    {
        InitializeGrid();
    }

    if (BlockedCells.Num() > 0)
    {
        ApplyBlockedCells();
    }

    if (bShowDebugGrid && GetWorld())
    {
        UpdateCellVisuals();
    }
}

void APuzzleArea::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (!AreaBox)
    {
        return;
    }

    // 그리드 크기 검증
    if (GridRows <= 0) GridRows = 5;
    if (GridColumns <= 0) GridColumns = 5;
    if (CellSize <= 0.0f) CellSize = 300.0f;

    // 그리드 초기화
    InitializeGrid();

    // BlockedCells 적용
    if (BlockedCells.Num() > 0)
    {
        ApplyBlockedCells();
    }

    // 에디터에서 시각화 - 타이머 완전 제거
    UWorld* World = GetWorld();
    if (World)
    {
        FlushPersistentDebugLines(World);
        UpdateCellVisuals();

        // 추가 업데이트 없음 - 크래시 방지
    }
}
void APuzzleArea::InitializeGrid()
{
    // 유효하지 않은 그리드 크기 보정
    if (GridRows <= 0 || GridColumns <= 0)
    {
        GridRows = FMath::Max(1, GridRows);
        GridColumns = FMath::Max(1, GridColumns);
    }

    // 기존 타일 메시들 제거
    ClearTileMeshes();

    // 그리드 초기화
    Grid.Empty();
    int32 TotalCells = GridRows * GridColumns;
    Grid.SetNum(TotalCells);

    // 각 셀 정보 설정
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);

            if (Index >= 0 && Index < Grid.Num())
            {
                FVector CellLocation = GetWorldLocationFromGridIndex(Row, Column);

                Grid[Index].WorldLocation = CellLocation;
                Grid[Index].State = ECellState::Walkable;
                Grid[Index].PlacedActor = nullptr;
                Grid[Index].TileMesh = nullptr;
            }
        }
    }

    // 타일 메시 생성
    if (bShowTileMeshes)
    {
        CreateTileMeshes();
    }
}

void APuzzleArea::SetCellState(int32 Row, int32 Column, ECellState NewState)
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);

        if (Grid[Index].State != NewState)
        {
            Grid[Index].State = NewState;

            if (bShowTileMeshes)
            {
                UpdateTileMaterial(Row, Column);
            }
            else
            {
                UpdateCellVisuals();
            }
        }
    }
}

void APuzzleArea::ApplyAutoBlocking()
{
    // 나중에 구현할 자동 장애물 감지 로직
    // 현재는 빈 함수

    // TODO: 자동으로 장애물 감지해서 벽 설정
    // 예: 콜리전 체크, 스태틱 메시 감지 등

    UpdateCellVisuals();
}


void APuzzleArea::ApplyManualBlocking()
{
    // 먼저 모든 셀을 걸을 수 있는 상태로 초기화 (받침대가 있는 셀 제외)
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index >= 0 && Index < Grid.Num())
            {
                // 받침대가 없는 셀만 초기화
                if (Grid[Index].PlacedActor == nullptr)
                {
                    Grid[Index].State = ECellState::Walkable;
                }
            }
        }
    }

    // BlockedCells 배열의 좌표들을 벽으로 설정
    for (const FGridCoordinate& Coord : BlockedCells)
    {
        if (IsValidIndex(Coord.Row, Coord.Column))
        {
            int32 Index = GetIndexFrom2DCoord(Coord.Row, Coord.Column);
            if (Index >= 0 && Index < Grid.Num())
            {
                Grid[Index].State = ECellState::Unwalkable;
            }
        }
    }

    UpdateCellVisuals();
}


ECellState APuzzleArea::GetCellState(int32 Row, int32 Column) const
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);
        return Grid[Index].State;
    }
    return ECellState::Unwalkable;
}

bool APuzzleArea::IsValidIndex(int32 Row, int32 Column) const
{
    return Row >= 0 && Row < GridRows && Column >= 0 && Column < GridColumns;
}

int32 APuzzleArea::GetIndexFrom2DCoord(int32 Row, int32 Column) const
{
    if (Row < 0 || Row >= GridRows || Column < 0 || Column >= GridColumns)
    {
        return -1;
    }

    int32 Index = Row * GridColumns + Column;

    if (Index < 0 || Index >= Grid.Num())
    {
        return -1;
    }

    return Index;
}

// PuzzleArea.cpp - 3부: 좌표 변환 및 받침대 관리

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

    // 그리드 원점 오프셋 조정
    LocalLocation.X += (GridRows * CellSize * 0.5f);
    LocalLocation.Y += (GridColumns * CellSize * 0.5f);

    // 그리드 좌표 계산
    OutRow = FMath::FloorToInt(LocalLocation.X / CellSize);
    OutColumn = FMath::FloorToInt(LocalLocation.Y / CellSize);

    // 유효한 인덱스인지 확인
    return IsValidIndex(OutRow, OutColumn);
}

bool APuzzleArea::RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column)
{
    if (!Pedestal || !IsValidIndex(Row, Column))
    {
        return false;
    }

    // 그리드가 초기화되지 않은 경우 처리
    if (Grid.Num() == 0)
    {
        InitializeGrid();
    }

    int32 Index = GetIndexFrom2DCoord(Row, Column);
    if (Index < 0 || Index >= Grid.Num())
    {
        return false;
    }

    AActor* ExistingActor = Grid[Index].PlacedActor;
    AActor* PedestalActor = Cast<AActor>(Pedestal);

    // 이미 동일한 받침대가 등록되어 있으면 상태만 확인
    if (ExistingActor == PedestalActor)
    {
        if (Grid[Index].State != ECellState::Occupied)
        {
            Grid[Index].State = ECellState::Occupied;
            UpdateCellVisuals();
        }
        return true;
    }

    // 이미 다른 액터가 점유 중이면 충돌 처리
    if (ExistingActor != nullptr)
    {
        Grid[Index].State = ECellState::Unwalkable;
        UpdateCellVisuals();
        return false;
    }

    // 받침대 등록
    Grid[Index].PlacedActor = PedestalActor;
    Grid[Index].State = ECellState::Occupied;

    // 시각적 업데이트
    UpdateCellVisuals();

    return true;
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



void APuzzleArea::ApplyBlockedCells()
{
    if (bEnableAutoBlocking)
    {
        ApplyAutoBlocking();
    }
    else
    {
        ApplyManualBlocking();
    }
}

void APuzzleArea::ClearBlockedCells()
{
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index >= 0 && Index < Grid.Num())
            {
                if (Grid[Index].State != ECellState::Occupied || Grid[Index].PlacedActor == nullptr)
                {
                    Grid[Index].State = ECellState::Walkable;
                }
            }
        }
    }

    BlockedCells.Empty();
    UpdateCellVisuals();
}

// PuzzleArea.cpp - 5부: 바닥 타일 시스템

void APuzzleArea::CreateTileMeshes()
{
    if (!DefaultTileMesh) return;

    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            CreateTileAtCell(Row, Column);
        }
    }
}

void APuzzleArea::ClearTileMeshes()
{
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        if (Grid[i].TileMesh && IsValid(Grid[i].TileMesh))
        {
            Grid[i].TileMesh->DestroyComponent();
            Grid[i].TileMesh = nullptr;
        }
    }
}

void APuzzleArea::CreateTileAtCell(int32 Row, int32 Column)
{
    if (!IsValidIndex(Row, Column) || !DefaultTileMesh) return;

    int32 Index = GetIndexFrom2DCoord(Row, Column);

    // 이미 타일이 있으면 제거
    if (Grid[Index].TileMesh)
    {
        Grid[Index].TileMesh->DestroyComponent();
    }

    FString TileName = FString::Printf(TEXT("Tile_%d_%d"), Row, Column);
    Grid[Index].TileMesh = NewObject<UStaticMeshComponent>(this, *TileName);

    if (Grid[Index].TileMesh)
    {
        Grid[Index].TileMesh->SetupAttachment(RootComponent);
        Grid[Index].TileMesh->RegisterComponent();
        Grid[Index].TileMesh->SetStaticMesh(DefaultTileMesh);

        // 타일 스케일과 위치 설정
        Grid[Index].TileMesh->SetWorldScale3D(TileScale);
        Grid[Index].TileMesh->SetWorldLocation(Grid[Index].WorldLocation);

        // 콜리전 설정
        Grid[Index].TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Grid[Index].TileMesh->SetCollisionResponseToAllChannels(ECR_Block);
    }
}

void APuzzleArea::UpdateTileMaterial(int32 Row, int32 Column)
{
    if (!IsValidIndex(Row, Column)) return;

    int32 Index = GetIndexFrom2DCoord(Row, Column);
    if (!Grid[Index].TileMesh) return;

    UMaterialInterface* Material = GetMaterialForCellState(Grid[Index].State);
    if (Material)
    {
        Grid[Index].TileMesh->SetMaterial(0, Material);
    }
}

UMaterialInterface* APuzzleArea::GetMaterialForCellState(ECellState State)
{
    switch (State)
    {
    case ECellState::Walkable:
        return WalkableMaterial;
    case ECellState::Unwalkable:
        return UnwalkableMaterial;
    case ECellState::PedestalSlot:
        return PedestalSlotMaterial;
    case ECellState::Occupied:
        return OccupiedMaterial;
    default:
        return WalkableMaterial;
    }
}

// PuzzleArea.cpp - 6부: 시각화 및 디버그

void APuzzleArea::UpdateCellVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 그리드가 초기화되지 않은 경우 초기화
    if (Grid.Num() == 0)
    {
        InitializeGrid();
        return;
    }

    // 예상 그리드 크기와 실제 배열 크기 비교
    int32 ExpectedSize = GridRows * GridColumns;
    if (Grid.Num() != ExpectedSize)
    {
        InitializeGrid();
        return;
    }

    // 타일 머티리얼 업데이트
    if (bShowTileMeshes)
    {
        for (int32 Row = 0; Row < GridRows; Row++)
        {
            for (int32 Column = 0; Column < GridColumns; Column++)
            {
                UpdateTileMaterial(Row, Column);
            }
        }
    }

    // 디버그 표시
    if (bShowDebugGrid)
    {
        // 이전 디버그 라인 지우기
        FlushPersistentDebugLines(World);

        // 그리드 라인 그리기
        if (World->IsEditorWorld() || bShowGridInGame)
        {
            DrawDebugGrid(-1.0f);
        }

        // 에디터에서 셀 색상 표시
        if (World->IsEditorWorld())
        {
            DrawCellsInEditor();
        }
    }
}

void APuzzleArea::DrawDebugGrid(float Duration)
{
    UWorld* World = GetWorld();
    if (!World || GridRows <= 0 || GridColumns <= 0) return;

    // 그리드 원점 계산
    FVector ActorLocation = GetActorLocation();
    FVector GridOrigin = ActorLocation;
    GridOrigin.X -= (GridRows * CellSize * 0.5f);
    GridOrigin.Y -= (GridColumns * CellSize * 0.5f);
    GridOrigin.Z += GridHeight + 2.0f;

    // 지속 시간 설정
    float DebugDuration = (Duration <= 0.0f) ? -1.0f : Duration;
    bool bPersistent = (Duration <= 0.0f);

    // 세로 선들
    for (int32 i = 0; i <= GridRows; i++)
    {
        FVector Start = GridOrigin + FVector(i * CellSize, 0.0f, 0.0f);
        FVector End = GridOrigin + FVector(i * CellSize, GridColumns * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, bPersistent, DebugDuration, 200, 2.0f);
    }

    // 가로 선들
    for (int32 j = 0; j <= GridColumns; j++)
    {
        FVector Start = GridOrigin + FVector(0.0f, j * CellSize, 0.0f);
        FVector End = GridOrigin + FVector(GridRows * CellSize, j * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, bPersistent, DebugDuration, 200, 2.0f);
    }
}

void APuzzleArea::DrawCellsInEditor()
{
    UWorld* World = GetWorld();
    if (!World || Grid.Num() == 0) return;

    // 그리드 원점 계산
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z += GridHeight + 1.0f;

    float Margin = CellSize * 0.1f;

    // 각 셀 색상 그리기
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index < 0 || Index >= Grid.Num()) continue;

            const FGridCell& Cell = Grid[Index];
            FColor CellColor;

            // 셀 상태에 따라 색상 결정
            switch (Cell.State)
            {
            case ECellState::Walkable:
                CellColor = WalkableColor.ToFColor(true);
                break;
            case ECellState::Unwalkable:
                CellColor = UnwalkableColor.ToFColor(true);
                break;
            case ECellState::PedestalSlot:
                CellColor = PedestalSlotColor.ToFColor(true);
                break;
            case ECellState::Occupied:
                CellColor = OccupiedColor.ToFColor(true);
                break;
            default:
                CellColor = FColor::White;
                break;
            }

            // 셀의 꼭지점 계산
            FVector TopLeft = GridOrigin + FVector(Row * CellSize, Column * CellSize, 0.0f);
            FVector InnerTopLeft = TopLeft + FVector(Margin, Margin, 0.0f);
            FVector InnerBottomRight = TopLeft + FVector(CellSize - Margin, CellSize - Margin, 0.0f);

            // 박스 중심과 크기 계산
            FVector BoxCenter = (InnerTopLeft + InnerBottomRight) * 0.5f;
            FVector BoxExtent = (InnerBottomRight - InnerTopLeft) * 0.5f;

            // 셀 색상 박스 그리기
            DrawDebugBox(World, BoxCenter, BoxExtent, CellColor, true, -1.0f, 150, 1.0f);
        }
    }
}