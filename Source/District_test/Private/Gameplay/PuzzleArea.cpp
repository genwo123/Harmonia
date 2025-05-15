// PuzzleArea.cpp
#include "Gameplay/PuzzleArea.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Pedestal.h" // Pedestal 헤더 추가

APuzzleArea::APuzzleArea()
{
    PrimaryActorTick.bCanEverTick = true;

    // 기본값 설정 - 셀 크기를 400으로 설정
    GridRows = 5;
    GridColumns = 5;
    CellSize = 400.0f; // 4미터 (언리얼 단위)
    GridHeight = 0.0f;
    bShowDebugGrid = true;
    bShowGridInGame = false; // 게임에서는 기본적으로 그리드 표시 안 함

    // 색상 기본값
    WalkableColor = FLinearColor::Green;
    UnwalkableColor = FLinearColor::Red;
    PedestalSlotColor = FLinearColor::Yellow;
    OccupiedColor = FLinearColor::Blue;

    // 컴포넌트 생성 및 설정
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

    // 게임 시작 시 그리드 시각화 (에디터 조건 제거)
    if (bShowDebugGrid && GetWorld())
    {
        UpdateCellVisuals();
    }
}

void APuzzleArea::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 에디터에서 변경 시 즉시 업데이트
    UpdateGridSizeFromActorScale();

    // 이전 라인 지우고 다시 그리기
    if (GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
        UpdateCellVisuals();
    }
}


void APuzzleArea::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 에디터에서만 주기적으로 업데이트
    if (bShowDebugGrid && GetWorld() && GetWorld()->IsEditorWorld())
    {
        static float TimeSinceLastUpdate = 0.0f;
        TimeSinceLastUpdate += DeltaTime;

        // 1초마다 한 번 업데이트 (성능 최적화)
        if (TimeSinceLastUpdate >= 1.0f)
        {
            TimeSinceLastUpdate = 0.0f;
            UpdateCellVisuals();
        }
    }

    // 게임 시작했을 때는 Tick에서 업데이트하지 않음 (필요 시에만 호출)
}

