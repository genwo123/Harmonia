// PuzzleStarter.cpp
#include "Gameplay/PuzzleStarter.h"
#include "Gameplay/GridMazeManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

APuzzleStarter::APuzzleStarter()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
        MeshComponent->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        SlotMeshAsset = SphereMeshAsset.Object;
    }

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    SlotMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh1"));
    SlotMesh1->SetupAttachment(RootComponent);
    SlotMesh1->SetRelativeLocation(FVector(-100.0f, 0.0f, 50.0f));
    SlotMesh1->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SlotMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh2"));
    SlotMesh2->SetupAttachment(RootComponent);
    SlotMesh2->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    SlotMesh2->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SlotMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh3"));
    SlotMesh3->SetupAttachment(RootComponent);
    SlotMesh3->SetRelativeLocation(FVector(100.0f, 0.0f, 50.0f));
    SlotMesh3->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (SphereMeshAsset.Succeeded())
    {
        SlotMesh1->SetStaticMesh(SphereMeshAsset.Object);
        SlotMesh2->SetStaticMesh(SphereMeshAsset.Object);
        SlotMesh3->SetStaticMesh(SphereMeshAsset.Object);
    }

    // 기본 슬롯 3개 설정
    CoreSlots.SetNum(3);
    CoreSlots[0].RequiredCoreTag = FName("BP_RedCore");
    CoreSlots[1].RequiredCoreTag = FName("BP_BlueCore");
    CoreSlots[2].RequiredCoreTag = FName("BP_GreenCore");

    bAutoStartWhenComplete = true;
    bAllCoresInserted = false;
    bPuzzleStarted = false;
    SoundVolume = 1.0f;
}

void APuzzleStarter::BeginPlay()
{
    Super::BeginPlay();


    CoreSlots[0].SlotMesh = SlotMesh1;
    CoreSlots[1].SlotMesh = SlotMesh2;
    CoreSlots[2].SlotMesh = SlotMesh3;

    for (int32 i = 0; i < CoreSlots.Num(); i++)
    {
        if (CoreSlots[i].SlotMesh)
        {
            CoreSlots[i].SlotMesh->SetVisibility(false);
        }
    }
}

#if WITH_EDITOR
void APuzzleStarter::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UWorld* World = GetWorld();
    if (World && World->WorldType == EWorldType::Editor)
    {
        if (SlotMesh1) SlotMesh1->SetVisibility(true);
        if (SlotMesh2) SlotMesh2->SetVisibility(true);
        if (SlotMesh3) SlotMesh3->SetVisibility(true);
    }
}
#endif

void APuzzleStarter::CreateSlotMeshes()
{
    
    
}

bool APuzzleStarter::TryInsertCore(AActor* CoreActor)
{
    if (!CoreActor || bPuzzleStarted)
    {
        return false;
    }

    FName CoreTag = NAME_None;
    if (CoreActor->Tags.Num() > 0)
    {
        for (const FName& Tag : CoreActor->Tags)
        {
            if (Tag == FName("RedCore") || Tag == FName("BlueCore") || Tag == FName("GreenCore"))
            {
                CoreTag = Tag;
                break;
            }
        }
    }

    if (CoreTag == NAME_None)
    {
        return false;
    }

    int32 SlotIndex = FindEmptySlotForCore(CoreTag);

    if (SlotIndex == -1)
    {
        PlaySound(WrongCoreSound);
        OnWrongCoreInserted.Broadcast(CoreTag);
        OnWrongCoreInsertedBP(CoreTag);
        return false;
    }

    CoreSlots[SlotIndex].bIsInserted = true;
    CoreSlots[SlotIndex].InsertedCoreActor = CoreActor;
    UpdateSlotVisual(SlotIndex, true);

    if (CoreSlots[SlotIndex].SlotMesh)
    {
        CoreActor->AttachToComponent(
            CoreSlots[SlotIndex].SlotMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );
        CoreActor->SetActorRelativeLocation(FVector::ZeroVector);
        CoreActor->SetActorRelativeRotation(FRotator::ZeroRotator);
        CoreActor->SetActorEnableCollision(false);
        CoreActor->SetActorHiddenInGame(false);
    }

    PlaySound(CoreInsertSound);
    OnCoreInserted.Broadcast(CoreTag);
    OnCoreInsertedBP(CoreTag, SlotIndex);

    if (AreAllSlotsFilled())
    {
        bAllCoresInserted = true;
        PlaySound(AllCoresCompleteSound);
        OnAllCoresInserted.Broadcast();
        OnAllCoresInsertedBP();

        if (bAutoStartWhenComplete)
        {
            StartConnectedPuzzle();
        }
    }

    return true;
}

int32 APuzzleStarter::FindEmptySlotForCore(FName CoreTag)
{
    for (int32 i = 0; i < CoreSlots.Num(); i++)
    {
        if (CoreSlots[i].RequiredCoreTag == CoreTag && !CoreSlots[i].bIsInserted)
        {
            return i;
        }
    }
    return -1;
}

bool APuzzleStarter::AreAllSlotsFilled() const
{
    for (const FCoreSlot& Slot : CoreSlots)
    {
        if (!Slot.bIsInserted)
        {
            return false;
        }
    }
    return true;
}

