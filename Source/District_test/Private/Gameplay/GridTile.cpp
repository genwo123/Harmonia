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

AGridTile::AGridTile()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    TileMesh->SetupAttachment(RootSceneComponent);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TileMesh->SetStaticMesh(CubeMeshAsset.Object);
        TileMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.2f));
    }

    TileMesh->SetMobility(EComponentMobility::Movable);
    TileMesh->SetSimulatePhysics(false);
    TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TileMesh->SetCollisionResponseToAllChannels(ECR_Block);

    PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
    PillarMesh->SetupAttachment(RootSceneComponent);
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

    TileLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TileLight"));
    TileLight->SetupAttachment(PillarMesh);
    TileLight->SetRelativeLocation(FVector(0.0f, 0.0f, LightHeight));

    TileLight->SetIntensity(1500.0f);
    TileLight->SetLightColor(FLinearColor::Gray);
    TileLight->SetAttenuationRadius(1000.0f);
    TileLight->SetSourceRadius(20.0f);
    TileLight->SetMobility(EComponentMobility::Movable);
    TileLight->SetVisibility(true);
    TileLight->SetCastShadows(true);

    ActiveLightIntensity = 1500.0f;
    DefaultLightIntensity = 1500.0f;
    LightRadius = 1000.0f;
    LightHeight = 100.0f;

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

void AGridTile::BeginPlay()
{
    Super::BeginPlay();

    ForceMobilitySettings();

    SetTileState(ETileState::Default);
}

void AGridTile::SetTileState(ETileState NewState)
{
    if (CurrentState != NewState)
    {
        ETileState OldState = CurrentState;
        CurrentState = NewState;

        StopBlinking();
        StopPulsing();

        FLinearColor StateColor = GetCurrentStateColor();

        switch (NewState)
        {
        case ETileState::Default:
            TurnOnLight(StateColor);
            break;

        case ETileState::Start:
            TurnOnLight(StateColor);
            break;

        case ETileState::StartPoint:
            TurnOnLight(StateColor);
            if (bEnableBlinking)
            {
                StartBlinking(0.0f);
            }
            break;

        case ETileState::Correct:
            TurnOnLight(StateColor);
            PlayCorrectSound();
            OnCorrectInteraction();
            break;

        case ETileState::Wrong:
            TurnOnLight(StateColor);
            if (bEnableBlinking)
            {
                StartBlinking(3.0f);
            }
            PlayWrongSound();
            OnWrongInteraction();
            break;

        case ETileState::Goal:
            TurnOnLight(StateColor);
            break;

        case ETileState::Hint:
            TurnOnLight(StateColor);
            SetLightIntensity(ActiveLightIntensity * 0.7f);
            break;

        default:
            TurnOnLight(StateColor);
            break;
        }

        OnTileStateChanged.Broadcast(NewState);
        OnStateChanged(OldState, NewState);
    }
}

FLinearColor AGridTile::GetCurrentStateColor() const
{
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
            return FLinearColor(0.4f, 1.0f, 0.4f, 1.0f);
        case ETileState::Goal:
            return FLinearColor::Yellow;
        case ETileState::Hint:
            return FLinearColor(0.8f, 0.4f, 1.0f, 1.0f);
        default:
            return OwnerManager->WaitingColor;
        }
    }

    FLinearColor CustomColor = GetCustomStateColor(CurrentState);
    if (CustomColor != FLinearColor::Black)
    {
        return CustomColor;
    }

    switch (CurrentState)
    {
    case ETileState::Default:
        return WaitingColor;
    case ETileState::Start:
        return ReadyColor;
    case ETileState::StartPoint:
        return StartPointColor;
    case ETileState::Correct:
        return CorrectColor;
    case ETileState::Wrong:
        return WrongColor;
    case ETileState::Goal:
        return GoalColor;
    case ETileState::Hint:
        return HintColor;
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

    FString TileName = FString::Printf(TEXT("GridTile_%d_%d"), X, Y);
    SetActorLabel(*TileName);
}

void AGridTile::SetOwnerManager(AGridMazeManager* Manager)
{
    OwnerManager = Manager;
}

void AGridTile::SetTileThickness(float NewThickness)
{
    if (TileMesh)
    {
        FVector CurrentScale = TileMesh->GetRelativeScale3D();
        CurrentScale.Z = NewThickness / 100.0f;
        TileMesh->SetRelativeScale3D(CurrentScale);
    }
}

void AGridTile::TurnOnLight(FLinearColor Color)
{
    if (TileLight)
    {
        TileLight->SetLightColor(Color);
        TileLight->SetIntensity(ActiveLightIntensity);
        TileLight->SetVisibility(true);
    }

    bIsActivated = true;
    OnLightTurnedOn(Color);
}

void AGridTile::TurnOffLight()
{
    if (TileLight)
    {
        TileLight->SetIntensity(0.0f);
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

void AGridTile::StartBlinking(float Duration)
{
    if (!bEnableBlinking) return;

    bIsBlinking = true;

    if (Duration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, [this]()
            {
                StopBlinking();
            }, Duration, false);
    }
}

void AGridTile::StopBlinking()
{
    if (bIsBlinking)
    {
        bIsBlinking = false;
        GetWorld()->GetTimerManager().ClearTimer(BlinkTimerHandle);

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

void AGridTile::UpdateTileVisuals()
{
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

void AGridTile::SetCanBeInteracted(bool bCanInteract)
{
    bCanBeInteracted = bCanInteract;
}

void AGridTile::UpdateBlinking(float DeltaTime)
{
    if (!bIsBlinking || !TileLight) return;

    float Time = GetWorld()->GetTimeSeconds();
    float BlinkFactor = FMath::Sin(Time * BlinkSpeed * PI) * 0.5f + 0.5f;
    float CurrentIntensity = FMath::Lerp(DefaultLightIntensity, ActiveLightIntensity, BlinkFactor);

    TileLight->SetIntensity(CurrentIntensity);
}

void AGridTile::UpdatePulsing(float DeltaTime)
{
    if (!bIsPulsing) return;

    float Time = GetWorld()->GetTimeSeconds();
    float PulseFactor = FMath::Sin(Time * PulseSpeed * PI) * 0.3f + 0.7f;
}

void AGridTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsBlinking)
    {
        UpdateBlinking(DeltaTime);
    }

    if (bIsPulsing)
    {
        UpdatePulsing(DeltaTime);
    }
}

FLinearColor AGridTile::GetCustomStateColor_Implementation(ETileState State) const
{
    return FLinearColor::Black;
}

void AGridTile::Interact_Implementation(AActor* Interactor)
{
    if (!bCanBeInteracted) return;

    if (OwnerManager)
    {
        OwnerManager->OnTileStep(this, Interactor);
    }

    OnTileStepped.Broadcast(this, Interactor);

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