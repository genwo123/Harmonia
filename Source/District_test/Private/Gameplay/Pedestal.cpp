// Pedestal.cpp
#include "Gameplay/Pedestal.h"
#include "Kismet/GameplayStatics.h"
#include "Character/HamoniaCharacter.h"

APedestal::APedestal()
{
    PrimaryActorTick.bCanEverTick = false;

    // 기본값 설정
    CurrentState = EPedestalState::Empty;
    PlacedObject = nullptr;
    OwnerPuzzleArea = nullptr;
    GridRow = -1;
    GridColumn = -1;

    // 메시 컴포넌트 설정
    if (!MeshComponent)
    {
        MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
        RootComponent = MeshComponent;
    }

    // 콜리전 설정 - 중요한 부분
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetGenerateOverlapEvents(true);

    // 상호작용 영역 추가 (PickupActor처럼) - 블루프린트에서 위치 조정 가능
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
    InteractionSphere->SetGenerateOverlapEvents(true);
    InteractionSphere->SetRelativeLocation(FVector(0, 0, 50.0f)); // 기본 위치 설정

    // 오버랩 이벤트 바인딩
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APedestal::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APedestal::OnInteractionSphereEndOverlap);

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

void APedestal::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 에디터에서 위치 변경 시 자동으로 호출됨
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
    // 상호작용 타입에 따라 다른 동작 수행
    switch (InteractionType)
    {
    case EInteractionType::Push:
        // 밀기 상호작용 (플레이어 방향으로 밀기)
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
        // 회전 상호작용
        UE_LOG(LogTemp, Warning, TEXT("Rotate interaction from F key"));
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
    // 디버그 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("Pedestal::Push called with direction: %s"), *Direction.ToString());

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

    UE_LOG(LogTemp, Warning, TEXT("Pushing in grid direction: %d"), (int32)GridDirection);

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

    UE_LOG(LogTemp, Warning, TEXT("Current position: (%d, %d), Target position: (%d, %d)"),
        GridRow, GridColumn, TargetRow, TargetColumn);

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
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - cell already occupied by %s"), *ActorAtTarget->GetName());
        return false;
    }

    // 현재 위치에서 받침대 제거
    OwnerPuzzleArea->SetCellState(GridRow, GridColumn, ECellState::Walkable);

    // 새 위치로 이동
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(TargetRow, TargetColumn);
    UE_LOG(LogTemp, Warning, TEXT("Moving pedestal to: %s"), *NewLocation.ToString());
    SetActorLocation(NewLocation);

    // 그리드 위치 업데이트
    GridRow = TargetRow;
    GridColumn = TargetColumn;

    // 퍼즐 에리어에 등록
    OwnerPuzzleArea->PlaceActorAtCell(this, TargetRow, TargetColumn);

    UE_LOG(LogTemp, Warning, TEXT("Pedestal push completed successfully"));
    return true;
}

void APedestal::Rotate(float Degrees)
{
    // 디버그 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("Pedestal::Rotate called with degrees: %f"), Degrees);

    // 현재 회전에 각도 추가
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += Degrees;
    UE_LOG(LogTemp, Warning, TEXT("Rotating from %f to %f"), GetActorRotation().Yaw, NewRotation.Yaw);
    SetActorRotation(NewRotation);

    // 설치된 오브제도 같이 회전
    if (PlacedObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("Also rotating placed object: %s"), *PlacedObject->GetName());
        PlacedObject->SetActorRotation(NewRotation);
    }

    UE_LOG(LogTemp, Warning, TEXT("Pedestal rotation completed successfully"));
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

void APedestal::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // 플레이어 캐릭터인지 확인
    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(OtherActor);
    if (Character)
    {
        UE_LOG(LogTemp, Display, TEXT("Player entered interaction sphere of pedestal: %s"), *GetName());

        // 캐릭터의 상호작용 상태 업데이트
        Character->bIsLookingAtInteractable = true;
        Character->CurrentInteractableActor = this;
        Character->CurrentInteractionText = InteractionText;
        Character->CurrentInteractionType = InteractionType;
    }
}

void APedestal::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // 플레이어 캐릭터인지 확인
    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(OtherActor);
    if (Character && Character->CurrentInteractableActor == this)
    {
        UE_LOG(LogTemp, Display, TEXT("Player exited interaction sphere of pedestal: %s"), *GetName());

        // 캐릭터의 상호작용 상태 초기화
        Character->bIsLookingAtInteractable = false;
        Character->CurrentInteractableActor = nullptr;
        Character->CurrentInteractionText = FString();
    }
}