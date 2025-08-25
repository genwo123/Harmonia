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


// GridTile.cpp - ������ �Լ���

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

    // ===== ������ ����Ʈ ���� =====
    TileLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TileLight"));
    TileLight->SetupAttachment(PillarMesh);
    TileLight->SetRelativeLocation(FVector(0.0f, 0.0f, LightHeight));

    // ����Ʈ �⺻ ���� - ���� ���·� ����
    TileLight->SetIntensity(1500.0f);               // 0.0f���� 1500.0f�� ����
    TileLight->SetLightColor(FLinearColor::Gray);   // �⺻ ȸ��
    TileLight->SetAttenuationRadius(1000.0f);       // �� ���� ����
    TileLight->SetSourceRadius(20.0f);
    TileLight->SetMobility(EComponentMobility::Movable);
    TileLight->SetVisibility(true);
    TileLight->SetCastShadows(true);

    // ����Ʈ �������� ����
    ActiveLightIntensity = 1500.0f;     // 3000.0f���� 1500.0f�� ����
    DefaultLightIntensity = 1500.0f;    // 0.0f���� 1500.0f�� ����
    LightRadius = 1000.0f;              // �� �а�
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

// ====== ������ BeginPlay ======
void AGridTile::BeginPlay()
{
    Super::BeginPlay();

    // Mobility ���� ����
    ForceMobilitySettings();

    // �ʱ� ���� ���� - ȸ�� �Һ����� ����
    SetTileState(ETileState::Default);  // UpdateTileVisuals() ���
}

// ====== ������ SetTileState ======
void AGridTile::SetTileState(ETileState NewState)
{
    if (CurrentState != NewState)
    {
        ETileState OldState = CurrentState;
        CurrentState = NewState;

        // ���� ȿ�� ����
        StopBlinking();
        StopPulsing();

        // Manager �Ǵ� ��ü ���� ��������
        FLinearColor StateColor = GetCurrentStateColor();

        switch (NewState)
        {
        case ETileState::Default:
            // ȸ�� (���) - �Һ� �ѵ� (���� ����!)
            TurnOnLight(StateColor);
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - DEFAULT state (GRAY light ON)"), GridX, GridY);
            break;

        case ETileState::Start:
            // �ʷ� (�غ�) - �Һ� ����
            TurnOnLight(StateColor);
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - START state (GREEN light ON)"), GridX, GridY);
            break;

        case ETileState::StartPoint:
            // ���� �ʷ� (������) - �Һ� ���� + ������
            TurnOnLight(StateColor);
            if (bEnableBlinking)
            {
                StartBlinking(0.0f); // ��� ������
            }
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - START POINT (BRIGHT GREEN + BLINKING)"), GridX, GridY);
            break;

        case ETileState::Correct:
            // �Ķ� (����) - �Һ� ����
            TurnOnLight(StateColor);
            PlayCorrectSound();
            OnCorrectInteraction();
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - CORRECT state (BLUE light ON)"), GridX, GridY);
            break;

        case ETileState::Wrong:
            // ���� (����) - �Һ� ���� + ������
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
            // ��� (��ǥ��) - �̸������
            TurnOnLight(StateColor);
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - GOAL state (YELLOW light ON)"), GridX, GridY);
            break;

        case ETileState::Hint:
            // ���� (��Ʈ) - �̸������
            TurnOnLight(StateColor);
            SetLightIntensity(ActiveLightIntensity * 0.7f); // �ణ ��Ӱ�
            UE_LOG(LogTemp, Warning, TEXT("Tile (%d, %d) - HINT state (PURPLE light ON)"), GridX, GridY);
            break;

        default:
            TurnOnLight(StateColor);
            break;
        }

        // �̺�Ʈ ��ε�ĳ��Ʈ
        OnTileStateChanged.Broadcast(NewState);
        OnStateChanged(OldState, NewState);
    }
}

