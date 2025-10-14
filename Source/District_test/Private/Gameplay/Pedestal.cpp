// Pedestal.cpp
#include "Gameplay/Pedestal.h"
#include "Kismet/GameplayStatics.h"
#include "Character/HamoniaCharacter.h"
#include "Gameplay/PickupActor.h"


APedestal::APedestal()
{
    PrimaryActorTick.bCanEverTick = false;

    CurrentState = EPedestalState::Empty;
    PlacedObject = nullptr;
    OwnerPuzzleArea = nullptr;
    GridRow = -1;
    GridColumn = -1;
    TargetGridRow = 0;
    TargetGridColumn = 0;
    TargetPuzzleArea = nullptr;

    if (!MeshComponent)
    {
        MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
        RootComponent = MeshComponent;
    }

    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetGenerateOverlapEvents(true);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
    InteractionSphere->SetGenerateOverlapEvents(true);
    InteractionSphere->SetRelativeLocation(FVector(0, 0, 50.0f));

    AttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttachmentPoint"));
    AttachmentPoint->SetupAttachment(RootComponent);
    AttachmentPoint->SetRelativeLocation(FVector(0, 0, 80.0f));
    AttachmentPoint->SetMobility(EComponentMobility::Movable);

    AttachedActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("AttachedActorComponent"));
    AttachedActorComponent->SetupAttachment(AttachmentPoint);

#if WITH_EDITORONLY_DATA
    AttachmentPoint->bVisualizeComponent = true;
#endif

    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APedestal::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APedestal::OnInteractionSphereEndOverlap);

    InteractionText = "Interact with Pedestal";
    InteractionType = EInteractionType::Default;

    bAutoSnapToGrid = true;
}

void APedestal::BeginPlay()
{
    Super::BeginPlay();

    if (AttachedActorComponent)
    {
        AttachedActorComponent->SetVisibility(true);
        AttachedActorComponent->SetHiddenInGame(false);

        if (AActor* ChildActor = AttachedActorComponent->GetChildActor())
        {
            ChildActor->SetActorHiddenInGame(false);

            if (APickupActor* Pickup = Cast<APickupActor>(ChildActor))
            {
                if (Pickup->MeshComponent)
                {
                    Pickup->MeshComponent->SetVisibility(true);
                    Pickup->MeshComponent->SetHiddenInGame(false);
                    Pickup->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    Pickup->MeshComponent->SetSimulatePhysics(false);
                }
                if (Pickup->InteractionSphere)
                {
                    Pickup->InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    Pickup->InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
                }
            }
        }
    }

    // 그리드 시스템 사용하는 경우만
    if (bUseGridSystem)
    {
        if (TargetPuzzleArea)
        {
            OwnerPuzzleArea = TargetPuzzleArea;
            MoveToGridPosition(TargetGridRow, TargetGridColumn);
        }
        else
        {
            FindOwnerPuzzleArea();
        }
    }
}



void APedestal::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (bApplyInEditor)
    {
        UpdateAttachedActor();
    }

    if (!bAutoSnapToGrid || !bUseGridSystem)
        return;

    if (TargetPuzzleArea)
    {
        OwnerPuzzleArea = TargetPuzzleArea;
        MoveToGridPosition(TargetGridRow, TargetGridColumn);
    }
    else
    {
        FindOwnerPuzzleArea();
    }
}


bool APedestal::MoveToGridPosition(int32 NewRow, int32 NewColumn)
{
    if (!bUseGridSystem)
        return false;

    if (!OwnerPuzzleArea)
    {
        return false;
    }

    if (!OwnerPuzzleArea->IsValidIndex(NewRow, NewColumn))
    {
        return false;
    }

    ClearPreviousCell();

    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NewRow, NewColumn);
    NewLocation.Z = GetActorLocation().Z;
    SetActorLocation(NewLocation);

    GridRow = NewRow;
    GridColumn = NewColumn;
    TargetGridRow = NewRow;
    TargetGridColumn = NewColumn;

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
            if (OwnerPuzzleArea->Grid[Index].PlacedActor == this)
            {
                OwnerPuzzleArea->Grid[Index].PlacedActor = nullptr;
                OwnerPuzzleArea->Grid[Index].State = ECellState::Walkable;
            }
        }
    }
}

