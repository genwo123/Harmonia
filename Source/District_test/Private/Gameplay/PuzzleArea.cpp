// PuzzleArea.cpp
#include "Gameplay/PuzzleArea.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Pedestal.h" // Pedestal ��� �߰�

APuzzleArea::APuzzleArea()
{
    PrimaryActorTick.bCanEverTick = true;

    // �⺻�� ���� - �� ũ�⸦ 400���� ����
    GridRows = 5;
    GridColumns = 5;
    CellSize = 400.0f; // 4���� (�𸮾� ����)
    GridHeight = 0.0f;
    bShowDebugGrid = true;
    bShowGridInGame = false; // ���ӿ����� �⺻������ �׸��� ǥ�� �� ��

    // ���� �⺻��
    WalkableColor = FLinearColor::Green;
    UnwalkableColor = FLinearColor::Red;
    PedestalSlotColor = FLinearColor::Yellow;
    OccupiedColor = FLinearColor::Blue;

    // ������Ʈ ���� �� ����
    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    SetRootComponent(AreaBox);
    AreaBox->SetCollisionProfileName(TEXT("BlockAll"));
    AreaBox->SetBoxExtent(FVector(GridRows * CellSize * 0.5f, GridColumns * CellSize * 0.5f, 50.0f));
}

void APuzzleArea::BeginPlay()
{
    Super::BeginPlay();

    if (Grid.Num() == 0)
    {
        InitializeGrid();
    }

    // ���� ���� �� �׸��� �ð�ȭ (������ ���� ����)
    if (bShowDebugGrid && GetWorld())
    {
        UpdateCellVisuals();
    }
}

void APuzzleArea::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // �����Ϳ��� ���� �� ��� ������Ʈ
    UpdateGridSizeFromActorScale();

    // ���� ���� ����� �ٽ� �׸���
    if (GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        UpdateCellVisuals();
    }
}


void APuzzleArea::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // �����Ϳ����� �ֱ������� ������Ʈ
    if (bShowDebugGrid && GetWorld() && GetWorld()->IsEditorWorld())
    {
        static float TimeSinceLastUpdate = 0.0f;
        TimeSinceLastUpdate += DeltaTime;

        // 1�ʸ��� �� �� ������Ʈ (���� ����ȭ)
        if (TimeSinceLastUpdate >= 1.0f)
        {
            TimeSinceLastUpdate = 0.0f;
            UpdateCellVisuals();
        }
    }

    // ���� �������� ���� Tick���� ������Ʈ���� ���� (�ʿ� �ÿ��� ȣ��)
}

