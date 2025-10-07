#include "Gameplay/PuzzleArea.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Pedestal.h"

APuzzleArea::APuzzleArea()
{
    PrimaryActorTick.bCanEverTick = false;

    // GridRoot를 RootComponent로 설정 - 그리드 시스템의 중심
    GridRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GridRoot"));
    SetRootComponent(GridRoot);

    // AreaBox는 GridRoot에 붙이지만 완전히 독립적
    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    AreaBox->SetupAttachment(GridRoot);
    AreaBox->SetCollisionProfileName(TEXT("BlockAll"));
    AreaBox->SetBoxExtent(AreaBoxExtent);
    AreaBox->SetRelativeLocation(AreaBoxOffset);
    AreaBox->SetMobility(EComponentMobility::Movable);
    AreaBox->bVisualizeComponent = true;

    GridRows = 5;
    GridColumns = 5;
    CellSize = 300.0f;
    GridHeight = 0.0f;

    bShowDebugGrid = false;
    bShowGridInGame = false;
    bShowGridLinesInEditor = false;

    WalkableColor = FLinearColor::Green;
    UnwalkableColor = FLinearColor::Red;
    PedestalSlotColor = FLinearColor::Yellow;
    OccupiedColor = FLinearColor::Blue;

    bShowTileMeshes = true;
    TileScale = FVector(3.0f, 3.0f, 0.4f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        DefaultTileMesh = CubeMeshAsset.Object;
    }
}

void APuzzleArea::SetBoxExtent(FVector NewExtent)
{
    AreaBoxExtent = NewExtent;
    UpdateAreaBoxTransform();
}

FVector APuzzleArea::GetBoxExtent() const
{
    if (AreaBox)
    {
        return AreaBox->GetScaledBoxExtent();
    }
    return AreaBoxExtent;
}

void APuzzleArea::UpdateAreaBoxTransform()
{
    if (AreaBox)
    {
        AreaBox->SetBoxExtent(AreaBoxExtent);
        AreaBox->SetRelativeLocation(AreaBoxOffset);
    }
}

#if WITH_EDITOR
void APuzzleArea::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();

        // AreaBox 관련 프로퍼티 변경 시 AreaBox만 업데이트
        if (PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleArea, AreaBoxExtent) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleArea, AreaBoxOffset))
        {
            UpdateAreaBoxTransform();
        }
        // Grid 관련 프로퍼티 변경 시 Grid만 업데이트
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleArea, GridRows) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleArea, GridColumns) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleArea, CellSize) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(APuzzleArea, GridHeight))
        {
            // Grid만 다시 초기화 (AreaBox는 영향 없음)
            InitializeGrid();
            UpdateCellVisuals();
        }
    }
}
#endif

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

    // BeginPlay에서도 AreaBox 위치 확정
    UpdateAreaBoxTransform();
}

void APuzzleArea::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (GridRows <= 0) GridRows = 5;
    if (GridColumns <= 0) GridColumns = 5;
    if (CellSize <= 0.0f) CellSize = 300.0f;

    // AreaBox 업데이트 (그리드와 독립적)
    UpdateAreaBoxTransform();

    // Grid 초기화
    InitializeGrid();

    if (BlockedCells.Num() > 0)
    {
        ApplyBlockedCells();
    }

    UWorld* World = GetWorld();
    if (World)
    {
        FlushPersistentDebugLines(World);
        UpdateCellVisuals();
    }
}

void APuzzleArea::InitializeGrid()
{
    if (GridRows <= 0 || GridColumns <= 0)
    {
        GridRows = FMath::Max(1, GridRows);
        GridColumns = FMath::Max(1, GridColumns);
    }

    ClearTileMeshes();

    Grid.Empty();
    int32 TotalCells = GridRows * GridColumns;
    Grid.SetNum(TotalCells);

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
    UpdateCellVisuals();
}

void APuzzleArea::ApplyManualBlocking()
{
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index >= 0 && Index < Grid.Num())
            {
                if (Grid[Index].PlacedActor == nullptr)
                {
                    Grid[Index].State = ECellState::Walkable;
                }
            }
        }
    }

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

FVector APuzzleArea::GetWorldLocationFromGridIndex(int32 Row, int32 Column) const
{
    FVector LocalLocation(
        (Row + 0.5f) * CellSize - (GridRows * CellSize * 0.5f),
        (Column + 0.5f) * CellSize - (GridColumns * CellSize * 0.5f),
        GridHeight
    );

    return GetActorTransform().TransformPosition(LocalLocation);
}

