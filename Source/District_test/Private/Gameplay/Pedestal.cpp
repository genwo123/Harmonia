// Pedestal.cpp
#include "Gameplay/Pedestal.h"
#include "Kismet/GameplayStatics.h"
#include "Character/HamoniaCharacter.h"

APedestal::APedestal()
{
    PrimaryActorTick.bCanEverTick = false;

    // �⺻�� ����
    CurrentState = EPedestalState::Empty;
    PlacedObject = nullptr;
    OwnerPuzzleArea = nullptr;
    GridRow = -1;
    GridColumn = -1;

    // Ÿ�� �׸��� ��ǥ �ʱ�ȭ
    TargetGridRow = 0;
    TargetGridColumn = 0;
    TargetPuzzleArea = nullptr;

    // �޽� ������Ʈ ����
    if (!MeshComponent)
    {
        MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
        RootComponent = MeshComponent;
    }

    // �ݸ��� ���� - �߿��� �κ�
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetGenerateOverlapEvents(true);

    // ��ȣ�ۿ� ���� �߰� (PickupActoró��) - �������Ʈ���� ��ġ ���� ����
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
    InteractionSphere->SetGenerateOverlapEvents(true);
    InteractionSphere->SetRelativeLocation(FVector(0, 0, 50.0f)); // �⺻ ��ġ ����

    //���� ��ġ ����
    AttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttachmentPoint"));
    AttachmentPoint->SetupAttachment(RootComponent);
    AttachmentPoint->SetRelativeLocation(FVector(0, 0, 80.0f));

    // ������ �̺�Ʈ ���ε�
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APedestal::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APedestal::OnInteractionSphereEndOverlap);

    // �⺻ ��ȣ�ۿ� ����
    InteractionText = "Interact with Pedestal";
    InteractionType = EInteractionType::Default;
}


void APedestal::BeginPlay()
{
    Super::BeginPlay();

    // Ÿ�� ���� ����� �����Ǿ����� ���
    if (TargetPuzzleArea)
    {
        OwnerPuzzleArea = TargetPuzzleArea;

        // Ÿ�� �׸��� ��ǥ�� �̵�
        if (MoveToGridPosition(TargetGridRow, TargetGridColumn))
        {
            UE_LOG(LogTemp, Display, TEXT("Pedestal moved to target position (%d, %d) in %s"),
                TargetGridRow, TargetGridColumn, *TargetPuzzleArea->GetName());
        }
    }
    else
    {
        // Ÿ���� ������ ���� ������� ���� ������ ã��
        FindOwnerPuzzleArea();
    }
}

void APedestal::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // Ÿ�� ���� ����� �����Ǿ����� ���
    if (TargetPuzzleArea)
    {
        OwnerPuzzleArea = TargetPuzzleArea;

        // Ÿ�� �׸��� ��ǥ�� �̵�
        if (MoveToGridPosition(TargetGridRow, TargetGridColumn))
        {
            // �����Ϳ����� �α� ���
            if (GWorld && GWorld->IsEditorWorld())
            {
                UE_LOG(LogTemp, Display, TEXT("Pedestal moved to target position (%d, %d) in %s"),
                    TargetGridRow, TargetGridColumn, *TargetPuzzleArea->GetName());
            }
        }
    }
    else
    {
        // Ÿ���� ������ ���� ������� ���� ������ ã��
        FindOwnerPuzzleArea();
    }
}


bool APedestal::MoveToGridPosition(int32 NewRow, int32 NewColumn)
{
    // ���� ������ Ȯ��
    if (!OwnerPuzzleArea)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot move to grid position - no puzzle area set"));
        return false;
    }

    // �׸��� ��ǥ ��ȿ�� �˻�
    if (!OwnerPuzzleArea->IsValidIndex(NewRow, NewColumn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid grid position: (%d, %d)"), NewRow, NewColumn);
        return false;
    }

    // ���� �� ���� ����
    ClearPreviousCell();

    // �� ��ġ ���
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NewRow, NewColumn);

    // ���� Z ���� ����
    NewLocation.Z = GetActorLocation().Z;

    // ��ġ ����
    SetActorLocation(NewLocation);

    // �׸��� ��ġ ������Ʈ
    GridRow = NewRow;
    GridColumn = NewColumn;

    // Ÿ�� �׸��� ��ǥ�� ������Ʈ
    TargetGridRow = NewRow;
    TargetGridColumn = NewColumn;

    // ���� ����� ���
    bool bRegistered = OwnerPuzzleArea->RegisterPedestal(this, NewRow, NewColumn);

    return bRegistered;
}

