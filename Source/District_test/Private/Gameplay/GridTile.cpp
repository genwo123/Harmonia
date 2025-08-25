// GridTile.cpp - Pathfinding Puzzle Complete Version
#include "Gameplay/GridTile.h"
#include "Gameplay/GridMazeManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"


// GridTile.cpp - 수정된 함수들

AGridTile::AGridTile()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create tile mesh (base platform)
    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    TileMesh->SetupAttachment(RootSceneComponent);

    // Load default cube mesh for tile
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TileMesh->SetStaticMesh(CubeMeshAsset.Object);
        TileMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.1f)); // Flat tile
    }

    TileMesh->SetMobility(EComponentMobility::Movable);
    TileMesh->SetSimulatePhysics(false);
    TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TileMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create pillar mesh (interaction object)
    PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
    PillarMesh->SetupAttachment(TileMesh);
    PillarMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylinderMeshAsset.Succeeded())
    {
        PillarMesh->SetStaticMesh(CylinderMeshAsset.Object);
        PillarMesh->SetWorldScale3D(FVector(0.3f, 0.3f, 1.0f));
    }

    PillarMesh->SetMobility(EComponentMobility::Movable);
    PillarMesh->SetSimulatePhysics(false);
    PillarMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PillarMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // ===== 수정된 라이트 설정 =====
    TileLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TileLight"));
    TileLight->SetupAttachment(PillarMesh);
    TileLight->SetRelativeLocation(FVector(0.0f, 0.0f, LightHeight));

    // 라이트 기본 설정 - 켜진 상태로 시작
    TileLight->SetIntensity(1500.0f);               // 0.0f에서 1500.0f로 변경
    TileLight->SetLightColor(FLinearColor::Gray);   // 기본 회색
    TileLight->SetAttenuationRadius(1000.0f);       // 더 넓은 범위
    TileLight->SetSourceRadius(20.0f);
    TileLight->SetMobility(EComponentMobility::Movable);
    TileLight->SetVisibility(true);
    TileLight->SetCastShadows(true);

    // 라이트 설정값들 수정
    ActiveLightIntensity = 1500.0f;     // 3000.0f에서 1500.0f로 변경
    DefaultLightIntensity = 1500.0f;    // 0.0f에서 1500.0f로 변경
    LightRadius = 1000.0f;              // 더 넓게
    LightHeight = 100.0f;

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(PillarMesh);
    InteractionSphere->SetSphereRadius(100.0f);
    InteractionSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionSphere->SetMobility(EComponentMobility::Movable);
    InteractionSphere->SetSimulatePhysics(false);
}

// ====== 수정된 BeginPlay ======
void AGridTile::BeginPlay()
{
    Super::BeginPlay();

    // Mobility 설정 강제
    ForceMobilitySettings();

    // 초기 상태 설정 - 회색 불빛으로 시작
    SetTileState(ETileState::Default);  // UpdateTileVisuals() 대신
}

// ====== 수정된 SetTileState ======
void AGridTile::SetTileState(ETileState NewState)
{
    if (CurrentState != NewState)
    {
        ETileState OldState = CurrentState;
        CurrentState = NewState;

        // 이전 효과 중지
        StopBlinking();
        StopPulsing();

        // Manager 또는 자체 색상 가져오기
        FLinearColor StateColor = GetCurrentStateColor();

        switch (NewState)
        {
        case ETileState::Default:
            // 회색 (대기) - 불빛 켜둠 (끄지 않음!)
            TurnOnLight(StateColor);
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - DEFAULT state (GRAY light ON)"), GridX, GridY);
            break;

        case ETileState::Start:
            // 초록 (준비) - 불빛 켜짐
            TurnOnLight(StateColor);
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - START state (GREEN light ON)"), GridX, GridY);
            break;

        case ETileState::StartPoint:
            // 밝은 초록 (시작점) - 불빛 켜짐 + 깜빡임
            TurnOnLight(StateColor);
            if (bEnableBlinking)
            {
                StartBlinking(0.0f); // 계속 깜빡임
            }
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - START POINT (BRIGHT GREEN + BLINKING)"), GridX, GridY);
            break;

        case ETileState::Correct:
            // 파랑 (정답) - 불빛 켜짐
            TurnOnLight(StateColor);
            PlayCorrectSound();
            OnCorrectInteraction();
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - CORRECT state (BLUE light ON)"), GridX, GridY);
            break;

        case ETileState::Wrong:
            // 빨강 (오답) - 불빛 켜짐 + 깜빡임
            TurnOnLight(StateColor);
            if (bEnableBlinking)
            {
                StartBlinking(3.0f);
            }
            PlayWrongSound();
            OnWrongInteraction();
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - WRONG state (RED light ON + BLINKING)"), GridX, GridY);
            break;

        case ETileState::Goal:
            // 노랑 (목표점) - 미리보기용
            TurnOnLight(StateColor);
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - GOAL state (YELLOW light ON)"), GridX, GridY);
            break;

        case ETileState::Hint:
            // 보라 (힌트) - 미리보기용
            TurnOnLight(StateColor);
            SetLightIntensity(ActiveLightIntensity * 0.7f); // 약간 어둡게
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - HINT state (PURPLE light ON)"), GridX, GridY);
            break;

        default:
            TurnOnLight(StateColor);
            break;
        }

        // 이벤트 브로드캐스트
        OnTileStateChanged.Broadcast(NewState);
        OnStateChanged(OldState, NewState);
    }
}

