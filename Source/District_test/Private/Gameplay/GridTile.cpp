// GridTile.cpp
#include "Gameplay/GridTile.h"
#include "Gameplay/GridMazeManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"

AGridTile::AGridTile()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create scene root
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // Create tile mesh
    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    TileMesh->SetupAttachment(SceneRoot);

    // 기본 큐브 메시 로드
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        TileMesh->SetStaticMesh(CubeMeshAsset.Object);
        TileMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.1f));
    }

    TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TileMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create light component
    TileLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TileLight"));
    TileLight->SetupAttachment(TileMesh);
    TileLight->SetRelativeLocation(FVector(0.0f, 0.0f, LightHeight));
    TileLight->SetIntensity(DefaultLightIntensity);
    TileLight->SetLightColor(DefaultColor);
    TileLight->SetAttenuationRadius(LightRadius);
    TileLight->SetSourceRadius(10.0f);

    // Create trigger box
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(TileMesh);
    TriggerBox->SetBoxExtent(FVector(60.0f, 60.0f, 30.0f));
    TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind trigger events
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGridTile::OnTriggerBeginOverlap);
}

void AGridTile::BeginPlay()
{
    Super::BeginPlay();

    // Create dynamic material instance
    if (TileMesh && TileMesh->GetMaterial(0))
    {
        DynamicMaterial = TileMesh->CreateDynamicMaterialInstance(0);
    }

    // Set initial visual state
    UpdateVisuals();

    UE_LOG(LogTemp, Log, TEXT("GridTile created at (%d, %d)"), GridX, GridY);
}

void AGridTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update visual effects
    if (bIsBlinking)
    {
        UpdateBlinking(DeltaTime);
    }

    if (bIsShowingHint)
    {
        UpdateHint(DeltaTime);
    }
}

// ====== TILE STATE MANAGEMENT ======
void AGridTile::SetTileState(ETileState NewState)
{
    if (CurrentState != NewState)
    {
        ETileState OldState = CurrentState;
        CurrentState = NewState;

        // Stop previous effects
        StopBlinking();

        // Update visuals
        UpdateVisuals();

        // Start state-specific effects
        switch (NewState)
        {
        case ETileState::Correct:
            if (bEnableBlinking)
            {
                StartBlinking(3.0f);
            }
            break;

        case ETileState::Wrong:
            if (bEnableBlinking)
            {
                StartBlinking(2.0f);
            }
            break;

        case ETileState::Start:
        case ETileState::Goal:
            TileLight->SetIntensity(ActiveLightIntensity);
            break;

        default:
            TileLight->SetIntensity(DefaultLightIntensity);
            break;
        }

        // Blueprint event
        OnTileStateChanged(NewState);

        UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d) state changed: %d -> %d"),
            GridX, GridY, (int32)OldState, (int32)NewState);
    }
}

void AGridTile::SetGridPosition(int32 X, int32 Y)
{
    GridX = X;
    GridY = Y;

    // 디버그용 이름 설정
    FString TileName = FString::Printf(TEXT("GridTile_%d_%d"), X, Y);
    SetActorLabel(*TileName);
}

void AGridTile::SetCorrectPath(bool bIsCorrect)
{
    bIsPartOfCorrectPath = bIsCorrect;
    UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d) set as %s"),
        GridX, GridY, bIsCorrect ? TEXT("CORRECT PATH") : TEXT("NOT CORRECT PATH"));
}

void AGridTile::SetOwnerManager(AGridMazeManager* Manager)
{
    OwnerManager = Manager;
    if (OwnerManager)
    {
        UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d) connected to manager"), GridX, GridY);
    }
}

// ====== TRIGGER EVENTS ======
void AGridTile::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if it's the player character
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        UE_LOG(LogTemp, Warning, TEXT("Player stepped on tile (%d, %d)"), GridX, GridY);

        // Manager에 직접 알림 (가장 중요!)
        if (OwnerManager)
        {
            OwnerManager->OnTileStepped(this, OtherActor);
            UE_LOG(LogTemp, Warning, TEXT("Notified manager about tile step"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("OwnerManager is NULL! Tile (%d, %d) not connected to manager"), GridX, GridY);
        }

        // Broadcast the stepping event (델리게이트)
        OnTileStepped.Broadcast(this, OtherActor);

        // Blueprint event
        OnPlayerEntered(OtherActor);

        // Play step sound
        if (bPlayStepSound && StepSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, StepSound, GetActorLocation(), SoundVolume);
        }
    }
}