bool APuzzleArea::GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const
{
    FVector LocalLocation = GetActorTransform().InverseTransformPosition(WorldLocation);

    LocalLocation.X += (GridRows * CellSize * 0.5f);
    LocalLocation.Y += (GridColumns * CellSize * 0.5f);

    OutRow = FMath::FloorToInt(LocalLocation.X / CellSize);
    OutColumn = FMath::FloorToInt(LocalLocation.Y / CellSize);

    return IsValidIndex(OutRow, OutColumn);
}

bool APuzzleArea::RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column)
{
    if (!Pedestal || !IsValidIndex(Row, Column))
    {
        return false;
    }

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

    if (ExistingActor == PedestalActor)
    {
        if (Grid[Index].State != ECellState::Occupied)
        {
            Grid[Index].State = ECellState::Occupied;
            UpdateCellVisuals();
        }
        return true;
    }

    if (ExistingActor != nullptr)
    {
        Grid[Index].State = ECellState::Unwalkable;
        UpdateCellVisuals();
        return false;
    }

    Grid[Index].PlacedActor = PedestalActor;
    Grid[Index].State = ECellState::Occupied;

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

    if (Grid[Index].TileMesh)
    {
        Grid[Index].TileMesh->DestroyComponent();
    }

    FString TileName = FString::Printf(TEXT("Tile_%d_%d"), Row, Column);
    Grid[Index].TileMesh = NewObject<UStaticMeshComponent>(this, *TileName);

    if (Grid[Index].TileMesh)
    {
        // GridRoot에 붙여서 그리드 시스템과 함께 움직이도록
        Grid[Index].TileMesh->SetupAttachment(GridRoot);
        Grid[Index].TileMesh->RegisterComponent();
        Grid[Index].TileMesh->SetStaticMesh(DefaultTileMesh);

        Grid[Index].TileMesh->SetWorldScale3D(TileScale);
        Grid[Index].TileMesh->SetWorldLocation(Grid[Index].WorldLocation);

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

void APuzzleArea::UpdateCellVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (Grid.Num() == 0)
    {
        InitializeGrid();
        return;
    }

    int32 ExpectedSize = GridRows * GridColumns;
    if (Grid.Num() != ExpectedSize)
    {
        InitializeGrid();
        return;
    }

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

    if (bShowDebugGrid)
    {
        FlushPersistentDebugLines(World);

        if (World->IsEditorWorld())
        {
            if (bShowGridLinesInEditor)
            {
                DrawDebugGrid(-1.0f);
            }
            DrawCellsInEditor();
        }
        else if (bShowGridInGame)
        {
            DrawDebugGrid(-1.0f);
        }
    }
}

void APuzzleArea::DrawDebugGrid(float Duration)
{
    UWorld* World = GetWorld();
    if (!World || GridRows <= 0 || GridColumns <= 0) return;

    FVector ActorLocation = GetActorLocation();
    FVector GridOrigin = ActorLocation;
    GridOrigin.X -= (GridRows * CellSize * 0.5f);
    GridOrigin.Y -= (GridColumns * CellSize * 0.5f);
    GridOrigin.Z += GridHeight + 2.0f;

    float DebugDuration = (Duration <= 0.0f) ? -1.0f : Duration;
    bool bPersistent = (Duration <= 0.0f);

    for (int32 i = 0; i <= GridRows; i++)
    {
        FVector Start = GridOrigin + FVector(i * CellSize, 0.0f, 0.0f);
        FVector End = GridOrigin + FVector(i * CellSize, GridColumns * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, bPersistent, DebugDuration, 200, 2.0f);
    }

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

    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z += GridHeight + 1.0f;

    float Margin = CellSize * 0.1f;

    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index < 0 || Index >= Grid.Num()) continue;

            const FGridCell& Cell = Grid[Index];
            FColor CellColor;

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

            FVector TopLeft = GridOrigin + FVector(Row * CellSize, Column * CellSize, 0.0f);
            FVector InnerTopLeft = TopLeft + FVector(Margin, Margin, 0.0f);
            FVector InnerBottomRight = TopLeft + FVector(CellSize - Margin, CellSize - Margin, 0.0f);

            FVector BoxCenter = (InnerTopLeft + InnerBottomRight) * 0.5f;
            FVector BoxExtent = (InnerBottomRight - InnerTopLeft) * 0.5f;

            DrawDebugBox(World, BoxCenter, BoxExtent, CellColor, true, -1.0f, 150, 1.0f);
        }
    }
}