// PuzzleArea.cpp
#include "PuzzleArea.h"
#include "DrawDebugHelpers.h"

APuzzleArea::APuzzleArea()
{
    PrimaryActorTick.bCanEverTick = true;

    // �⺻�� ����
    GridRows = 10;
    GridColumns = 10;
    CellSize = 400.0f; // 4����
    GridHeight = 0.0f;
    bShowDebugGrid = true;

    // ���� �⺻��
    WalkableColor = FLinearColor::Green;
    UnwalkableColor = FLinearColor::Red;
    PedestalSlotColor = FLinearColor::Yellow;

    // ������Ʈ ���� �� ����
    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    SetRootComponent(AreaBox);
    AreaBox->SetCollisionProfileName(TEXT("BlockAll"));
    AreaBox->SetBoxExtent(FVector(GridRows * CellSize * 0.5f, GridColumns * CellSize * 0.5f, 50.0f));

    // �ν��Ͻ��� ����ƽ �޽� ������Ʈ ����
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

    // ���� �߿��� �׻� �׸��� ǥ��
    if (bShowDebugGrid && GetWorld())
    {
        // ������ �׷��� ����� ���� ��� �����
        FlushPersistentDebugLines(GetWorld());

        // ���ο� �׸��� ���� �׸���
        DrawDebugGrid(-1.0f); // -1.0f�� �����Ͽ� ���� ǥ��
    }
}

void APuzzleArea::InitializeGrid()
{
    // ���� �׸��� �ʱ�ȭ
    Grid.Empty();
    Grid.SetNum(GridRows * GridColumns);

    // �� �� ���� ���� - ���� ���� ��ġ
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);

            if (Index >= 0 && Index < Grid.Num())
            {
                // ���� ��ġ ��� - ������ 0���� �Ͽ� ���� ���� �ٵ��� ��
                FVector CellLocation = GetWorldLocationFromGridIndex(Row, Column);

                // �� ���� ����
                Grid[Index].WorldLocation = CellLocation;
                Grid[Index].State = ECellState::Walkable; // �⺻ �̵� ���� ����
                Grid[Index].PlacedActor = nullptr;
            }
        }
    }
}

void APuzzleArea::UpdateTileMeshes()
{
    // ���� �ν��Ͻ� �ʱ�ȭ
    WalkableTiles->ClearInstances();
    UnwalkableTiles->ClearInstances();
    PedestalSlotTiles->ClearInstances();

    // �� Ÿ�� �޽� ����
    WalkableTiles->SetStaticMesh(WalkableTileMesh);
    UnwalkableTiles->SetStaticMesh(UnwalkableTileMesh);
    PedestalSlotTiles->SetStaticMesh(PedestalSlotTileMesh);

    // Ÿ�� �޽� ũ�� ���� (�޽��� �⺻ ũ�⿡ ���� �������� �����ؾ� ��)
    // ��: �⺻ �޽ð� 100x100�� ��� 4.0 �������� ����
    float TileScale = CellSize / 100.0f; // �޽��� �⺻ ũ�⿡ ���� ���� �ʿ�

    // �� ���� ������ �޽� �ν��Ͻ� �߰�
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        const FGridCell& Cell = Grid[i];

        // �� �߾ӿ� ��Ȯ�� ��ġ�ϰ� ������ ����
        FTransform TileTransform(
            FQuat::Identity,
            Cell.WorldLocation,
            FVector(TileScale, TileScale, 1.0f) // ���̴� �״�� �����ϰų� �ʿ信 ���� ����
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

        // �ν��Ͻ��� �޽� ������Ʈ
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
    return ECellState::Unwalkable; // ���� ����� �̵� �Ұ� ��ȯ
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

    // �׸��� ���� ������ ���� (�׸���� �߾��� �����̹Ƿ�)
    LocalLocation.X += (GridRows * CellSize * 0.5f);
    LocalLocation.Y += (GridColumns * CellSize * 0.5f);

    // �׸��� ��ǥ ���
    OutRow = FMath::FloorToInt(LocalLocation.X / CellSize);
    OutColumn = FMath::FloorToInt(LocalLocation.Y / CellSize);

    // ��ȿ�� �ε������� Ȯ��
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

    // ��ħ�� ���� ��ġ
    FVector Location = GetWorldLocationFromGridIndex(Row, Column);
    FRotator Rotation = FRotator::ZeroRotator;

    // ��ħ�� ����
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = this;

    AActor* Pedestal = World->SpawnActor<AActor>(PedestalClass, Location, Rotation, SpawnParams);

    if (Pedestal)
    {
        // ������ ��ħ�븦 �׸��忡 ���
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

    // ���� ����־�߸� ��ġ ����
    if (Grid[Index].PlacedActor != nullptr && Grid[Index].PlacedActor != Actor)
    {
        return false;
    }

    // ���� ��ġ ���� - ��Ȯ�� �� �߾ӿ� ��ġ
    FVector Location = Grid[Index].WorldLocation;
    Actor->SetActorLocation(Location);

    // �׸��忡 ���� ���
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

    // �� ���� ��踦 ǥ���ϴ� ���� �׸���
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z = GetActorLocation().Z + GridHeight + 1.0f;

    // �׸��� ���� �׸��� - ���⼭ �Ű����� ����
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
    // ��� ���� ��ġ�� ���� ���� (��ħ�� ����)
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        if (Grid[i].PlacedActor)
        {
            // ��ħ������ Ȯ�� ���� �ʿ� (���⼭�� �����ϰ� ó��)
            // ���� ���ӿ����� ��ħ��� ���� ��Ҹ� �����ϴ� ����� �ʿ���
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
    // ������ ���� ������ ��������
    FVector ActorScale = GetActorScale3D();

    // �ڽ� ������Ʈ�� �⺻ ũ�� ��������
    FVector BoxExtent = AreaBox->GetUnscaledBoxExtent();

    // ���� ���� ũ�� ��� (�⺻ �ڽ� ũ�� * ������)
    float WorldWidth = BoxExtent.X * 2.0f * ActorScale.X;
    float WorldHeight = BoxExtent.Y * 2.0f * ActorScale.Y;

    // �� ũ��� �����ϸ鼭 ��� �� �� ���
    GridRows = FMath::Max(1, FMath::FloorToInt(WorldWidth / CellSize));
    GridColumns = FMath::Max(1, FMath::FloorToInt(WorldHeight / CellSize));

    // �׸��� ���ʱ�ȭ
    InitializeGrid();
    UpdateTileMeshes();

    // ����� �׸��� ������Ʈ
    if (bShowDebugGrid && GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        DrawDebugGrid(0.0f);
    }
}

void APuzzleArea::SetGridSize(float Width, float Height)
{
    // �� ũ��� �����ϸ鼭 ��� �� �� ���
    GridRows = FMath::Max(1, FMath::FloorToInt(Width / CellSize));
    GridColumns = FMath::Max(1, FMath::FloorToInt(Height / CellSize));

    // �ڽ� ũ�� ����
    FVector NewExtent(Width * 0.5f, Height * 0.5f, AreaBox->GetUnscaledBoxExtent().Z);
    AreaBox->SetBoxExtent(NewExtent);

    // �׸��� ���ʱ�ȭ
    InitializeGrid();
    UpdateTileMeshes();

    // ����� �׸��� ������Ʈ
    if (bShowDebugGrid && GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        DrawDebugGrid(0.0f);
    }
}