bool APuzzleStarter::IsSlotEmpty(int32 SlotIndex) const
{
    if (CoreSlots.IsValidIndex(SlotIndex))
    {
        return !CoreSlots[SlotIndex].bIsInserted;
    }
    return false;
}

void APuzzleStarter::StartConnectedPuzzle()
{
    if (!ConnectedMazeManager || !IsValid(ConnectedMazeManager))
    {
        return;
    }

    if (!bAllCoresInserted)
    {
        return;
    }

    bPuzzleStarted = true;

    ConnectedMazeManager->CompleteReset();

    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (ConnectedMazeManager && IsValid(ConnectedMazeManager))
            {
                ConnectedMazeManager->StartPuzzleWithCountdown();
            }
        });
}

void APuzzleStarter::ResetAllSlots()
{
    for (int32 i = 0; i < CoreSlots.Num(); i++)
    {
        if (CoreSlots[i].InsertedCoreActor && IsValid(CoreSlots[i].InsertedCoreActor))
        {
            CoreSlots[i].InsertedCoreActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            CoreSlots[i].InsertedCoreActor->SetActorHiddenInGame(false);
            CoreSlots[i].InsertedCoreActor->SetActorEnableCollision(true);
        }

        CoreSlots[i].bIsInserted = false;
        CoreSlots[i].InsertedCoreActor = nullptr;
        UpdateSlotVisual(i, false);
    }

    bAllCoresInserted = false;
    bPuzzleStarted = false;
}

void APuzzleStarter::UpdateSlotVisual(int32 SlotIndex, bool bInserted)
{
    if (!CoreSlots.IsValidIndex(SlotIndex) || !CoreSlots[SlotIndex].SlotMesh)
    {
        return;
    }

    UStaticMeshComponent* SlotMesh = CoreSlots[SlotIndex].SlotMesh;

    if (bInserted)
    {
        SlotMesh->SetVisibility(true);
    }
    else
    {
        SlotMesh->SetVisibility(false);
        UMaterialInstanceDynamic* DynamicMaterial = SlotMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            FLinearColor Color = EmptySlotColor;
            DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        }
    }
}

FLinearColor APuzzleStarter::GetCoreColorByTag(FName CoreTag)
{
    if (CoreTag == FName("RedCore"))
    {
        return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
    }
    else if (CoreTag == FName("BlueCore"))
    {
        return FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);
    }
    else if (CoreTag == FName("GreenCore"))
    {
        return FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);
    }
    return EmptySlotColor;
}

void APuzzleStarter::PlaySound(USoundBase* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(), SoundVolume);
    }
}

// InteractableInterface 구현
void APuzzleStarter::Interact_Implementation(AActor* Interactor)
{
    if (!CanInteract_Implementation(Interactor))
    {
        return;
    }

    // 상호작용 시 현재 상태 표시 또는 수동 시작
    if (bAllCoresInserted && !bPuzzleStarted && !bAutoStartWhenComplete)
    {
        StartConnectedPuzzle();
    }
}

bool APuzzleStarter::CanInteract_Implementation(AActor* Interactor)
{
    if (bPuzzleStarted)
    {
        return false;
    }

    return ConnectedMazeManager != nullptr;
}

FString APuzzleStarter::GetInteractionText_Implementation()
{
    if (bPuzzleStarted)
    {
        return TEXT("Puzzle Running...");
    }

    if (!ConnectedMazeManager)
    {
        return TEXT("No Puzzle Connected");
    }

    int32 InsertedCount = 0;
    for (const FCoreSlot& Slot : CoreSlots)
    {
        if (Slot.bIsInserted)
        {
            InsertedCount++;
        }
    }

    if (bAllCoresInserted)
    {
        return bAutoStartWhenComplete ? TEXT("Starting...") : TEXT("Press F to Start");
    }

    return FString::Printf(TEXT("Insert Cores (%d/%d)"), InsertedCount, CoreSlots.Num());
}

EInteractionType APuzzleStarter::GetInteractionType_Implementation()
{
    return EInteractionType::Activate;
}

bool APuzzleStarter::TryInsertCoreByTag(FName CoreTag)
{
    if (bPuzzleStarted)
    {
        return false;
    }

    if (CoreTag == NAME_None)
    {
        return false;
    }

    if (CoreTag != FName("BP_RedCore") && CoreTag != FName("BP_BlueCore") && CoreTag != FName("BP_GreenCore"))
    {
        return false;
    }

    int32 SlotIndex = FindEmptySlotForCore(CoreTag);

    if (SlotIndex == -1)
    {
        PlaySound(WrongCoreSound);
        OnWrongCoreInserted.Broadcast(CoreTag);
        OnWrongCoreInsertedBP(CoreTag);
        return false;
    }

    CoreSlots[SlotIndex].bIsInserted = true;
    CoreSlots[SlotIndex].InsertedCoreActor = nullptr;
    UpdateSlotVisual(SlotIndex, true);

    PlaySound(CoreInsertSound);
    OnCoreInserted.Broadcast(CoreTag);
    OnCoreInsertedBP(CoreTag, SlotIndex);

    if (AreAllSlotsFilled())
    {
        bAllCoresInserted = true;
        PlaySound(AllCoresCompleteSound);
        OnAllCoresInserted.Broadcast();
        OnAllCoresInsertedBP();

        if (bAutoStartWhenComplete)
        {
            StartConnectedPuzzle();
        }
    }

    return true;
}