FLinearColor AGridTile::GetCurrentStateColor() const
{
    // Manager의 색상 설정 우선 사용
    if (OwnerManager)
    {
        switch (CurrentState)
        {
        case ETileState::Default:
            return OwnerManager->WaitingColor;
        case ETileState::Start:
            return OwnerManager->ReadyColor;
        case ETileState::Correct:
            return OwnerManager->CorrectColor;
        case ETileState::Wrong:
            return OwnerManager->WrongColor;
        case ETileState::StartPoint:
            return FLinearColor(0.4f, 1.0f, 0.4f, 1.0f); // 밝은 초록
        case ETileState::Goal:
            return FLinearColor::Yellow;
        case ETileState::Hint:
            return FLinearColor(0.8f, 0.4f, 1.0f, 1.0f); // 보라
        default:
            return OwnerManager->WaitingColor;
        }
    }

    // Manager가 없으면 커스텀 색상 확인
    FLinearColor CustomColor = GetCustomStateColor(CurrentState);
    if (CustomColor != FLinearColor::Black)
    {
        return CustomColor;
    }

    // 기본 색상 매핑
    switch (CurrentState)
    {
    case ETileState::Default:
        return WaitingColor;        // 회색
    case ETileState::Start:
        return ReadyColor;          // 초록
    case ETileState::StartPoint:
        return StartPointColor;     // 밝은 초록
    case ETileState::Correct:
        return CorrectColor;        // 파랑
    case ETileState::Wrong:
        return WrongColor;          // 빨강
    case ETileState::Goal:
        return GoalColor;           // 노랑
    case ETileState::Hint:
        return HintColor;           // 보라
    default:
        return WaitingColor;
    }
}

void AGridTile::UpdateFromManagerColors()
{
    if (OwnerManager && CurrentState != ETileState::Default)
    {
        FLinearColor NewColor = GetCurrentStateColor();
        if (TileLight)
        {
            TileLight->SetLightColor(NewColor);
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) color updated from manager: (%.2f, %.2f, %.2f)"),
                GridX, GridY, NewColor.R, NewColor.G, NewColor.B);
        }
    }
}





void AGridTile::ForceMobilitySettings()
{
    if (TileMesh) TileMesh->SetMobility(EComponentMobility::Movable);
    if (PillarMesh) PillarMesh->SetMobility(EComponentMobility::Movable);
    if (TileLight) TileLight->SetMobility(EComponentMobility::Movable);
    if (InteractionSphere) InteractionSphere->SetMobility(EComponentMobility::Movable);
    if (RootSceneComponent) RootSceneComponent->SetMobility(EComponentMobility::Movable);
}


void AGridTile::SetGridPosition(int32 X, int32 Y)
{
    GridX = X;
    GridY = Y;

    // 디버그용 이름 설정
    FString TileName = FString::Printf(TEXT("GridTile_%d_%d"), X, Y);
    SetActorLabel(*TileName);
}

void AGridTile::SetOwnerManager(AGridMazeManager* Manager)
{
    OwnerManager = Manager;
}

// ====== 불빛 제어 ======
void AGridTile::TurnOnLight(FLinearColor Color)
{
    if (TileLight)
    {
        TileLight->SetLightColor(Color);
        TileLight->SetIntensity(ActiveLightIntensity);
        TileLight->SetVisibility(true);

        UE_LOG(LogTemp, Warning, TEXT("Light turned ON - Color: R:%.1f G:%.1f B:%.1f, Intensity: %.1f"),
            Color.R, Color.G, Color.B, ActiveLightIntensity);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TileLight is NULL!"));
    }

    bIsActivated = true;
    OnLightTurnedOn(Color);
}

void AGridTile::TurnOffLight()
{
    if (TileLight)
    {
        TileLight->SetIntensity(0.0f);
        UE_LOG(LogTemp, Warning, TEXT("Light turned OFF"));
    }

    bIsActivated = false;
    OnLightTurnedOff();
}

void AGridTile::SetLightColor(FLinearColor NewColor)
{
    if (TileLight)
    {
        TileLight->SetLightColor(NewColor);
    }
}

void AGridTile::SetLightIntensity(float NewIntensity)
{
    if (TileLight)
    {
        TileLight->SetIntensity(NewIntensity);
    }
}

