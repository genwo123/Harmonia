#include "Gameplay/PickupActor.h"
#include "Gameplay/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APickupActor::APickupActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Scene Component를 루트로 생성
    SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    RootComponent = SceneComponent;

    // 메시 컴포넌트를 자식으로 생성
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

    // 물리 설정
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetEnableGravity(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MeshComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
    MeshComponent->SetMobility(EComponentMobility::Movable);

    // 상호작용 영역
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);

    // 기본값
    bDrawDebug = true;
    MeshRotation = FRotator::ZeroRotator;
}

void APickupActor::ApplyMeshRotation()
{
    if (MeshComponent)
    {
        // MeshComponent만 회전
        MeshComponent->SetRelativeRotation(MeshRotation);
    }
}

void APickupActor::BeginPlay()
{
    Super::BeginPlay();

    if (!ItemClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickupActor %s has no ItemClass set!"), *GetName());
    }

    // 강제 회전 적용
    ApplyMeshRotation();

    UE_LOG(LogTemp, Warning, TEXT("PickupActor BeginPlay - Mesh Rotation Applied: %s"),
        *MeshComponent->GetRelativeRotation().ToString());
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
    UE_LOG(LogTemp, Warning, TEXT("PickupActor::Interact called on %s by %s"),
        *GetName(), *Interactor->GetName());

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