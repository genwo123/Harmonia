#include "Gameplay/PickupActor.h"
#include "Gameplay/InventoryComponent.h"
#include "Gameplay/PuzzleInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"           
#include "Components/WidgetComponent.h" 
#include "Kismet/GameplayStatics.h"

APickupActor::APickupActor()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
    MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetMobility(EComponentMobility::Movable);

    InteractionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidgetComponent"));
    InteractionWidgetComponent->SetupAttachment(MeshComponent);
    InteractionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    InteractionWidgetComponent->SetDrawSize(FVector2D(200.f, 50.f));
    InteractionWidgetComponent->SetVisibility(false);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(MeshComponent);
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
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }

    if (InteractionWidgetComponent)
    {
        InteractionWidgetComponent->SetVisibility(false);
    }

    ApplyMeshRotation();
}

void APickupActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (InteractionWidgetComponent && MeshComponent && InteractionWidgetComponent->IsVisible())
    {
        FVector MeshBoundsOrigin;
        FVector MeshBoundsExtent;
        MeshComponent->GetLocalBounds(MeshBoundsOrigin, MeshBoundsExtent);

        FVector WidgetOffset = MeshBoundsOrigin + FVector(0.f, 0.f, MeshBoundsExtent.Z + 50.f);
        InteractionWidgetComponent->SetRelativeLocation(WidgetOffset);
    }
}

void APickupActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyMeshRotation();

    if (InteractionWidgetComponent && MeshComponent)
    {
        FVector MeshBoundsOrigin;
        FVector MeshBoundsExtent;
        MeshComponent->GetLocalBounds(MeshBoundsOrigin, MeshBoundsExtent);

        FVector WidgetOffset = FVector(0.f, 0.f, MeshBoundsExtent.Z + 50.f);
        InteractionWidgetComponent->SetRelativeLocation(WidgetOffset);

        InteractionWidgetComponent->SetVisibility(false);  // Ãß°¡
    }
}

void APickupActor::UpdateWidgetPosition()
{
    if (!InteractionWidgetComponent || !MeshComponent)
        return;

    FVector MeshBoundsOrigin;
    FVector MeshBoundsExtent;
    MeshComponent->GetLocalBounds(MeshBoundsOrigin, MeshBoundsExtent);

    FVector WidgetOffset = FVector(0.f, 0.f, MeshBoundsExtent.Z + 50.f);
    InteractionWidgetComponent->SetRelativeLocation(WidgetOffset);
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

void APickupActor::Interact_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Warning, TEXT("PickupActor: Interact called!"));

    UPuzzleInteractionComponent* PuzzleComp = FindComponentByClass<UPuzzleInteractionComponent>();

    if (PuzzleComp && PuzzleComp->bCanBePickedUp)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupActor: Has PuzzleInteractionComponent"));
        PuzzleComp->PickUp(Interactor);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("PickupActor: Trying to pickup item..."));

    if (PickupItem(Interactor))
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupActor: Pickup SUCCESS!"));

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
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PickupActor: Pickup FAILED!"));
    }
}

bool APickupActor::CanInteract_Implementation(AActor* Interactor)
{
    UPuzzleInteractionComponent* PuzzleComp = FindComponentByClass<UPuzzleInteractionComponent>();
    if (PuzzleComp)
    {
        return true;
    }

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

void APickupActor::ShowInteractionWidget_Implementation()
{
    if (InteractionWidgetComponent)
    {
        InteractionWidgetComponent->SetVisibility(true);
    }
}

void APickupActor::HideInteractionWidget_Implementation()
{
    if (InteractionWidgetComponent)
    {
        InteractionWidgetComponent->SetVisibility(false);
    }
}