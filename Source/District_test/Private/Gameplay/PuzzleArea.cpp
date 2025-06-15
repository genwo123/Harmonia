// PuzzleArea.cpp - 1��: ������ �� �⺻ �Լ���
#include "Gameplay/PuzzleArea.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Pedestal.h"

APuzzleArea::APuzzleArea()
{
    PrimaryActorTick.bCanEverTick = false; // Tick ��Ȱ��ȭ

    // �⺻ �׸��� ����
    GridRows = 5;
    GridColumns = 5;
    CellSize = 300.0f;
    GridHeight = 0.0f;

    // ����� ǥ�� ����
    bShowDebugGrid = true;
    bShowGridInGame = false;

    // �� ���� �⺻��
    WalkableColor = FLinearColor::Green;
    UnwalkableColor = FLinearColor::Red;
    PedestalSlotColor = FLinearColor::Yellow;
    OccupiedColor = FLinearColor::Blue;

    // �ٴ� Ÿ�� ����
    bShowTileMeshes = true;
    TileScale = FVector(3.0f, 3.0f, 0.4f);

    // ������Ʈ ����
    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    SetRootComponent(AreaBox);
    AreaBox->SetCollisionProfileName(TEXT("BlockAll"));
    AreaBox->SetBoxExtent(FVector(GridRows * CellSize * 0.5f, GridColumns * CellSize * 0.5f, 50.0f));

    // �⺻ Ÿ�� �޽� �ε�
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

    // �׸��� ũ�� ����
    if (GridRows <= 0) GridRows = 5;
    if (GridColumns <= 0) GridColumns = 5;
    if (CellSize <= 0.0f) CellSize = 300.0f;

    // �׸��� �ʱ�ȭ
    InitializeGrid();

    // BlockedCells ����
    if (BlockedCells.Num() > 0)
    {
        ApplyBlockedCells();
    }

    // �����Ϳ��� �ð�ȭ - Ÿ�̸� ���� ����
    UWorld* World = GetWorld();
    if (World)
    {
        FlushPersistentDebugLines(World);
        UpdateCellVisuals();

        // �߰� ������Ʈ ���� - ũ���� ����
    }
}
void APuzzleArea::InitializeGrid()
{
    // ��ȿ���� ���� �׸��� ũ�� ����
    if (GridRows <= 0 || GridColumns <= 0)
    {
        GridRows = FMath::Max(1, GridRows);
        GridColumns = FMath::Max(1, GridColumns);
    }

    // ���� Ÿ�� �޽õ� ����
    ClearTileMeshes();

    // �׸��� �ʱ�ȭ
    Grid.Empty();
    int32 TotalCells = GridRows * GridColumns;
    Grid.SetNum(TotalCells);

    // �� �� ���� ����
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

    // Ÿ�� �޽� ����
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
    // ���߿� ������ �ڵ� ��ֹ� ���� ����
    // ����� �� �Լ�

    // TODO: �ڵ����� ��ֹ� �����ؼ� �� ����
    // ��: �ݸ��� üũ, ����ƽ �޽� ���� ��

    UpdateCellVisuals();
}


void APuzzleArea::ApplyManualBlocking()
{
    // ���� ��� ���� ���� �� �ִ� ���·� �ʱ�ȭ (��ħ�밡 �ִ� �� ����)
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index >= 0 && Index < Grid.Num())
            {
                // ��ħ�밡 ���� ���� �ʱ�ȭ
                if (Grid[Index].PlacedActor == nullptr)
                {
                    Grid[Index].State = ECellState::Walkable;
                }
            }
        }
    }

    // BlockedCells �迭�� ��ǥ���� ������ ����
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

// PuzzleArea.cpp - 3��: ��ǥ ��ȯ �� ��ħ�� ����

FVector APuzzleArea::GetWorldLocationFromGridIndex(int32 Row, int32 Column) const
{
    // �׸��� ������ ���� �߾����κ��� -GridSize/2 ��ġ�� ����
    FVector LocalLocation(
        (Row + 0.5f) * CellSize - (GridRows * CellSize * 0.5f),
        (Column + 0.5f) * CellSize - (GridColumns * CellSize * 0.5f),
        GridHeight
    );

    // ���� ��ǥ�� ��ȯ
    return GetActorTransform().TransformPosition(LocalLocation);
}

bool APuzzleArea::GetGridIndexFromWorldLocation(const FVector& WorldLocation, int32& OutRow, int32& OutColumn) const
{
    // ���� ���� ���� ��ǥ�� ��ȯ
    FVector LocalLocation = GetActorTransform().InverseTransformPosition(WorldLocation);

    // �׸��� ���� ������ ����
    LocalLocation.X += (GridRows * CellSize * 0.5f);
    LocalLocation.Y += (GridColumns * CellSize * 0.5f);

    // �׸��� ��ǥ ���
    OutRow = FMath::FloorToInt(LocalLocation.X / CellSize);
    OutColumn = FMath::FloorToInt(LocalLocation.Y / CellSize);

    // ��ȿ�� �ε������� Ȯ��
    return IsValidIndex(OutRow, OutColumn);
}

