// GridTile.cpp
#include "Gameplay/GridTile.h"
#include "Gameplay/GridMazeManager.h"
#include "Engine/Engine.h"
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
    TileLight->SetIntensity(0.0f);
    TileLight->SetLightColor(InactiveColor);
    TileLight->SetAttenuationRadius(LightRadius);
    TileLight->SetSourceRadius(20.0f);
    TileLight->SetMobility(EComponentMobility::Movable);
    TileLight->SetVisibility(false);
    TileLight->SetCastShadows(true);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(PillarMesh);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
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
    SetTileState(ETileState::Inactive);
}

void AGridTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsBlinking)
    {
        UpdateBlinking(DeltaTime);
    }
}

void AGridTile::SetTileState(ETileState NewState)
{
    if (CurrentState != NewState)
    {
        ETileState OldState = CurrentState;
        CurrentState = NewState;

        StopBlinking();
        ApplyStateVisuals(NewState);

        OnTileStateChanged.Broadcast(NewState);
        OnStateChanged(OldState, NewState);
    }
}

void AGridTile::ApplyStateVisuals(ETileState State)
{
    FLinearColor StateColor;
    float StateIntensity;

    switch (State)
    {
    case ETileState::Inactive:
        StateColor = InactiveColor;
        StateIntensity = InactiveLightIntensity;
        TurnOffLight();
        return;

    case ETileState::Preview:
        StateColor = PreviewColor;
        StateIntensity = PreviewLightIntensity;
        break;

    case ETileState::Ready:
        StateColor = ReadyColor;
        StateIntensity = ReadyLightIntensity;
        break;

    case ETileState::FirstStep:
        StateColor = FirstStepColor;
        StateIntensity = FirstStepLightIntensity;
        if (bEnableBlinking && bBlinkOnFirstStep)
        {
            StartBlinking(0.0f);  // 무한 깜박임
        }
        break;

    case ETileState::Correct:
        StateColor = CorrectColor;
        StateIntensity = CorrectLightIntensity;
        PlayCorrectSound();
        OnCorrectInteraction();
        break;

    case ETileState::Wrong:
        StateColor = WrongColor;
        StateIntensity = WrongLightIntensity;
        if (bEnableBlinking && bBlinkOnWrong)
        {
            StartBlinking(3.0f);
        }
        PlayWrongSound();
        OnWrongInteraction();
        break;

    default:
        StateColor = InactiveColor;
        StateIntensity = InactiveLightIntensity;
        break;
    }

    TurnOnLight(StateColor, StateIntensity);
}

FLinearColor AGridTile::GetCurrentStateColor() const
{
    switch (CurrentState)
    {
    case ETileState::Inactive:
        return InactiveColor;
    case ETileState::Preview:
        return PreviewColor;
    case ETileState::Ready:
        return ReadyColor;
    case ETileState::FirstStep:
        return FirstStepColor;
    case ETileState::Correct:
        return CorrectColor;
    case ETileState::Wrong:
        return WrongColor;
    default:
        return InactiveColor;
    }
}

float AGridTile::GetCurrentLightIntensity() const
{
    switch (CurrentState)
    {
    case ETileState::Inactive:
        return InactiveLightIntensity;
    case ETileState::Preview:
        return PreviewLightIntensity;
    case ETileState::Ready:
        return ReadyLightIntensity;
    case ETileState::FirstStep:
        return FirstStepLightIntensity;
    case ETileState::Correct:
        return CorrectLightIntensity;
    case ETileState::Wrong:
        return WrongLightIntensity;
    default:
        return InactiveLightIntensity;
    }
}

void AGridTile::TurnOnLight(FLinearColor Color, float Intensity)
{
    if (TileLight)
    {
        TileLight->SetLightColor(Color);
        TileLight->SetIntensity(Intensity);
        TileLight->SetVisibility(true);
    }

    bIsActivated = true;
    OnLightTurnedOn(Color, Intensity);
}

void AGridTile::TurnOffLight()
{
    if (TileLight)
    {
        TileLight->SetIntensity(0.0f);
        TileLight->SetVisibility(false);
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
            TileLight->SetIntensity(GetCurrentLightIntensity());
        }
    }
}

void AGridTile::UpdateBlinking(float DeltaTime)
{
    if (!bIsBlinking || !TileLight) return;

    float Time = GetWorld()->GetTimeSeconds();
    float BlinkFactor = FMath::Sin(Time * BlinkSpeed * PI) * 0.5f + 0.5f;

    float TargetIntensity = GetCurrentLightIntensity();
    float CurrentIntensity = FMath::Lerp(TargetIntensity * 0.3f, TargetIntensity, BlinkFactor);

    TileLight->SetIntensity(CurrentIntensity);
}

void AGridTile::ResetToDefault()
{
    SetTileState(ETileState::Inactive);
    StopBlinking();
    bIsActivated = false;
}

void AGridTile::PlayTileSound(USoundBase* Sound)
{
    if (Sound && bPlayInteractionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), SoundVolume);
    }
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

void AGridTile::SetGridPosition(int32 X, int32 Y)
{
    GridX = X;
    GridY = Y;

    FString TileName = FString::Printf(TEXT("GridTile_%d_%d"), X, Y);

#if WITH_EDITOR
    SetActorLabel(*TileName);
#endif
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

void AGridTile::ForceMobilitySettings()
{
    if (TileMesh) TileMesh->SetMobility(EComponentMobility::Movable);
    if (PillarMesh) PillarMesh->SetMobility(EComponentMobility::Movable);
    if (TileLight) TileLight->SetMobility(EComponentMobility::Movable);
    if (InteractionSphere) InteractionSphere->SetMobility(EComponentMobility::Movable);
    if (RootSceneComponent) RootSceneComponent->SetMobility(EComponentMobility::Movable);
}

// InteractableInterface 구현
void AGridTile::Interact_Implementation(AActor* Interactor)
{
    if (!bCanBeInteracted) return;

    if (OwnerManager)
    {
        OwnerManager->OnTileStep(this, Interactor);
    }

    OnTileStepped.Broadcast(this, Interactor);
    OnPlayerInteracted(Interactor);
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