#if WITH_EDITOR
void APedestal::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    if (bFinished)
    {
        if (TargetPuzzleArea)
        {
            OwnerPuzzleArea = TargetPuzzleArea;
        }

        if (!OwnerPuzzleArea)
        {
            FindOwnerPuzzleArea();
        }

        if (OwnerPuzzleArea)
        {
            int32 NewRow, NewColumn;
            if (OwnerPuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), NewRow, NewColumn))
            {
                if (NewRow != GridRow || NewColumn != GridColumn)
                {
                    ClearPreviousCell();

                    FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NewRow, NewColumn);
                    CellCenter.Z = GetActorLocation().Z;
                    SetActorLocation(CellCenter);

                    GridRow = NewRow;
                    GridColumn = NewColumn;
                    TargetGridRow = NewRow;
                    TargetGridColumn = NewColumn;

                    OwnerPuzzleArea->RegisterPedestal(this, NewRow, NewColumn);
                }
            }
        }
    }
}

void APedestal::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr)
        ? PropertyChangedEvent.Property->GetFName()
        : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(APedestal, PreAttachedActorClass) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(APedestal, AttachmentOffset) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(APedestal, AttachmentRotation) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(APedestal, AttachmentScale))
    {
        UpdateAttachedActor();
    }
}
#endif

void APedestal::FindOwnerPuzzleArea()
{
    TArray<AActor*> FoundAreas;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleArea::StaticClass(), FoundAreas);

    if (!bAutoSnapToGrid)
    {
        return;
    }

    for (AActor* Area : FoundAreas)
    {
        APuzzleArea* PuzzleArea = Cast<APuzzleArea>(Area);
        if (PuzzleArea)
        {
            int32 Row, Column;
            if (PuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), Row, Column))
            {
                OwnerPuzzleArea = PuzzleArea;
                GridRow = Row;
                GridColumn = Column;

                if (GetWorld() && GetWorld()->IsEditorWorld())
                {
                    FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(Row, Column);
                    CellCenter.Z = GetActorLocation().Z;
                    SetActorLocation(CellCenter);
                }

                OwnerPuzzleArea->RegisterPedestal(this, Row, Column);
                return;
            }
        }
    }
}

void APedestal::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
}

bool APedestal::Push(FVector Direction)
{
    if (!bUseGridSystem)
        return false;


    Direction = -Direction;
    FVector AdjustedDirection(-Direction.Y, -Direction.X, Direction.Z);
    Direction = AdjustedDirection;

    if (!OwnerPuzzleArea)
    {
        FindOwnerPuzzleArea();
        if (!OwnerPuzzleArea)
        {
            return false;
        }
    }

    EGridDirection GridDirection;
    if (FMath::Abs(Direction.X) > FMath::Abs(Direction.Y))
    {
        GridDirection = Direction.X > 0 ? EGridDirection::East : EGridDirection::West;
    }
    else
    {
        GridDirection = Direction.Y > 0 ? EGridDirection::South : EGridDirection::North;
    }

    int32 CurrentRow = GridRow;
    int32 CurrentColumn = GridColumn;
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

    if (!OwnerPuzzleArea->IsValidIndex(TargetRow, TargetColumn))
    {
        return false;
    }

    ECellState TargetState = OwnerPuzzleArea->GetCellState(TargetRow, TargetColumn);
    if (TargetState == ECellState::Unwalkable || TargetState == ECellState::Occupied)
    {
        return false;
    }

    ClearPreviousCell();

    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(TargetRow, TargetColumn);
    SetActorLocation(NewLocation);

    GridRow = TargetRow;
    GridColumn = TargetColumn;
    TargetGridRow = TargetRow;
    TargetGridColumn = TargetColumn;

    OwnerPuzzleArea->RegisterPedestal(this, TargetRow, TargetColumn);
    return true;
}

void APedestal::SnapToGridCenter()
{
    if (!bUseGridSystem)
        return;

    if (!OwnerPuzzleArea)
    {
        FindOwnerPuzzleArea();
        if (!OwnerPuzzleArea)
        {
            return;
        }
    }

    int32 NearestRow, NearestColumn;
    if (OwnerPuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), NearestRow, NearestColumn))
    {
        FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NearestRow, NearestColumn);
        CellCenter.Z = GetActorLocation().Z;
        SetActorLocation(CellCenter);

        GridRow = NearestRow;
        GridColumn = NearestColumn;

        OwnerPuzzleArea->RegisterPedestal(this, NearestRow, NearestColumn);
    }
}

void APedestal::Rotate(float Degrees)
{
    if (!bCanRotate)
    {
        return;
    }

    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += 45.0f;
    SetActorRotation(NewRotation);

    if (PlacedObject && bObjectFollowsRotation)
    {
        PlacedObject->SetActorRotation(NewRotation);
    }
}

AActor* APedestal::GetAttachedChildActor() const
{
    return AttachedActorComponent ? AttachedActorComponent->GetChildActor() : nullptr;
}

