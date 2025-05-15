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

    // 타겟 그리드 좌표 초기화
    TargetGridRow = 0;
    TargetGridColumn = 0;
    TargetPuzzleArea = nullptr;

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

    //부착 위치 설정
    AttachmentPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttachmentPoint"));
    AttachmentPoint->SetupAttachment(RootComponent);
    AttachmentPoint->SetRelativeLocation(FVector(0, 0, 80.0f));

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

    // 타겟 퍼즐 에리어가 지정되었으면 사용
    if (TargetPuzzleArea)
    {
        OwnerPuzzleArea = TargetPuzzleArea;

        // 타겟 그리드 좌표로 이동
        if (MoveToGridPosition(TargetGridRow, TargetGridColumn))
        {
            UE_LOG(LogTemp, Display, TEXT("Pedestal moved to target position (%d, %d) in %s"),
                TargetGridRow, TargetGridColumn, *TargetPuzzleArea->GetName());
        }
    }
    else
    {
        // 타겟이 없으면 기존 방식으로 퍼즐 에리어 찾기
        FindOwnerPuzzleArea();
    }
}

void APedestal::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 타겟 퍼즐 에리어가 지정되었으면 사용
    if (TargetPuzzleArea)
    {
        OwnerPuzzleArea = TargetPuzzleArea;

        // 타겟 그리드 좌표로 이동
        if (MoveToGridPosition(TargetGridRow, TargetGridColumn))
        {
            // 에디터에서만 로그 출력
            if (GWorld && GWorld->IsEditorWorld())
            {
                UE_LOG(LogTemp, Display, TEXT("Pedestal moved to target position (%d, %d) in %s"),
                    TargetGridRow, TargetGridColumn, *TargetPuzzleArea->GetName());
            }
        }
    }
    else
    {
        // 타겟이 없으면 기존 방식으로 퍼즐 에리어 찾기
        FindOwnerPuzzleArea();
    }
}