// ����� ������ �̹� �׷��� �ִ��� Ȯ���ϴ� ���� �Լ�
bool APuzzleArea::IsDebugLinePresent(UWorld* World)
{
    // ������ �׽�Ʈ�� ù ��° ���� �߾ӿ� ������ �ִ��� Ȯ��
    if (Grid.Num() > 0)
    {
        FVector CellCenter = Grid[0].WorldLocation;

        return false; // ���������� �׻� false ��ȯ (�׻� �ٽ� �׸�)
    }
    return false;
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

void APuzzleArea::RefreshCellVisuals()
{
    // ������ �׷��� ����� �ð�ȭ ��� �����
    if (GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
    }

    // �� �ð�ȭ ������Ʈ
    UpdateCellVisuals();
}


void APuzzleArea::SetCellState(int32 Row, int32 Column, ECellState NewState)
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);

        // ���� ���¿� �ٸ� ��쿡�� ������Ʈ
        if (Grid[Index].State != NewState)
        {
            // �� ���� ���� (�ϳ��� ���¸� ����)
            Grid[Index].State = NewState;

            // ��� �ð�ȭ ������Ʈ
            UpdateCellVisuals();
        }
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
        Grid[Index].State = ECellState::Occupied; // ���� ����

        // �ð��� ������Ʈ
        UpdateCellVisuals();
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

    // ���� �̹� �ٸ� ���ͷ� �����Ǿ� ������ ������ Unwalkable(������)���� ����
    if (Grid[Index].PlacedActor != nullptr && Grid[Index].PlacedActor != Actor)
    {
        Grid[Index].State = ECellState::Unwalkable; // �浹 ���� ǥ��
        UpdateCellVisuals(); // ��� �ð��� ������Ʈ
        return false;
    }

    // ���� ��ġ ���� - ��Ȯ�� �� �߾ӿ� ��ġ
    FVector Location = Grid[Index].WorldLocation;
    Actor->SetActorLocation(Location);

    // �׸��忡 ���� ���
    Grid[Index].PlacedActor = Actor;

    // ���Ͱ� ��ġ�Ǹ� ���¸� Occupied(�Ķ���)�� ����
    Grid[Index].State = ECellState::Occupied;

    // ��� �ð��� ������Ʈ
    UpdateCellVisuals();

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

    // �׸��� ���� �׸��� - ��� �Ǽ� (�β���, �׻� ���̵���)
    for (int32 i = 0; i <= GridRows; i++)
    {
        FVector Start = GridOrigin + FVector(i * CellSize, 0.0f, 0.0f);
        FVector End = GridOrigin + FVector(i * CellSize, GridColumns * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, true, -1.0f, 100, 3.0f); // ������, �켱���� ����, �β��� ��
    }

    for (int32 j = 0; j <= GridColumns; j++)
    {
        FVector Start = GridOrigin + FVector(0.0f, j * CellSize, 0.0f);
        FVector End = GridOrigin + FVector(GridRows * CellSize, j * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, true, -1.0f, 100, 3.0f); // ������, �켱���� ����, �β��� ��
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

        // UpdateTileMeshes ��� UpdateCellVisuals ȣ��
        UpdateCellVisuals();
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

    // ���� �׸��� ���� �� ��ġ�� ���� ����
    TArray<ECellState> OldStates;
    TArray<AActor*> OldActors;
    int32 OldGridRows = GridRows;
    int32 OldGridColumns = GridColumns;

    if (Grid.Num() > 0)
    {
        OldStates.SetNum(Grid.Num());
        OldActors.SetNum(Grid.Num());

        for (int32 i = 0; i < Grid.Num(); i++)
        {
            OldStates[i] = Grid[i].State;
            OldActors[i] = Grid[i].PlacedActor;
        }
    }

    // �� ũ��� �����ϸ鼭 ��� �� �� ���
    GridRows = FMath::Max(1, FMath::FloorToInt(WorldWidth / CellSize));
    GridColumns = FMath::Max(1, FMath::FloorToInt(WorldHeight / CellSize));

    // �׸��� ũ�Ⱑ ����� ��쿡�� ���ʱ�ȭ
    if (OldGridRows != GridRows || OldGridColumns != GridColumns || Grid.Num() == 0)
    {
        // �׸��� ���ʱ�ȭ
        InitializeGrid();

        // ���� ���� �� ���� ���� (������ ���� ������)
        if (OldStates.Num() > 0)
        {
            int32 MinRows = FMath::Min(OldGridRows, GridRows);
            int32 MinColumns = FMath::Min(OldGridColumns, GridColumns);

            for (int32 Row = 0; Row < MinRows; Row++)
            {
                for (int32 Column = 0; Column < MinColumns; Column++)
                {
                    int32 OldIndex = Row * OldGridColumns + Column;
                    int32 NewIndex = Row * GridColumns + Column;

                    if (OldIndex < OldStates.Num() && NewIndex < Grid.Num())
                    {
                        Grid[NewIndex].State = OldStates[OldIndex];
                        Grid[NewIndex].PlacedActor = OldActors[OldIndex];
                    }
                }
            }
        }

        // �ð��� ������Ʈ
        UpdateCellVisuals();
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

    // UpdateTileMeshes ��� UpdateCellVisuals ȣ��
    UpdateCellVisuals();

    // ����� �׸��� ������Ʈ
    if (bShowDebugGrid && GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        DrawDebugGrid(0.0f);
    }
}

void APuzzleArea::DrawCellsInGame(bool bEnable)
{
    UWorld* World = GetWorld();
    if (!World || !bEnable) return;

    // �׸��� ���� ���
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z = GetActorLocation().Z + GridHeight;

    float Margin = CellSize * 0.1f;

    // ���ӿ����� Ư�� ���¸� ǥ���ϰų� ����ȭ�� ǥ�� ����
    // ��: �̵� �Ұ� ������ ǥ��
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index < 0 || Index >= Grid.Num()) continue;

            const FGridCell& Cell = Grid[Index];

            // ���ӿ����� �ʿ��� ���¸� ǥ�� (����� �ּ� ó��)
            // ���� ���, ��ġ �Ұ� ������ ǥ���ϰ� �ʹٸ�:
            /*
            if (Cell.State == ECellState::Unwalkable)
            {
                FVector TopLeft = GridOrigin + FVector(Row * CellSize, Column * CellSize, 0.1f);
                FVector InnerTopLeft = TopLeft + FVector(Margin, Margin, 0.0f);
                FVector InnerBottomRight = TopLeft + FVector(CellSize - Margin, CellSize - Margin, 0.0f);
                FVector BoxCenter = (InnerTopLeft + InnerBottomRight) * 0.5f;
                FVector BoxExtent = (InnerBottomRight - InnerTopLeft) * 0.5f;

                DrawDebugBox(World, BoxCenter, BoxExtent, UnwalkableColor.ToFColor(true), false, -1.0f, 0, 0);
            }
            */
        }
    }
}