bool APuzzleArea::RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column)
{
    if (!Pedestal || !IsValidIndex(Row, Column))
    {
        return false;
    }

    // �׸��尡 �ʱ�ȭ���� ���� ��� ó��
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

    // �̹� ������ ��ħ�밡 ��ϵǾ� ������ ���¸� Ȯ��
    if (ExistingActor == PedestalActor)
    {
        if (Grid[Index].State != ECellState::Occupied)
        {
            Grid[Index].State = ECellState::Occupied;
            UpdateCellVisuals();
        }
        return true;
    }

    // �̹� �ٸ� ���Ͱ� ���� ���̸� �浹 ó��
    if (ExistingActor != nullptr)
    {
        Grid[Index].State = ECellState::Unwalkable;
        UpdateCellVisuals();
        return false;
    }

    // ��ħ�� ���
    Grid[Index].PlacedActor = PedestalActor;
    Grid[Index].State = ECellState::Occupied;

    // �ð��� ������Ʈ
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

// PuzzleArea.cpp - 5��: �ٴ� Ÿ�� �ý���

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

    // �̹� Ÿ���� ������ ����
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

        // Ÿ�� �����ϰ� ��ġ ����
        Grid[Index].TileMesh->SetWorldScale3D(TileScale);
        Grid[Index].TileMesh->SetWorldLocation(Grid[Index].WorldLocation);

        // �ݸ��� ����
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

// PuzzleArea.cpp - 6��: �ð�ȭ �� �����

void APuzzleArea::UpdateCellVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // �׸��尡 �ʱ�ȭ���� ���� ��� �ʱ�ȭ
    if (Grid.Num() == 0)
    {
        InitializeGrid();
        return;
    }

    // ���� �׸��� ũ��� ���� �迭 ũ�� ��
    int32 ExpectedSize = GridRows * GridColumns;
    if (Grid.Num() != ExpectedSize)
    {
        InitializeGrid();
        return;
    }

    // Ÿ�� ��Ƽ���� ������Ʈ
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

    // ����� ǥ��
    if (bShowDebugGrid)
    {
        // ���� ����� ���� �����
        FlushPersistentDebugLines(World);

        // �׸��� ���� �׸���
        if (World->IsEditorWorld() || bShowGridInGame)
        {
            DrawDebugGrid(-1.0f);
        }

        // �����Ϳ��� �� ���� ǥ��
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

    // �׸��� ���� ���
    FVector ActorLocation = GetActorLocation();
    FVector GridOrigin = ActorLocation;
    GridOrigin.X -= (GridRows * CellSize * 0.5f);
    GridOrigin.Y -= (GridColumns * CellSize * 0.5f);
    GridOrigin.Z += GridHeight + 2.0f;

    // ���� �ð� ����
    float DebugDuration = (Duration <= 0.0f) ? -1.0f : Duration;
    bool bPersistent = (Duration <= 0.0f);

    // ���� ����
    for (int32 i = 0; i <= GridRows; i++)
    {
        FVector Start = GridOrigin + FVector(i * CellSize, 0.0f, 0.0f);
        FVector End = GridOrigin + FVector(i * CellSize, GridColumns * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, bPersistent, DebugDuration, 200, 2.0f);
    }

    // ���� ����
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

    // �׸��� ���� ���
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z += GridHeight + 1.0f;

    float Margin = CellSize * 0.1f;

    // �� �� ���� �׸���
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index < 0 || Index >= Grid.Num()) continue;

            const FGridCell& Cell = Grid[Index];
            FColor CellColor;

            // �� ���¿� ���� ���� ����
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

            // ���� ������ ���
            FVector TopLeft = GridOrigin + FVector(Row * CellSize, Column * CellSize, 0.0f);
            FVector InnerTopLeft = TopLeft + FVector(Margin, Margin, 0.0f);
            FVector InnerBottomRight = TopLeft + FVector(CellSize - Margin, CellSize - Margin, 0.0f);

            // �ڽ� �߽ɰ� ũ�� ���
            FVector BoxCenter = (InnerTopLeft + InnerBottomRight) * 0.5f;
            FVector BoxExtent = (InnerBottomRight - InnerTopLeft) * 0.5f;

            // �� ���� �ڽ� �׸���
            DrawDebugBox(World, BoxCenter, BoxExtent, CellColor, true, -1.0f, 150, 1.0f);
        }
    }
}