bool APedestal::MoveToGridPosition(int32 NewRow, int32 NewColumn)
{
    // 퍼즐 에리어 확인
    if (!OwnerPuzzleArea)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot move to grid position - no puzzle area set"));
        return false;
    }

    // 그리드 좌표 유효성 검사
    if (!OwnerPuzzleArea->IsValidIndex(NewRow, NewColumn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid grid position: (%d, %d)"), NewRow, NewColumn);
        return false;
    }

    // 이전 셀 참조 제거
    ClearPreviousCell();

    // 새 위치 계산
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NewRow, NewColumn);

    // 현재 Z 높이 유지
    NewLocation.Z = GetActorLocation().Z;

    // 위치 설정
    SetActorLocation(NewLocation);

    // 그리드 위치 업데이트
    GridRow = NewRow;
    GridColumn = NewColumn;

    // 타겟 그리드 좌표도 업데이트
    TargetGridRow = NewRow;
    TargetGridColumn = NewColumn;

    // 퍼즐 에리어에 등록
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
            // 이 받침대가 현재 셀을 점유하고 있는 경우에만 초기화
            if (OwnerPuzzleArea->Grid[Index].PlacedActor == this)
            {
                OwnerPuzzleArea->Grid[Index].PlacedActor = nullptr;
                OwnerPuzzleArea->Grid[Index].State = ECellState::Walkable;

                // 로그 출력 (디버그용)
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

    // 이동이 완료된 경우에만 처리
    if (bFinished)
    {
        // 우선 타겟 퍼즐 에리어 확인
        if (TargetPuzzleArea)
        {
            OwnerPuzzleArea = TargetPuzzleArea;
        }

        // 퍼즐 에리어가 없으면 찾기
        if (!OwnerPuzzleArea)
        {
            FindOwnerPuzzleArea();
        }

        if (OwnerPuzzleArea)
        {
            // 현재 위치에서 그리드 좌표 찾기
            int32 NewRow, NewColumn;
            if (OwnerPuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), NewRow, NewColumn))
            {
                // 이전 셀과 다른 경우에만 처리
                if (NewRow != GridRow || NewColumn != GridColumn)
                {
                    // 이전 셀 참조 제거
                    ClearPreviousCell();

                    // 셀 중앙으로 이동
                    FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NewRow, NewColumn);
                    CellCenter.Z = GetActorLocation().Z; // Z 높이 유지
                    SetActorLocation(CellCenter);

                    // 그리드 위치 업데이트
                    GridRow = NewRow;
                    GridColumn = NewColumn;

                    // 타겟 그리드 좌표도 업데이트
                    TargetGridRow = NewRow;
                    TargetGridColumn = NewColumn;

                    // 퍼즐 에리어에 등록
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
    // 모든 퍼즐 에리어 찾기
    TArray<AActor*> FoundAreas;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleArea::StaticClass(), FoundAreas);

    UE_LOG(LogTemp, Warning, TEXT("Found %d PuzzleAreas in level"), FoundAreas.Num());

    for (AActor* Area : FoundAreas)
    {
        APuzzleArea* PuzzleArea = Cast<APuzzleArea>(Area);
        if (PuzzleArea)
        {
            // 이 받침대가 해당 퍼즐 에리어 내에 있는지 확인
            int32 Row, Column;
            if (PuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), Row, Column))
            {
                UE_LOG(LogTemp, Warning, TEXT("Pedestal is located within PuzzleArea %s at grid position (%d, %d)"),
                    *PuzzleArea->GetName(), Row, Column);

                // 퍼즐 에리어 설정 및 그리드 위치 저장
                OwnerPuzzleArea = PuzzleArea;
                GridRow = Row;
                GridColumn = Column;

                // 셀 중앙으로 위치 조정 (에디터에서만)
                if (GetWorld() && GetWorld()->IsEditorWorld())
                {
                    FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(Row, Column);
                    CellCenter.Z = GetActorLocation().Z; // Z 좌표 유지
                    SetActorLocation(CellCenter);

                    UE_LOG(LogTemp, Display, TEXT("Auto-centered at grid position (%d,%d): %s"),
                        Row, Column, *CellCenter.ToString());
                }

                // 퍼즐 에리어에 받침대 등록
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
    // 디버그 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("Pedestal::Push called with direction: %s"), *Direction.ToString());

    // 중요: 방향 반전 추가 - 플레이어가 바라보는 방향으로 밀리도록
    Direction = -Direction;
    UE_LOG(LogTemp, Warning, TEXT("Adjusted push direction (player view): %s"), *Direction.ToString());

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

    // 방향 벡터를 그리드 방향으로 변환 (플레이어 바라보는 방향으로 이동)
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

    // 현재 그리드 위치
    int32 CurrentRow = GridRow;
    int32 CurrentColumn = GridColumn;

    // 이동할 위치 계산 (정확히 한 칸)
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

    // 유효한 위치인지 확인
    if (!OwnerPuzzleArea->IsValidIndex(TargetRow, TargetColumn))
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - target position is invalid"));
        return false;
    }

    // 이동 가능한지 확인 (이동 불가 영역이나 다른 받침대가 있는지)
    ECellState TargetState = OwnerPuzzleArea->GetCellState(TargetRow, TargetColumn);
    if (TargetState == ECellState::Unwalkable || TargetState == ECellState::Occupied)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot push - target cell is unwalkable or occupied"));
        return false;
    }

    // 이전 셀 참조 제거 (추가된 부분)
    ClearPreviousCell();

    // 새 위치로 정확히 이동 (그리드 위치에 맞춤)
    FVector NewLocation = OwnerPuzzleArea->GetWorldLocationFromGridIndex(TargetRow, TargetColumn);
    UE_LOG(LogTemp, Warning, TEXT("Moving pedestal to: %s"), *NewLocation.ToString());
    SetActorLocation(NewLocation);

    // 그리드 위치 업데이트
    GridRow = TargetRow;
    GridColumn = TargetColumn;

    // 타겟 그리드 좌표도 업데이트
    TargetGridRow = TargetRow;
    TargetGridColumn = TargetColumn;

    // 퍼즐 에리어에 등록
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

    // 현재 위치에서 가장 가까운 그리드 셀 찾기
    int32 NearestRow, NearestColumn;
    if (OwnerPuzzleArea->GetGridIndexFromWorldLocation(GetActorLocation(), NearestRow, NearestColumn))
    {
        // 찾은 셀의 중앙 위치 계산
        FVector CellCenter = OwnerPuzzleArea->GetWorldLocationFromGridIndex(NearestRow, NearestColumn);

        // Z 위치는 유지
        CellCenter.Z = GetActorLocation().Z;

        // 위치 설정
        SetActorLocation(CellCenter);

        // 그리드 위치 업데이트
        GridRow = NearestRow;
        GridColumn = NearestColumn;

        // 퍼즐 에리어에 등록
        OwnerPuzzleArea->RegisterPedestal(this, NearestRow, NearestColumn);
    }
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
        UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot place object - already occupied"));
        return false;
    }

    // 오브제 설치
    if (Object)
    {
        // AttachmentPoint가 없으면 생성
        if (!AttachmentPoint)
        {
            UE_LOG(LogTemp, Warning, TEXT("Creating AttachmentPoint at runtime"));
            AttachmentPoint = NewObject<USceneComponent>(this, TEXT("AttachmentPoint"));
            AttachmentPoint->RegisterComponent();
            AttachmentPoint->SetupAttachment(RootComponent);
            AttachmentPoint->SetRelativeLocation(FVector(0, 0, MeshComponent->Bounds.BoxExtent.Z)); // 메시 상단에 위치
        }

        // 오브제를 AttachmentPoint에 배치
        Object->AttachToComponent(AttachmentPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        // 부착 지점에 정확히 배치 (오프셋 제거)
        Object->SetActorRelativeLocation(FVector::ZeroVector);

        // 받침대와 같은 회전 적용
        Object->SetActorRotation(GetActorRotation());

        UE_LOG(LogTemp, Display, TEXT("Object %s placed on pedestal at attachment point"), *Object->GetName());

        // 상태 업데이트
        PlacedObject = Object;
        CurrentState = EPedestalState::Occupied;
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Pedestal: Cannot place null object"));
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