bool APuzzleArea::RegisterPedestal(APedestal* Pedestal, int32 Row, int32 Column)
{
    if (!Pedestal || !IsValidIndex(Row, Column))
    {
        return false;
    }

    int32 Index = GetIndexFrom2DCoord(Row, Column);

    // ���� �̹� �ٸ� ���ͷ� �����Ǿ� �ִ��� Ȯ��
    AActor* ExistingActor = Grid[Index].PlacedActor;
    AActor* PedestalActor = Cast<AActor>(Pedestal);

    // �̹� ������ ��ħ�밡 ��ϵǾ� ������ ���¸� Ȯ��
    if (ExistingActor == PedestalActor)
    {
        // ���°� Occupied�� �ƴϸ� ���� (�׸��� �ð�ȭ ������Ʈ)
        if (Grid[Index].State != ECellState::Occupied)
        {
            Grid[Index].State = ECellState::Occupied;
            UpdateCellVisuals();
        }
        return true;
    }

    // �̹� �ٸ� ���Ͱ� ���� ���̸�, �浹 ó��
    if (ExistingActor != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell already occupied by %s, marking as Unwalkable (red)"),
            *ExistingActor->GetName());

        // �� ���¸� Unwalkable(������)�� ����
        Grid[Index].State = ECellState::Unwalkable;

        // �ð��� ������Ʈ
        UpdateCellVisuals();

        return false; // ��ġ ���� ��ȯ
    }

    // ��ħ�� ��� (ù ��° ��ħ��)
    Grid[Index].PlacedActor = PedestalActor;
    Grid[Index].State = ECellState::Occupied;

    // �ð��� ������Ʈ
    UpdateCellVisuals();

    UE_LOG(LogTemp, Display, TEXT("Pedestal %s registered at grid position (%d, %d)"),
        *Pedestal->GetName(), Row, Column);

    return true;
}


void APuzzleArea::UpdateCellVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // ������ �׷��� ��� ����� ���� �����
    FlushPersistentDebugLines(World);

    // �׸��� ���� �׸��� (�����Ϳ����� �׻�, ���ӿ����� ������ ����)
    if (World->IsEditorWorld() || bShowGridInGame)
    {
        DrawDebugGrid(-1.0f); // ���������� ǥ��
    }

    // �� ���´� ȯ�濡 ���� �ٸ��� ǥ��
    if (World->IsEditorWorld())
    {
        // �����Ϳ����� �� ���� ǥ��
        DrawCellsInEditor();
    }
    else if (World->HasBegunPlay() && bShowGridInGame)
    {
        // ���ӿ����� ������ ���� ǥ�� (�⺻������ ��Ȱ��ȭ)
        DrawCellsInGame(bShowGridInGame);
    }
}
void APuzzleArea::DrawCellsInEditor()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // �׸��� ���� ���
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z = GetActorLocation().Z + GridHeight;

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
            FVector TopLeft = GridOrigin + FVector(Row * CellSize, Column * CellSize, 0.1f);

            // ������ ������ ���� �簢�� ������ ���
            FVector InnerTopLeft = TopLeft + FVector(Margin, Margin, 0.0f);
            FVector InnerBottomRight = TopLeft + FVector(CellSize - Margin, CellSize - Margin, 0.0f);

            // ���� �簢���� �߽����� ũ�� ���
            FVector BoxCenter = (InnerTopLeft + InnerBottomRight) * 0.5f;
            FVector BoxExtent = (InnerBottomRight - InnerTopLeft) * 0.5f;

            // �� ���ο� ���� �ڽ� �׸��� (���������� ����)
            DrawDebugBox(World, BoxCenter, BoxExtent, CellColor, true, -1.0f, 200, 0);
        }
    }
}
void APuzzleArea::GetGridDimensions(int32& OutRows, int32& OutColumns) const
{
    OutRows = GridRows;
    OutColumns = GridColumns;
}