// 디버그 라인이 이미 그려져 있는지 확인하는 헬퍼 함수
bool APuzzleArea::IsDebugLinePresent(UWorld* World)
{
    // 간단한 테스트로 첫 번째 셀의 중앙에 라인이 있는지 확인
    if (Grid.Num() > 0)
    {
        FVector CellCenter = Grid[0].WorldLocation;

        return false; // 보수적으로 항상 false 반환 (항상 다시 그림)
    }
    return false;
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

void APuzzleArea::RefreshCellVisuals()
{
    // 이전에 그려진 디버그 시각화 모두 지우기
    if (GetWorld())
    {
        FlushPersistentDebugLines(GetWorld());
    }

    // 셀 시각화 업데이트
    UpdateCellVisuals();
}


void APuzzleArea::SetCellState(int32 Row, int32 Column, ECellState NewState)
{
    if (IsValidIndex(Row, Column))
    {
        int32 Index = GetIndexFrom2DCoord(Row, Column);

        // 이전 상태와 다른 경우에만 업데이트
        if (Grid[Index].State != NewState)
        {
            // 새 상태 설정 (하나의 상태만 가짐)
            Grid[Index].State = NewState;

            // 즉시 시각화 업데이트
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
        Grid[Index].State = ECellState::Occupied; // 상태 변경

        // 시각적 업데이트
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

    // 셀이 이미 다른 액터로 점유되어 있으면 색상을 Unwalkable(빨간색)으로 변경
    if (Grid[Index].PlacedActor != nullptr && Grid[Index].PlacedActor != Actor)
    {
        Grid[Index].State = ECellState::Unwalkable; // 충돌 상태 표시
        UpdateCellVisuals(); // 즉시 시각적 업데이트
        return false;
    }

    // 액터 위치 설정 - 정확히 셀 중앙에 배치
    FVector Location = Grid[Index].WorldLocation;
    Actor->SetActorLocation(Location);

    // 그리드에 액터 등록
    Grid[Index].PlacedActor = Actor;

    // 액터가 배치되면 상태를 Occupied(파란색)로 변경
    Grid[Index].State = ECellState::Occupied;

    // 즉시 시각적 업데이트
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

    // 각 셀의 경계를 표시하는 라인 그리기
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z = GetActorLocation().Z + GridHeight + 1.0f;

    // 그리드 라인 그리기 - 흰색 실선 (두껍게, 항상 보이도록)
    for (int32 i = 0; i <= GridRows; i++)
    {
        FVector Start = GridOrigin + FVector(i * CellSize, 0.0f, 0.0f);
        FVector End = GridOrigin + FVector(i * CellSize, GridColumns * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, true, -1.0f, 100, 3.0f); // 영구적, 우선순위 높음, 두꺼운 선
    }

    for (int32 j = 0; j <= GridColumns; j++)
    {
        FVector Start = GridOrigin + FVector(0.0f, j * CellSize, 0.0f);
        FVector End = GridOrigin + FVector(GridRows * CellSize, j * CellSize, 0.0f);
        DrawDebugLine(World, Start, End, FColor::White, true, -1.0f, 100, 3.0f); // 영구적, 우선순위 높음, 두꺼운 선
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

        // UpdateTileMeshes 대신 UpdateCellVisuals 호출
        UpdateCellVisuals();
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

    // 이전 그리드 상태 및 배치된 액터 저장
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

    // 셀 크기는 유지하면서 행과 열 수 계산
    GridRows = FMath::Max(1, FMath::FloorToInt(WorldWidth / CellSize));
    GridColumns = FMath::Max(1, FMath::FloorToInt(WorldHeight / CellSize));

    // 그리드 크기가 변경된 경우에만 재초기화
    if (OldGridRows != GridRows || OldGridColumns != GridColumns || Grid.Num() == 0)
    {
        // 그리드 재초기화
        InitializeGrid();

        // 이전 상태 및 액터 복원 (가능한 범위 내에서)
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

        // 시각적 업데이트
        UpdateCellVisuals();
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

    // UpdateTileMeshes 대신 UpdateCellVisuals 호출
    UpdateCellVisuals();

    // 디버그 그리드 업데이트
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

    // 그리드 원점 계산
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z = GetActorLocation().Z + GridHeight;

    float Margin = CellSize * 0.1f;

    // 게임에서는 특정 상태만 표시하거나 간소화된 표시 가능
    // 예: 이동 불가 영역만 표시
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Column = 0; Column < GridColumns; Column++)
        {
            int32 Index = GetIndexFrom2DCoord(Row, Column);
            if (Index < 0 || Index >= Grid.Num()) continue;

            const FGridCell& Cell = Grid[Index];

            // 게임에서는 필요한 상태만 표시 (현재는 주석 처리)
            // 예를 들어, 설치 불가 영역만 표시하고 싶다면:
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

    // 셀이 이미 다른 액터로 점유되어 있는지 확인
    AActor* ExistingActor = Grid[Index].PlacedActor;
    AActor* PedestalActor = Cast<AActor>(Pedestal);

    // 이미 동일한 받침대가 등록되어 있으면 상태만 확인
    if (ExistingActor == PedestalActor)
    {
        // 상태가 Occupied가 아니면 변경 (그리고 시각화 업데이트)
        if (Grid[Index].State != ECellState::Occupied)
        {
            Grid[Index].State = ECellState::Occupied;
            UpdateCellVisuals();
        }
        return true;
    }

    // 이미 다른 액터가 점유 중이면, 충돌 처리
    if (ExistingActor != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell already occupied by %s, marking as Unwalkable (red)"),
            *ExistingActor->GetName());

        // 셀 상태를 Unwalkable(빨간색)로 변경
        Grid[Index].State = ECellState::Unwalkable;

        // 시각적 업데이트
        UpdateCellVisuals();

        return false; // 배치 실패 반환
    }

    // 받침대 등록 (첫 번째 받침대)
    Grid[Index].PlacedActor = PedestalActor;
    Grid[Index].State = ECellState::Occupied;

    // 시각적 업데이트
    UpdateCellVisuals();

    UE_LOG(LogTemp, Display, TEXT("Pedestal %s registered at grid position (%d, %d)"),
        *Pedestal->GetName(), Row, Column);

    return true;
}


void APuzzleArea::UpdateCellVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 이전에 그려진 모든 디버그 라인 지우기
    FlushPersistentDebugLines(World);

    // 그리드 라인 그리기 (에디터에서는 항상, 게임에서는 설정에 따라)
    if (World->IsEditorWorld() || bShowGridInGame)
    {
        DrawDebugGrid(-1.0f); // 영구적으로 표시
    }

    // 셀 상태는 환경에 따라 다르게 표시
    if (World->IsEditorWorld())
    {
        // 에디터에서만 셀 색상 표시
        DrawCellsInEditor();
    }
    else if (World->HasBegunPlay() && bShowGridInGame)
    {
        // 게임에서는 설정에 따라 표시 (기본적으로 비활성화)
        DrawCellsInGame(bShowGridInGame);
    }
}
void APuzzleArea::DrawCellsInEditor()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 그리드 원점 계산
    FVector GridOrigin = GetActorLocation();
    GridOrigin.X -= GridRows * CellSize * 0.5f;
    GridOrigin.Y -= GridColumns * CellSize * 0.5f;
    GridOrigin.Z = GetActorLocation().Z + GridHeight;

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
            FVector TopLeft = GridOrigin + FVector(Row * CellSize, Column * CellSize, 0.1f);

            // 마진을 적용한 내부 사각형 꼭지점 계산
            FVector InnerTopLeft = TopLeft + FVector(Margin, Margin, 0.0f);
            FVector InnerBottomRight = TopLeft + FVector(CellSize - Margin, CellSize - Margin, 0.0f);

            // 내부 사각형의 중심점과 크기 계산
            FVector BoxCenter = (InnerTopLeft + InnerBottomRight) * 0.5f;
            FVector BoxExtent = (InnerBottomRight - InnerTopLeft) * 0.5f;

            // 셀 내부에 색상 박스 그리기 (영구적으로 설정)
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

    // 셀이 비어있는지 확인
    int32 Index = GetIndexFrom2DCoord(Row, Column);
    if (Grid[Index].PlacedActor != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cell at (%d, %d) is already occupied"), Row, Column);
        return nullptr;
    }

    // 월드 위치 계산
    FVector Location = GetWorldLocationFromGridIndex(Row, Column);
    FRotator Rotation = FRotator::ZeroRotator;

    // 받침대 생성
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APedestal* NewPedestal = World->SpawnActor<APedestal>(InPedestalClass, Location, Rotation, SpawnParams);

    if (NewPedestal)
    {
        // 그리드 정보 설정
        NewPedestal->SetPuzzleArea(this);
        NewPedestal->SetGridPosition(Row, Column);

        // 셀 상태 업데이트
        Grid[Index].PlacedActor = NewPedestal;
        Grid[Index].State = ECellState::Occupied;

        // 타겟 정보 설정 (블루프린트 속성)
        NewPedestal->TargetPuzzleArea = this;
        NewPedestal->TargetGridRow = Row;
        NewPedestal->TargetGridColumn = Column;

        // 시각화 업데이트
        UpdateCellVisuals();

        UE_LOG(LogTemp, Display, TEXT("Created pedestal at grid position (%d, %d)"), Row, Column);
    }

    return NewPedestal;
}

void APuzzleArea::ResetGridState()
{
    // 모든 받침대 찾기
    TArray<AActor*> FoundPedestals;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APedestal::StaticClass(), FoundPedestals);

    UE_LOG(LogTemp, Warning, TEXT("Resetting grid state. Found %d pedestals"), FoundPedestals.Num());

    // 모든 셀 상태 초기화
    for (int32 i = 0; i < Grid.Num(); i++)
    {
        Grid[i].State = ECellState::Walkable;
        Grid[i].PlacedActor = nullptr;
    }

    // 받침대의 실제 위치에 따라 다시 등록
    for (AActor* Actor : FoundPedestals)
    {
        APedestal* Pedestal = Cast<APedestal>(Actor);
        if (Pedestal)
        {
            int32 Row, Column;
            if (GetGridIndexFromWorldLocation(Pedestal->GetActorLocation(), Row, Column))
            {
                // 받침대 그리드 위치 업데이트
                Pedestal->SetGridPosition(Row, Column);
                Pedestal->SetPuzzleArea(this);

                // 셀에 받침대 등록
                int32 Index = GetIndexFrom2DCoord(Row, Column);
                Grid[Index].PlacedActor = Pedestal;
                Grid[Index].State = ECellState::Occupied;

                UE_LOG(LogTemp, Display, TEXT("Re-registered pedestal at (%d, %d)"), Row, Column);
            }
        }
    }

    // 시각화 업데이트
    UpdateCellVisuals();

    UE_LOG(LogTemp, Warning, TEXT("Grid state reset complete"));
}