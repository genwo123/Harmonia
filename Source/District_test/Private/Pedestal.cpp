// Pedestal.cpp
#include "Pedestal.h"
#include "Kismet/GameplayStatics.h"

APedestal::APedestal()
{
    PrimaryActorTick.bCanEverTick = false;

    // 기본값 설정
    CurrentState = EPedestalState::Empty;
    PlacedObject = nullptr;
    OwnerPuzzleArea = nullptr;
    GridRow = -1;
    GridColumn = -1;

    // 메시 컴포넌트 설정 (이미 부모 클래스에서 생성됨)
    if (MeshComponent)
    {
        MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
    }

    // 기본 상호작용 설정
    InteractionText = "Interact with Pedestal";
    InteractionType = EInteractionType::Default;
}

void APedestal::BeginPlay()
{
    Super::BeginPlay();

    // 퍼즐 에리어 찾기
    FindOwnerPuzzleArea();
}

void APedestal::FindOwnerPuzzleArea()
{
    if (!OwnerPuzzleArea)
    {
        // 모든 퍼즐 에리어 찾기
        TArray<AActor*> FoundAreas;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleArea::StaticClass(), FoundAreas);

        for (AActor* Area : FoundAreas)
        {
            APuzzleArea* PuzzleArea = Cast<APuzzleArea>(Area);
            if (PuzzleArea)
            {
                // 해당 퍼즐 에리어에 속하는지 확인
                int32 Row, Column;
                if (PuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), Row, Column))
                {
                    OwnerPuzzleArea = PuzzleArea;
                    GridRow = Row;
                    GridColumn = Column;
                    break;
                }
            }
        }
    }
}

void APedestal::Interact_Implementation(AActor* Interactor)
{
    // 상호작용 타입에 따라 다른 동작 수행
    switch (InteractionType)
    {
    case EInteractionType::Push:
        // 밀기 상호작용 (플레이어 방향으로 밀기)
        if (Interactor)
        {
            FVector Direction = GetActorLocation() - Interactor->GetActorLocation();
            Direction.Z = 0;
            Direction.Normalize();
            Push(Direction);
        }
        break;

    case EInteractionType::Rotate:
        // 회전 상호작용
        Rotate();
        break;

    default:
        // 기본 동작 (부모 클래스 호출)
        Super::Interact_Implementation(Interactor);
        break;
    }
}

bool APedestal::Push(FVector Direction)
{
    // 퍼즐 에리어가 없으면 찾기
    if (!OwnerPuzzleArea)
    {
        FindOwnerPuzzleArea();

        if (!OwnerPuzzleArea)
        {
            UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - no owner puzzle area found"));
            return false;
        }
    }

    // 방향 벡터를 그리드 방향으로 변환
    EGridDirection GridDirection;

    // 가장 큰 방향 성분 찾기
    if (FMath::Abs(Direction.X) > FMath::Abs(Direction.Y))
    {
        // X 방향이 더 큼
        GridDirection = Direction.X > 0 ? EGridDirection::East : EGridDirection::West;
    }
    else
    {
        // Y 방향이 더 큼
        GridDirection = Direction.Y > 0 ? EGridDirection::South : EGridDirection::North;
    }

    // 이동할 위치 계산
    int32 TargetRow = GridRow;
    int32 TargetColumn = GridColumn;

    switch (GridDirection)
    {
    case EGridDirection::North:
        TargetRow--;
        break;
    case EGridDirection::East:
        TargetColumn++;
        break;
    case EGridDirection::South:
        TargetRow++;
        break;
    case EGridDirection::West:
        TargetColumn--;
        break;
    }

    // 유효한 위치인지 확인
    if (!OwnerPuzzleArea->IsValidIndex(TargetRow, TargetColumn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - target position is invalid"));
        return false;
    }

    // 이동 가능한지 확인 (이동 불가 영역이 아닌지, 다른 받침대가 없는지)
    ECellState TargetState = OwnerPuzzleArea->GetCellState(TargetRow, TargetColumn);
    if (TargetState == ECellState::Unwalkable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - target cell is unwalkable"));
        return false;
    }

    AActor* ActorAtTarget = OwnerPuzzleArea->GetActorAtCell(TargetRow, TargetColumn);
    if (ActorAtTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - cell already occupied"));
        return false;
    }

    // 현재 위치에서 받침대 제거
    OwnerPuzzleArea->SetCellState(GridRow, GridColumn, ECellState::Walkable);

    // 새 위치로 이동
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(TargetRow, TargetColumn);
    SetActorLocation(NewLocation);

    // 그리드 위치 업데이트
    GridRow = TargetRow;
    GridColumn = TargetColumn;

    // 퍼즐 에리어에 등록
    OwnerPuzzleArea->PlaceActorAtCell(this, TargetRow, TargetColumn);

    return true;
}

void APedestal::Rotate(float Degrees)
{
    // 현재 회전에 각도 추가
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += Degrees;
    SetActorRotation(NewRotation);

    // 설치된 오브제도 같이 회전
    if (PlacedObject)
    {
        PlacedObject->SetActorRotation(NewRotation);
    }
}

bool APedestal::PlaceObject(AActor* Object)
{
    // 이미 다른 오브제가 설치되어 있으면 실패
    if (CurrentState == EPedestalState::Occupied && PlacedObject != Object)
    {
        return false;
    }

    // 오브제 설치
    if (Object)
    {
        // 오브제를 받침대 위에 배치
        Object->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        // 메시 위로 약간 올려서 배치
        float ZOffset = MeshComponent->Bounds.BoxExtent.Z;
        Object->SetActorRelativeLocation(FVector(0, 0, ZOffset));

        // 받침대와 같은 회전 적용
        Object->SetActorRotation(GetActorRotation());

        // 상태 업데이트
        PlacedObject = Object;
        CurrentState = EPedestalState::Occupied;
        return true;
    }

    return false;
}

AActor* APedestal::RemoveObject()
{
    // 설치된 오브제가 없으면 null 반환
    if (CurrentState != EPedestalState::Occupied || !PlacedObject)
    {
        return nullptr;
    }

    // 오브제 제거
    AActor* RemovedObject = PlacedObject;
    RemovedObject->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    PlacedObject = nullptr;
    CurrentState = EPedestalState::Empty;

    return RemovedObject;
}

void APedestal::SetPuzzleArea(APuzzleArea* PuzzleArea)
{
    OwnerPuzzleArea = PuzzleArea;
}

void APedestal::SetGridPosition(int32 Row, int32 Column)
{
    GridRow = Row;
    GridColumn = Column;
}

void APedestal::GetGridPosition(int32& OutRow, int32& OutColumn) const
{
    OutRow = GridRow;
    OutColumn = GridColumn;
}