#include "Gameplay/PickupActor.h"
#include "Gameplay/InventoryComponent.h"
#include "Gameplay/PuzzleInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

APickupActor::APickupActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    RootComponent = SceneComponent;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetMobility(EComponentMobility::Movable);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

    bDrawDebug = true;
    MeshRotation = FRotator::ZeroRotator;
}

void APickupActor::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
        MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    }

    ApplyMeshRotation();
}

void APickupActor::ApplyMeshRotation()
{
    if (MeshComponent)
    {
        MeshComponent->SetRelativeRotation(MeshRotation);
    }
}

void APickupActor::SetMeshYawRotation(float Yaw)
{
    MeshRotation.Yaw = Yaw;
    ApplyMeshRotation();
}

void APickupActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyMeshRotation();
}

void APickupActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APickupActor::Interact_Implementation(AActor* Interactor)
{
    UPuzzleInteractionComponent* PuzzleComp = FindComponentByClass<UPuzzleInteractionComponent>();
    if (PuzzleComp && PuzzleComp->bCanBePickedUp)
    {
        PuzzleComp->PickUp(Interactor);
        return;
    }

    if (PickupItem(Interactor))
    {
        if (MeshComponent)
        {
            MeshComponent->SetVisibility(false);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        if (InteractionSphere)
        {
            InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        OnPickupSuccess(Interactor);
        SetLifeSpan(0.1f);
    }
}


bool APickupActor::CanInteract_Implementation(AActor* Interactor)
{
    UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComponent || !ItemClass)
    {
        return false;
    }
    return InventoryComponent->HasRoomForItem();
}

FString APickupActor::GetInteractionText_Implementation()
{
    return FString::Printf(TEXT("Pick up: %s"), *(!CustomItemName.IsEmpty() ? CustomItemName : TEXT("Item")));
}

EInteractionType APickupActor::GetInteractionType_Implementation()
{
    return EInteractionType::Pickup;
}

bool APickupActor::PickupItem(AActor* Interactor)
{
    if (!ItemClass)
    {
        return false;
    }

    UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComponent)
    {
        return false;
    }

    UItem* NewItem = NewObject<UItem>(InventoryComponent, ItemClass);
    if (!NewItem)
    {
        return false;
    }

    if (!CustomItemName.IsEmpty())
    {
        NewItem->Name = CustomItemName;
    }

    if (!CustomItemDescription.IsEmpty())
    {
        NewItem->Description = CustomItemDescription;
    }

    return InventoryComponent->AddItem(NewItem);
}

void APickupActor::OnPickupSuccess(AActor* Interactor)
{
    OnPickedUp(Interactor);

    if (PickupSound)
    {
        UGameplayStatics::PlaySound2D(this, PickupSound);
    }
}

#if WITH_EDITOR
void APickupActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property &&
        PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(APickupActor, MeshRotation))
    {
        ApplyMeshRotation();
    }
}
#endif