// ====== VISUAL FUNCTIONS ======
void AGridTile::UpdateVisuals()
{
    UpdateLightAndMaterial();
}

void AGridTile::UpdateLightAndMaterial()
{
    if (!TileLight)
        return;

    FLinearColor CurrentColor = GetCurrentStateColor();

    // Update light color
    TileLight->SetLightColor(CurrentColor);

    // Update material color if available
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), CurrentColor);
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), CurrentColor * 0.5f);
    }
}

FLinearColor AGridTile::GetCurrentStateColor()
{
    switch (CurrentState)
    {
    case ETileState::Correct:
        return CorrectColor;
    case ETileState::Wrong:
        return WrongColor;
    case ETileState::Start:
        return StartColor;
    case ETileState::Goal:
        return GoalColor;
    case ETileState::Hint:
        return HintColor;
    default:
        return DefaultColor;
    }
}

// ====== ANIMATION FUNCTIONS ======
void AGridTile::StartBlinking(float Duration)
{
    if (!bEnableBlinking)
        return;

    bIsBlinking = true;
    BlinkTimer = 0.0f;

    // Set timer to stop blinking
    GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, [this]()
        {
            StopBlinking();
        }, Duration, false);

    UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d) started blinking for %.1f seconds"), GridX, GridY, Duration);
}

void AGridTile::StopBlinking()
{
    if (bIsBlinking)
    {
        bIsBlinking = false;
        GetWorld()->GetTimerManager().ClearTimer(BlinkTimerHandle);

        // Reset to normal intensity
        UpdateVisuals();

        UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d) stopped blinking"), GridX, GridY);
    }
}

void AGridTile::ShowHint(float Duration)
{
    // 임시로 힌트 상태로 변경
    ETileState OriginalState = CurrentState;
    SetTileState(ETileState::Hint);

    bIsShowingHint = true;
    HintTimer = Duration;

    // Duration 후에 원래 상태로 복구
    GetWorld()->GetTimerManager().SetTimer(HintTimerHandle, [this, OriginalState]()
        {
            bIsShowingHint = false;
            SetTileState(OriginalState);
        }, Duration, false);

    UE_LOG(LogTemp, Log, TEXT("Tile (%d, %d) showing hint for %.1f seconds"), GridX, GridY, Duration);
}

void AGridTile::UpdateBlinking(float DeltaTime)
{
    BlinkTimer += DeltaTime;

    // 사인 웨이브로 깜빡임 효과
    float BlinkFactor = FMath::Sin(BlinkTimer * BlinkSpeed * PI) * 0.5f + 0.5f;
    float CurrentIntensity = FMath::Lerp(DefaultLightIntensity, ActiveLightIntensity, BlinkFactor);

    if (TileLight)
    {
        TileLight->SetIntensity(CurrentIntensity);
    }
}

void AGridTile::UpdateHint(float DeltaTime)
{
    HintTimer -= DeltaTime;

    if (HintTimer <= 0.0f)
    {
        bIsShowingHint = false;
    }
}

// ====== INTERACTABLE INTERFACE IMPLEMENTATION ======
void AGridTile::Interact_Implementation(AActor* Interactor)
{
    // 디버그용: 타일 정보 출력
    if (GEngine)
    {
        FString StateText;
        switch (CurrentState)
        {
        case ETileState::Default: StateText = TEXT("Default"); break;
        case ETileState::Correct: StateText = TEXT("Correct"); break;
        case ETileState::Wrong: StateText = TEXT("Wrong"); break;
        case ETileState::Start: StateText = TEXT("Start"); break;
        case ETileState::Goal: StateText = TEXT("Goal"); break;
        case ETileState::Hint: StateText = TEXT("Hint"); break;
        }

        FString Message = FString::Printf(TEXT("Tile (%d, %d) - State: %s, Correct Path: %s"),
            GridX, GridY, *StateText, bIsPartOfCorrectPath ? TEXT("Yes") : TEXT("No"));

        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, Message);
    }
}

bool AGridTile::CanInteract_Implementation(AActor* Interactor)
{
    return true; // 디버그용으로 항상 상호작용 가능
}

FString AGridTile::GetInteractionText_Implementation()
{
    return FString::Printf(TEXT("Tile (%d, %d)"), GridX, GridY);
}

EInteractionType AGridTile::GetInteractionType_Implementation()
{
    return EInteractionType::Use;
}