UActorComponent* APedestal::GetAttachedActorComponent(TSubclassOf<UActorComponent> ComponentClass)
{
    AActor* ChildActor = GetAttachedChildActor();
    if (!ChildActor || !ComponentClass)
    {
        return nullptr;
    }

    return ChildActor->GetComponentByClass(ComponentClass);
}


bool APedestal::PlaceObject(AActor* Object)
{
    if (CurrentState == EPedestalState::Occupied && PlacedObject != Object)
    {
        return false;
    }

    if (Object)
    {
        if (!AttachmentPoint)
        {
            AttachmentPoint = NewObject<USceneComponent>(this, TEXT("AttachmentPoint"));
            AttachmentPoint->RegisterComponent();
            AttachmentPoint->SetupAttachment(RootComponent);
            AttachmentPoint->SetRelativeLocation(FVector(0, 0, MeshComponent->Bounds.BoxExtent.Z));
        }

        Object->AttachToComponent(AttachmentPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        Object->SetActorRelativeLocation(FVector::ZeroVector);

        if (!bObjectFollowsRotation)
        {
            Object->SetActorRelativeRotation(FRotator::ZeroRotator);
        }

        PlacedObject = Object;
        CurrentState = EPedestalState::Occupied;
        return true;
    }

    return false;
}

AActor* APedestal::RemoveObject()
{
    if (CurrentState != EPedestalState::Occupied || !PlacedObject)
    {
        return nullptr;
    }

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

void APedestal::UpdateAttachedActor()
{
    if (!AttachedActorComponent)
        return;

    if (PreAttachedActorClass)
    {
        TSubclassOf<AActor> ActorClass = PreAttachedActorClass;
        AttachedActorComponent->SetChildActorClass(ActorClass);
        // 트랜스폼 초기화 제거!
    }
    else
    {
        AttachedActorComponent->SetChildActorClass(nullptr);
    }
}

void APedestal::RotateAttachment(float Degrees)
{
    if (!AttachmentPoint)
        return;

    FRotator CurrentRotation = AttachmentPoint->GetRelativeRotation();
    CurrentRotation.Yaw += Degrees;
    AttachmentPoint->SetRelativeRotation(CurrentRotation);

    if (AttachedActorComponent)
    {
        AttachmentRotation = AttachedActorComponent->GetRelativeRotation();
    }
}

float APedestal::GetAttachmentRotation() const
{
    if (!AttachmentPoint)
        return 0.0f;

    return AttachmentPoint->GetRelativeRotation().Yaw;
}

void APedestal::ClearAttachment()
{
    PreAttachedActorClass = nullptr;
    UpdateAttachedActor();
}

APickupActor* APedestal::DetachAttachedActor()
{
    if (!AttachedActorComponent || !AttachedActorComponent->GetChildActor())
        return nullptr;

    AActor* ChildActor = AttachedActorComponent->GetChildActor();
    FVector WorldLocation = ChildActor->GetActorLocation();
    FRotator WorldRotation = ChildActor->GetActorRotation();
    FVector WorldScale = ChildActor->GetActorScale3D();

    TSubclassOf<APickupActor> PickupClass = PreAttachedActorClass;

    AttachedActorComponent->SetChildActorClass(nullptr);
    PreAttachedActorClass = nullptr;

    if (GetWorld() && PickupClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APickupActor* NewPickup = GetWorld()->SpawnActor<APickupActor>(PickupClass, WorldLocation, WorldRotation, SpawnParams);
        
        if (NewPickup)
        {
            NewPickup->SetActorScale3D(WorldScale);
            NewPickup->MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            NewPickup->MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
            NewPickup->InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            return NewPickup;
        }
    }

    return nullptr;
}

APickupActor* APedestal::GetAttachedActor() const
{
    if (!AttachedActorComponent)
        return nullptr;

    return Cast<APickupActor>(AttachedActorComponent->GetChildActor());
}

bool APedestal::HasAttachedActor() const
{
    return AttachedActorComponent && AttachedActorComponent->GetChildActor() != nullptr;
}

void APedestal::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(OtherActor);
    if (Character)
    {
        Character->bIsLookingAtInteractable = true;
        Character->CurrentInteractableActor = this;
        Character->CurrentInteractionText = InteractionText;
        Character->CurrentInteractionType = InteractionType;
    }
}

void APedestal::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(OtherActor);
    if (Character && Character->CurrentInteractableActor == this)
    {
        Character->bIsLookingAtInteractable = false;
        Character->CurrentInteractableActor = nullptr;
        Character->CurrentInteractionText = FString();
    }
}