FLinearColor AGridTile::GetCurrentStateColor() const
{
    // Manager�� ���� ���� �켱 ���
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
            return FLinearColor(0.4f, 1.0f, 0.4f, 1.0f); // ���� �ʷ�
        case ETileState::Goal:
            return FLinearColor::Yellow;
        case ETileState::Hint:
            return FLinearColor(0.8f, 0.4f, 1.0f, 1.0f); // ����
        default:
            return OwnerManager->WaitingColor;
        }
    }

    // Manager�� ������ Ŀ���� ���� Ȯ��
    FLinearColor CustomColor = GetCustomStateColor(CurrentState);
    if (CustomColor != FLinearColor::Black)
    {
        return CustomColor;
    }

    // �⺻ ���� ����
    switch (CurrentState)
    {
    case ETileState::Default:
        return WaitingColor;        // ȸ��
    case ETileState::Start:
        return ReadyColor;          // �ʷ�
    case ETileState::StartPoint:
        return StartPointColor;     // ���� �ʷ�
    case ETileState::Correct:
        return CorrectColor;        // �Ķ�
    case ETileState::Wrong:
        return WrongColor;          // ����
    case ETileState::Goal:
        return GoalColor;           // ���
    case ETileState::Hint:
        return HintColor;           // ����
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

    // ����׿� �̸� ����
    FString TileName = FString::Printf(TEXT("GridTile_%d_%d"), X, Y);
    SetActorLabel(*TileName);
}

void AGridTile::SetOwnerManager(AGridMazeManager* Manager)
{
    OwnerManager = Manager;
}

// ====== �Һ� ���� ======
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

// ====== �ִϸ��̼� ���� ======
void AGridTile::StartBlinking(float Duration)
{
    if (!bEnableBlinking) return;

    bIsBlinking = true;

    if (Duration > 0.0f)
    {
        // ������ �ð� �� ����
        GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, [this]()
            {
                StopBlinking();
            }, Duration, false);
    }
    // Duration�� 0�̸� ��� ������
}

void AGridTile::StopBlinking()
{
    if (bIsBlinking)
    {
        bIsBlinking = false;
        GetWorld()->GetTimerManager().ClearTimer(BlinkTimerHandle);

        // ���� ���� ����
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

// ====== �ܰ� ���� ======
void AGridTile::UpdateTileVisuals()
{
    // ���� ���¿� �´� �������� ������Ʈ
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

// ====== ���� ���� ======
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

// ====== ��ȣ�ۿ� ���� ======
void AGridTile::SetCanBeInteracted(bool bCanInteract)
{
    bCanBeInteracted = bCanInteract;
}




// ====== �ִϸ��̼� ������Ʈ ======
void AGridTile::UpdateBlinking(float DeltaTime)
{
    if (!bIsBlinking || !TileLight) return;

    // ������ ������ ȿ��
    float Time = GetWorld()->GetTimeSeconds();
    float BlinkFactor = FMath::Sin(Time * BlinkSpeed * PI) * 0.5f + 0.5f;
    float CurrentIntensity = FMath::Lerp(DefaultLightIntensity, ActiveLightIntensity, BlinkFactor);

    TileLight->SetIntensity(CurrentIntensity);
}

void AGridTile::UpdatePulsing(float DeltaTime)
{
    if (!bIsPulsing) return;

    // �޽� ȿ�� (�ʿ�� ����)
    float Time = GetWorld()->GetTimeSeconds();
    float PulseFactor = FMath::Sin(Time * PulseSpeed * PI) * 0.3f + 0.7f;

    // �߰� �޽� ȿ�� ���� ����
}

void AGridTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // �ִϸ��̼� ������Ʈ
    if (bIsBlinking)
    {
        UpdateBlinking(DeltaTime);
    }

    if (bIsPulsing)
    {
        UpdatePulsing(DeltaTime);
    }
}

// ====== �������Ʈ ����Ƽ�� �̺�Ʈ ======
FLinearColor AGridTile::GetCustomStateColor_Implementation(ETileState State) const
{
    // �⺻������ ������ ��ȯ (Ŀ���� ���� ����)
    return FLinearColor::Black;
}

void AGridTile::Interact_Implementation(AActor* Interactor)
{
    if (!bCanBeInteracted) return;

    UE_LOG(LogTemp, Warning, TEXT("GridTile (%d, %d) interacted by %s"), GridX, GridY, *Interactor->GetName());

    // Manager���Ը� ��ȣ ������ �� (�ߺ� ������ ���� �ܼ��ϰ�)
    if (OwnerManager)
    {
        OwnerManager->OnTileStep(this, Interactor);
    }

    // ��������Ʈ ��ε�ĳ��Ʈ
    OnTileStepped.Broadcast(this, Interactor);

    // Ȱ��ȭ ��ŷ
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