void APedestal::ClearPreviousCell()
{
    if (OwnerPuzzleArea && GridRow >= 0 && GridColumn >= 0)
    {
        int32 Index = OwnerPuzzleArea->GetIndexFrom2DCoord(GridRow, GridColumn);
        if (Index >= 0 && Index < OwnerPuzzleArea->Grid.Num())
        {
            // �� ��ħ�밡 ���� ���� �����ϰ� �ִ� ��쿡�� �ʱ�ȭ
            if (OwnerPuzzleArea->Grid[Index].PlacedActor == this)
            {
                OwnerPuzzleArea->Grid[Index].PlacedActor = nullptr;
                OwnerPuzzleArea->Grid[Index].State = ECellState::Walkable;

                // �α� ��� (����׿�)
                UE_LOG(LogTemp, Verbose, TEXT("Cleared previous cell at (%d, %d)"),
                    GridRow, GridColumn);
            }
        }
    }
}

#if WITH_EDITOR
void APedestal::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    // �̵��� �Ϸ�� ��쿡�� ó��
    if (bFinished)
    {
        // �켱 Ÿ�� ���� ������ Ȯ��
        if (TargetPuzzleArea)
        {
            OwnerPuzzleArea = TargetPuzzleArea;
        }

        // ���� ����� ������ ã��
        if (!OwnerPuzzleArea)
        {
            FindOwnerPuzzleArea();
        }

        if (OwnerPuzzleArea)
        {
            // ���� ��ġ���� �׸��� ��ǥ ã��
            int32 NewRow, NewColumn;
            if (OwnerPuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), NewRow, NewColumn))
            {
                // ���� ���� �ٸ� ��쿡�� ó��
                if (NewRow != GridRow || NewColumn != GridColumn)
                {
                    // ���� �� ���� ����
                    ClearPreviousCell();

                    // �� �߾����� �̵�
                    FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NewRow, NewColumn);
                    CellCenter.Z = GetActorLocation().Z; // Z ���� ����
                    SetActorLocation(CellCenter);

                    // �׸��� ��ġ ������Ʈ
                    GridRow = NewRow;
                    GridColumn = NewColumn;

                    // Ÿ�� �׸��� ��ǥ�� ������Ʈ
                    TargetGridRow = NewRow;
                    TargetGridColumn = NewColumn;

                    // ���� ����� ���
                    OwnerPuzzleArea->RegisterPedestal(this, NewRow, NewColumn);

                    UE_LOG(LogTemp, Display, TEXT("Pedestal snapped to grid at (%d, %d)"),
                        NewRow, NewColumn);
                }
            }
        }
    }
}
#endif



