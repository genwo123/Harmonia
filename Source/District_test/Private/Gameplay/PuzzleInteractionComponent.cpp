#include "Gameplay/PuzzleInteractionComponent.h"
#include "Gameplay/Pedestal.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/HamoniaCharacter.h" 
#include "Components/StaticMeshComponent.h"

UPuzzleInteractionComponent::UPuzzleInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    bCanBePickedUp = true;
    bCanBePlacedOnPedestal = true;
    bStartWithPhysicsDisabled = true;
    bEnablePhysicsWhenDropped = true;
    CurrentPedestal = nullptr;
    HoldingActor = nullptr;

    bOriginalSimulatePhysics = false;
    OriginalCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
    bIsPhysicsSimulating = false;
}

void UPuzzleInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetEnableGravity(false);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}

void UPuzzleInteractionComponent::SetupInitialPhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        bOriginalSimulatePhysics = PrimComp->IsSimulatingPhysics();
        OriginalCollisionEnabled = PrimComp->GetCollisionEnabled();

        if (bStartWithPhysicsDisabled)
        {
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }
}

void UPuzzleInteractionComponent::EnablePhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(true);
        PrimComp->SetEnableGravity(true);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        PrimComp->SetMassOverrideInKg(NAME_None, 5.0f);
        PrimComp->SetLinearDamping(0.1f);
        PrimComp->SetAngularDamping(0.1f);

        bIsPhysicsSimulating = true;
    }
}

void UPuzzleInteractionComponent::DisablePhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetEnableGravity(false);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

        PrimComp->SetRelativeLocation(FVector::ZeroVector);
        PrimComp->SetRelativeRotation(FRotator::ZeroRotator);

        bIsPhysicsSimulating = false;
    }
}


bool UPuzzleInteractionComponent::PickUp(AActor* Picker)
{
    if (!bCanBePickedUp || HoldingActor)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        if (PrimComp->IsSimulatingPhysics())
        {
            PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
            PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        }

        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetEnableGravity(false);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimComp->SetVisibility(true, true);
        Owner->SetActorHiddenInGame(false);
    }

    Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    HoldingActor = Picker;

    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(Picker);
    if (Character && Character->HeldObjectAttachPoint)
    {
        FAttachmentTransformRules AttachRules(
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepWorld,
            false
        );

        Owner->AttachToComponent(Character->HeldObjectAttachPoint, AttachRules);
        Owner->SetActorRelativeLocation(FVector::ZeroVector);
        Owner->SetActorRelativeRotation(FRotator::ZeroRotator);
    }

    USoundBase* SoundToPlay = nullptr;
    if (SoundType == EPuzzleObjectSoundType::Type1)
    {
        SoundToPlay = PickupSound_Type1;
    }
    else if (SoundType == EPuzzleObjectSoundType::Type2)
    {
        SoundToPlay = PickupSound_Type2;
    }

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Owner->GetActorLocation());
    }

    return true;
}

bool UPuzzleInteractionComponent::PutDown(FVector Location, FRotator Rotation)
{
    if (!HoldingActor)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    FVector WorldLocation = Owner->GetActorLocation();
    FRotator WorldRotation = Owner->GetActorRotation();

    Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    Owner->SetActorLocation(WorldLocation);
    Owner->SetActorRotation(WorldRotation);

    if (PrimComp)
    {
        PrimComp->SetVisibility(true, true);
        Owner->SetActorHiddenInGame(false);

        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
        PrimComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

        PrimComp->SetMassOverrideInKg(NAME_None, 10.0f, true);
        PrimComp->SetEnableGravity(true);
        PrimComp->SetSimulatePhysics(true);
        PrimComp->WakeAllRigidBodies();
    }

    USoundBase* SoundToPlay = nullptr;
    if (SoundType == EPuzzleObjectSoundType::Type1)
    {
        SoundToPlay = DropSound_Type1;
    }
    else if (SoundType == EPuzzleObjectSoundType::Type2)
    {
        SoundToPlay = DropSound_Type2;
    }

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, WorldLocation);
    }

    HoldingActor = nullptr;
    return true;
}

void UPuzzleInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (HoldingActor && !Owner->GetAttachParentActor())
    {
        ACharacter* Character = Cast<ACharacter>(HoldingActor);
        if (Character)
        {
            FVector CameraLocation;
            FRotator CameraRotation;
            Character->GetActorEyesViewPoint(CameraLocation, CameraRotation);

            FVector NewLocation = CameraLocation +
                (CameraRotation.Vector() * HoldOffset.X) +
                (FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y) * HoldOffset.Y) +
                (FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Z) * HoldOffset.Z);

            Owner->SetActorLocation(NewLocation);

            if (bMatchCameraRotation)
            {
                Owner->SetActorRotation(CameraRotation);
            }
        }
    }
}

bool UPuzzleInteractionComponent::PlaceOnPedestal(APedestal* Pedestal)
{
    if (!Pedestal || !bCanBePlacedOnPedestal)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    if (HoldingActor)
    {
        Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

    DisablePhysics();

    if (Pedestal->PlaceObject(Owner))
    {
        CurrentPedestal = Pedestal;
        HoldingActor = nullptr;
        return true;
    }

    return false;
}

bool UPuzzleInteractionComponent::RemoveFromPedestal()
{
    if (!CurrentPedestal)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    AActor* RemovedObject = CurrentPedestal->RemoveObject();

    if (RemovedObject == Owner)
    {
        CurrentPedestal = nullptr;
        return true;
    }

    return false;
}