APedestal* APuzzleArea::CreatePedestalAtGridPosition(TSubclassOf<APedestal> InPedestalClass, int32 Row, int32 Column)
{
    if (!InPedestalClass || !IsValidIndex(Row, Column))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid parameters for pedestal creation"));
        return nullptr;
    }

    // ���� ����ִ��� Ȯ��
    int32 Index = GetIndexFrom2DCoord(Row, Column);
    if (Grid[Index].PlacedActor != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell at (%d, %d) is already occupied"), Row, Column);
        return nullptr;
    }

    // ���� ��ġ ���
    FVector Location = GetWorldLocationFromGridIndex(Row, Column);
    FRotator Rotation = FRotator::ZeroRotator;

    // ��ħ�� ����
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APedestal* NewPedestal = World->SpawnActor<APedestal>(InPedestalClass, Location, Rotation, SpawnParams);

    if (NewPedestal)
    {
        // �׸��� ���� ����
        NewPedestal->SetPuzzleArea(this);
        NewPedestal->SetGridPosition(Row, Column);

        // �� ���� ������Ʈ
        Grid[Index].PlacedActor = NewPedestal;
        Grid[Index].State = ECellState::Occupied;

        // Ÿ�� ���� ���� (�������Ʈ �Ӽ�)
        NewPedestal->TargetPuzzleArea = this;
        NewPedestal->TargetGridRow = Row;
        NewPedestal->TargetGridColumn = Column;

        // �ð�ȭ ������Ʈ
        UpdateCellVisuals();

        UE_LOG(LogTemp, Display, TEXT("Created pedestal at grid position (%d, %d)"), Row, Column);
    }

    return NewPedestal;
}

void APuzzleArea::ResetGridState()
{
    // ��� ��ħ�� ã��
    TArray<AActor*> FoundPedestals;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APedestal::StaticClass(), FoundPedestals);

    UE_LOG(LogTemp, Warning, TEXT("Resetting grid state. Found %d pedestals"), FoundPedestals.Num());

    // ��� �� ���� �ʱ�ȭ
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        Grid[i].State = ECellState::Walkable;
        Grid[i].PlacedActor = nullptr;
    }

    // ��ħ���� ���� ��ġ�� ���� �ٽ� ���
    for (AActor* Actor : FoundPedestals)
    {
        APedestal* Pedestal = Cast<APedestal>(Actor);
        if (Pedestal)
        {
            int32 Row, Column;
            if (GetGridIndexFromWorldLocation(Pedestal->GetActorLocation(), Row, Column))
            {
                // ��ħ�� �׸��� ��ġ ������Ʈ
                Pedestal->SetGridPosition(Row, Column);
                Pedestal->SetPuzzleArea(this);

                // ���� ��ħ�� ���
                int32 Index = GetIndexFrom2DCoord(Row, Column);
                Grid[Index].PlacedActor = Pedestal;
                Grid[Index].State = ECellState::Occupied;

                UE_LOG(LogTemp, Display, TEXT("Re-registered pedestal at (%d, %d)"), Row, Column);
            }
        }
    }

    // �ð�ȭ ������Ʈ
    UpdateCellVisuals();

    UE_LOG(LogTemp, Warning, TEXT("Grid state reset complete"));
}