void APedestal::FindOwnerPuzzleArea()
{
    // ��� ���� ������ ã��
    TArray<AActor*> FoundAreas;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleArea::StaticClass(), FoundAreas);

    UE_LOG(LogTemp, Warning, TEXT("Found %d PuzzleAreas in level"), FoundAreas.Num());

    for (AActor* Area : FoundAreas)
    {
        APuzzleArea* PuzzleArea = Cast<APuzzleArea>(Area);
        if (PuzzleArea)
        {
            // �� ��ħ�밡 �ش� ���� ������ ���� �ִ��� Ȯ��
            int32 Row, Column;
            if (PuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), Row, Column))
            {
                UE_LOG(LogTemp, Warning, TEXT("Pedestal is located within PuzzleArea %s at grid position (%d, %d)"),
                    *PuzzleArea->GetName(), Row, Column);

                // ���� ������ ���� �� �׸��� ��ġ ����
                OwnerPuzzleArea = PuzzleArea;
                GridRow = Row;
                GridColumn = Column;

                // �� �߾����� ��ġ ���� (�����Ϳ�����)
                if (GetWorld() && GetWorld()->IsEditorWorld())
                {
                    FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(Row, Column);
                    CellCenter.Z = GetActorLocation().Z; // Z ��ǥ ����
                    SetActorLocation(CellCenter);

                    UE_LOG(LogTemp, Display, TEXT("Auto-centered at grid position (%d,%d): %s"),
                        Row, Column, *CellCenter.ToString());
                }

                // ���� ����� ��ħ�� ���
                OwnerPuzzleArea->RegisterPedestal(this, Row, Column);

                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Pedestal is not within the bounds of PuzzleArea %s"),
                    *PuzzleArea->GetName());
            }
        }
    }

    UE_LOG(LogTemp, Error, TEXT("WARNING: Pedestal could not find any suitable PuzzleArea. Push functionality will be limited."));
}


void APedestal::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
}

bool APedestal::Push(FVector Direction)
{
    // ����� �α� �߰�
    UE_LOG(LogTemp, Warning, TEXT("Pedestal::Push called with direction: %s"), *Direction.ToString());

    // �߿�: ���� ���� �߰� - �÷��̾ �ٶ󺸴� �������� �и�����
    Direction = -Direction;
    UE_LOG(LogTemp, Warning, TEXT("Adjusted push direction (player view): %s"), *Direction.ToString());

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

    // ���� ���͸� �׸��� �������� ��ȯ (�÷��̾� �ٶ󺸴� �������� �̵�)
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

    UE_LOG(LogTemp, Warning, TEXT("Pushing in grid direction: %d"), (int32)GridDirection);

    // ���� �׸��� ��ġ
    int32 CurrentRow = GridRow;
    int32 CurrentColumn = GridColumn;

    // �̵��� ��ġ ��� (��Ȯ�� �� ĭ)
    int32 TargetRow = CurrentRow;
    int32 TargetColumn = CurrentColumn;

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

    UE_LOG(LogTemp, Warning, TEXT("Current position: (%d, %d), Target position: (%d, %d)"),
        CurrentRow, CurrentColumn, TargetRow, TargetColumn);

    // ��ȿ�� ��ġ���� Ȯ��
    if (!OwnerPuzzleArea->IsValidIndex(TargetRow, TargetColumn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - target position is invalid"));
        return false;
    }

    // �̵� �������� Ȯ�� (�̵� �Ұ� �����̳� �ٸ� ��ħ�밡 �ִ���)
    ECellState TargetState = OwnerPuzzleArea->GetCellState(TargetRow, TargetColumn);
    if (TargetState == ECellState::Unwalkable || TargetState == ECellState::Occupied)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - target cell is unwalkable or occupied"));
        return false;
    }

    // ���� �� ���� ���� (�߰��� �κ�)
    ClearPreviousCell();

    // �� ��ġ�� ��Ȯ�� �̵� (�׸��� ��ġ�� ����)
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(TargetRow, TargetColumn);
    UE_LOG(LogTemp, Warning, TEXT("Moving pedestal to: %s"), *NewLocation.ToString());
    SetActorLocation(NewLocation);

    // �׸��� ��ġ ������Ʈ
    GridRow = TargetRow;
    GridColumn = TargetColumn;

    // Ÿ�� �׸��� ��ǥ�� ������Ʈ
    TargetGridRow = TargetRow;
    TargetGridColumn = TargetColumn;

    // ���� ����� ���
    OwnerPuzzleArea->RegisterPedestal(this, TargetRow, TargetColumn);

    UE_LOG(LogTemp, Warning, TEXT("Pedestal push completed successfully"));
    return true;
}

