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

    // ���� ������ ã��
    FindOwnerPuzzleArea();
}

void APedestal::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // �����Ϳ��� ��ġ ���� �� �ڵ����� ȣ���
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
                    UE_LOG(LogTemp, Display, TEXT("Pedestal found PuzzleArea: %s, GridPosition: (%d, %d)"),
                        *PuzzleArea->GetName(), Row, Column);
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
            UE_LOG(LogTemp, Warning, TEXT("Push interaction from F key"));
            FVector Direction = GetActorLocation() - Interactor->GetActorLocation();
            Direction.Z = 0;
            Direction.Normalize();
            Push(Direction);
        }
        break;

    case EInteractionType::Rotate:
        // ȸ�� ��ȣ�ۿ�
        UE_LOG(LogTemp, Warning, TEXT("Rotate interaction from F key"));
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
    // ����� �α� �߰�
    UE_LOG(LogTemp, Warning, TEXT("Pedestal::Push called with direction: %s"), *Direction.ToString());

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

    UE_LOG(LogTemp, Warning, TEXT("Pushing in grid direction: %d"), (int32)GridDirection);

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

    UE_LOG(LogTemp, Warning, TEXT("Current position: (%d, %d), Target position: (%d, %d)"),
        GridRow, GridColumn, TargetRow, TargetColumn);

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
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - cell already occupied by %s"), *ActorAtTarget->GetName());
        return false;
    }

    // ���� ��ġ���� ��ħ�� ����
    OwnerPuzzleArea->SetCellState(GridRow, GridColumn, ECellState::Walkable);

    // �� ��ġ�� �̵�
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(TargetRow, TargetColumn);
    UE_LOG(LogTemp, Warning, TEXT("Moving pedestal to: %s"), *NewLocation.ToString());
    SetActorLocation(NewLocation);

    // �׸��� ��ġ ������Ʈ
    GridRow = TargetRow;
    GridColumn = TargetColumn;

    // ���� ����� ���
    OwnerPuzzleArea->PlaceActorAtCell(this, TargetRow, TargetColumn);

    UE_LOG(LogTemp, Warning, TEXT("Pedestal push completed successfully"));
    return true;
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