// Pedestal.cpp
#include "Pedestal.h"
#include "Kismet/GameplayStatics.h"

APedestal::APedestal()
{
    PrimaryActorTick.bCanEverTick = false;

    // �⺻�� ����
    CurrentState = EPedestalState::Empty;
    PlacedObject = nullptr;
    OwnerPuzzleArea = nullptr;
    GridRow = -1;
    GridColumn = -1;

    // �޽� ������Ʈ ���� (�̹� �θ� Ŭ�������� ������)
    if (MeshComponent)
    {
        MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
    }

    // �⺻ ��ȣ�ۿ� ����
    InteractionText = "Interact with Pedestal";
    InteractionType = EInteractionType::Default;
}

void APedestal::BeginPlay()
{
    Super::BeginPlay();

    // ���� ������ ã��
    FindOwnerPuzzleArea();
}

void APedestal::FindOwnerPuzzleArea()
{
    if (!OwnerPuzzleArea)
    {
        // ��� ���� ������ ã��
        TArray<AActor*> FoundAreas;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleArea::StaticClass(), FoundAreas);

        for (AActor* Area : FoundAreas)
        {
            APuzzleArea* PuzzleArea = Cast<APuzzleArea>(Area);
            if (PuzzleArea)
            {
                // �ش� ���� ����� ���ϴ��� Ȯ��
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
    // ��ȣ�ۿ� Ÿ�Կ� ���� �ٸ� ���� ����
    switch (InteractionType)
    {
    case EInteractionType::Push:
        // �б� ��ȣ�ۿ� (�÷��̾� �������� �б�)
        if (Interactor)
        {
            FVector Direction = GetActorLocation() - Interactor->GetActorLocation();
            Direction.Z = 0;
            Direction.Normalize();
            Push(Direction);
        }
        break;

    case EInteractionType::Rotate:
        // ȸ�� ��ȣ�ۿ�
        Rotate();
        break;

    default:
        // �⺻ ���� (�θ� Ŭ���� ȣ��)
        Super::Interact_Implementation(Interactor);
        break;
    }
}

bool APedestal::Push(FVector Direction)
{
    // ���� ����� ������ ã��
    if (!OwnerPuzzleArea)
    {
        FindOwnerPuzzleArea();

        if (!OwnerPuzzleArea)
        {
            UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - no owner puzzle area found"));
            return false;
        }
    }

    // ���� ���͸� �׸��� �������� ��ȯ
    EGridDirection GridDirection;

    // ���� ū ���� ���� ã��
    if (FMath::Abs(Direction.X) > FMath::Abs(Direction.Y))
    {
        // X ������ �� ŭ
        GridDirection = Direction.X > 0 ? EGridDirection::East : EGridDirection::West;
    }
    else
    {
        // Y ������ �� ŭ
        GridDirection = Direction.Y > 0 ? EGridDirection::South : EGridDirection::North;
    }

    // �̵��� ��ġ ���
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

    // ��ȿ�� ��ġ���� Ȯ��
    if (!OwnerPuzzleArea->IsValidIndex(TargetRow, TargetColumn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - target position is invalid"));
        return false;
    }

    // �̵� �������� Ȯ�� (�̵� �Ұ� ������ �ƴ���, �ٸ� ��ħ�밡 ������)
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

    // ���� ��ġ���� ��ħ�� ����
    OwnerPuzzleArea->SetCellState(GridRow, GridColumn, ECellState::Walkable);

    // �� ��ġ�� �̵�
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(TargetRow, TargetColumn);
    SetActorLocation(NewLocation);

    // �׸��� ��ġ ������Ʈ
    GridRow = TargetRow;
    GridColumn = TargetColumn;

    // ���� ����� ���
    OwnerPuzzleArea->PlaceActorAtCell(this, TargetRow, TargetColumn);

    return true;
}

void APedestal::Rotate(float Degrees)
{
    // ���� ȸ���� ���� �߰�
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += Degrees;
    SetActorRotation(NewRotation);

    // ��ġ�� �������� ���� ȸ��
    if (PlacedObject)
    {
        PlacedObject->SetActorRotation(NewRotation);
    }
}

bool APedestal::PlaceObject(AActor* Object)
{
    // �̹� �ٸ� �������� ��ġ�Ǿ� ������ ����
    if (CurrentState == EPedestalState::Occupied && PlacedObject != Object)
    {
        return false;
    }

    // ������ ��ġ
    if (Object)
    {
        // �������� ��ħ�� ���� ��ġ
        Object->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        // �޽� ���� �ణ �÷��� ��ġ
        float ZOffset = MeshComponent->Bounds.BoxExtent.Z;
        Object->SetActorRelativeLocation(FVector(0, 0, ZOffset));

        // ��ħ��� ���� ȸ�� ����
        Object->SetActorRotation(GetActorRotation());

        // ���� ������Ʈ
        PlacedObject = Object;
        CurrentState = EPedestalState::Occupied;
        return true;
    }

    return false;
}

AActor* APedestal::RemoveObject()
{
    // ��ġ�� �������� ������ null ��ȯ
    if (CurrentState != EPedestalState::Occupied || !PlacedObject)
    {
        return nullptr;
    }

    // ������ ����
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