void APedestal::SnapToGridCenter()
{
    if (!OwnerPuzzleArea)
    {
        FindOwnerPuzzleArea();
        if (!OwnerPuzzleArea)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot snap to grid - no owner puzzle area found"));
            return;
        }
    }

    // ���� ��ġ���� ���� ����� �׸��� �� ã��
    int32 NearestRow, NearestColumn;
    if (OwnerPuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), NearestRow, NearestColumn))
    {
        // ã�� ���� �߾� ��ġ ���
        FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NearestRow, NearestColumn);

        // Z ��ġ�� ����
        CellCenter.Z = GetActorLocation().Z;

        // ��ġ ����
        SetActorLocation(CellCenter);

        // �׸��� ��ġ ������Ʈ
        GridRow = NearestRow;
        GridColumn = NearestColumn;

        // ���� ����� ���
        OwnerPuzzleArea->RegisterPedestal(this, NearestRow, NearestColumn);
    }
}






void APedestal::Rotate(float Degrees)
{
    // ����� �α� �߰�
    UE_LOG(LogTemp, Warning, TEXT("Pedestal::Rotate called with degrees: %f"), Degrees);

    // ���� ȸ���� ���� �߰�
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += Degrees;
    UE_LOG(LogTemp, Warning, TEXT("Rotating from %f to %f"), GetActorRotation().Yaw, NewRotation.Yaw);
    SetActorRotation(NewRotation);

    // ��ġ�� �������� ���� ȸ��
    if (PlacedObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("Also rotating placed object: %s"), *PlacedObject->GetName());
        PlacedObject->SetActorRotation(NewRotation);
    }

    UE_LOG(LogTemp, Warning, TEXT("Pedestal rotation completed successfully"));
}

bool APedestal::PlaceObject(AActor* Object)
{
    // �̹� �ٸ� �������� ��ġ�Ǿ� ������ ����
    if (CurrentState == EPedestalState::Occupied && PlacedObject != Object)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot place object - already occupied"));
        return false;
    }

    // ������ ��ġ
    if (Object)
    {
        // AttachmentPoint�� ������ ����
        if (!AttachmentPoint)
        {
            UE_LOG(LogTemp, Warning, TEXT("Creating AttachmentPoint at runtime"));
            AttachmentPoint = NewObject<USceneComponent>(this, TEXT("AttachmentPoint"));
            AttachmentPoint->RegisterComponent();
            AttachmentPoint->SetupAttachment(RootComponent);
            AttachmentPoint->SetRelativeLocation(FVector(0, 0, MeshComponent->Bounds.BoxExtent.Z)); // �޽� ��ܿ� ��ġ
        }

        // �������� AttachmentPoint�� ��ġ
        Object->AttachToComponent(AttachmentPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        // ���� ������ ��Ȯ�� ��ġ (������ ����)
        Object->SetActorRelativeLocation(FVector::ZeroVector);

        // ��ħ��� ���� ȸ�� ����
        Object->SetActorRotation(GetActorRotation());

        UE_LOG(LogTemp, Display, TEXT("Object %s placed on pedestal at attachment point"), *Object->GetName());

        // ���� ������Ʈ
        PlacedObject = Object;
        CurrentState = EPedestalState::Occupied;
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot place null object"));
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

void APedestal::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // �÷��̾� ĳ�������� Ȯ��
    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(OtherActor);
    if (Character)
    {
        UE_LOG(LogTemp, Display, TEXT("Player entered interaction sphere of pedestal: %s"), *GetName());

        // ĳ������ ��ȣ�ۿ� ���� ������Ʈ
        Character->bIsLookingAtInteractable = true;
        Character->CurrentInteractableActor = this;
        Character->CurrentInteractionText = InteractionText;
        Character->CurrentInteractionType = InteractionType;
    }
}

void APedestal::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // �÷��̾� ĳ�������� Ȯ��
    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(OtherActor);
    if (Character && Character->CurrentInteractableActor == this)
    {
        UE_LOG(LogTemp, Display, TEXT("Player exited interaction sphere of pedestal: %s"), *GetName());

        // ĳ������ ��ȣ�ۿ� ���� �ʱ�ȭ
        Character->bIsLookingAtInteractable = false;
        Character->CurrentInteractableActor = nullptr;
        Character->CurrentInteractionText = FString();
    }
}