// ====== 애니메이션 제어 ======
void AGridTile::StartBlinking(float Duration)
{
    if (!bEnableBlinking) return;

    bIsBlinking = true;

    if (Duration > 0.0f)
    {
        // 지정된 시간 후 멈춤
        GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, [this]()
            {
                StopBlinking();
            }, Duration, false);
    }
    // Duration이 0이면 계속 깜빡임
}

void AGridTile::StopBlinking()
{
    if (bIsBlinking)
    {
        bIsBlinking = false;
        GetWorld()->GetTimerManager().ClearTimer(BlinkTimerHandle);

        // 정상 밝기로 복구
        if (TileLight && bIsActivated)
        {
            TileLight->SetIntensity(ActiveLightIntensity);
        }
    }
}

void AGridTile::StartPulsing(float Duration)
{
    if (!bEnablePulsing) return;

    bIsPulsing = true;

    GetWorld()->GetTimerManager().SetTimer(PulseTimerHandle, [this]()
        {
            StopPulsing();
        }, Duration, false);
}

void AGridTile::StopPulsing()
{
    if (bIsPulsing)
    {
        bIsPulsing = false;
        GetWorld()->GetTimerManager().ClearTimer(PulseTimerHandle);
    }
}

// ====== 외관 제어 ======
void AGridTile::UpdateTileVisuals()
{
    // 현재 상태에 맞는 색상으로 업데이트
    FLinearColor CurrentColor = GetCurrentStateColor();

    if (CurrentState == ETileState::Default)
    {
        TurnOffLight();
    }
    else
    {
        TurnOnLight(CurrentColor);
    }
}

void AGridTile::ResetToDefault()
{
    SetTileState(ETileState::Default);
    StopBlinking();
    StopPulsing();
    bIsActivated = false;
}

// ====== 사운드 제어 ======
void AGridTile::PlayTileSound(USoundBase* Sound)
{
    if (Sound && bPlayInteractionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), SoundVolume);
    }
}

void AGridTile::PlayInteractionSound()
{
    PlayTileSound(InteractionSound);
}

void AGridTile::PlayCorrectSound()
{
    PlayTileSound(CorrectSound);
}

void AGridTile::PlayWrongSound()
{
    PlayTileSound(WrongSound);
}

// ====== 상호작용 제어 ======
void AGridTile::SetCanBeInteracted(bool bCanInteract)
{
    bCanBeInteracted = bCanInteract;
}




// ====== 애니메이션 업데이트 ======
void AGridTile::UpdateBlinking(float DeltaTime)
{
    if (!bIsBlinking || !TileLight) return;

    // 사인파 깜빡임 효과
    float Time = GetWorld()->GetTimeSeconds();
    float BlinkFactor = FMath::Sin(Time * BlinkSpeed * PI) * 0.5f + 0.5f;
    float CurrentIntensity = FMath::Lerp(DefaultLightIntensity, ActiveLightIntensity, BlinkFactor);

    TileLight->SetIntensity(CurrentIntensity);
}

void AGridTile::UpdatePulsing(float DeltaTime)
{
    if (!bIsPulsing) return;

    // 펄스 효과 (필요시 구현)
    float Time = GetWorld()->GetTimeSeconds();
    float PulseFactor = FMath::Sin(Time * PulseSpeed * PI) * 0.3f + 0.7f;

    // 추가 펄스 효과 구현 가능
}

void AGridTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 애니메이션 업데이트
    if (bIsBlinking)
    {
        UpdateBlinking(DeltaTime);
    }

    if (bIsPulsing)
    {
        UpdatePulsing(DeltaTime);
    }
}

// ====== 블루프린트 네이티브 이벤트 ======
FLinearColor AGridTile::GetCustomStateColor_Implementation(ETileState State) const
{
    // 기본적으로 검은색 반환 (커스텀 색상 없음)
    return FLinearColor::Black;
}

void AGridTile::Interact_Implementation(AActor* Interactor)
{
    if (!bCanBeInteracted) return;

    UE_LOG(LogTemp, Warning, TEXT("GridTile (%d, %d) interacted by %s"), GridX, GridY, *Interactor->GetName());

    // Manager에게만 신호 보내고 끝 (중복 방지를 위해 단순하게)
    if (OwnerManager)
    {
        OwnerManager->OnTileStep(this, Interactor);
    }

    // 델리게이트 브로드캐스트
    OnTileStepped.Broadcast(this, Interactor);

    // 활성화 마킹
    bIsActivated = true;
}

bool AGridTile::CanInteract_Implementation(AActor* Interactor)
{
    return bCanBeInteracted && OwnerManager != nullptr;
}

FString AGridTile::GetInteractionText_Implementation()
{
    return FString::Printf(TEXT("Step on Tile (%d, %d)"), GridX, GridY);
}

EInteractionType AGridTile::GetInteractionType_Implementation()
{
    return EInteractionType::Activate;
}