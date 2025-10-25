#include "Gameplay/PuzzleInteractionComponent.h"
#include "Gameplay/Pedestal.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
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
}

void UPuzzleInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    SetupInitialPhysics();
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
    }
}

void UPuzzleInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (HoldingActor)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            if (!Owner->GetAttachParentActor())
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
        if (PrimComp != Owner->GetRootComponent())
        {
            FVector MeshWorldLocation = PrimComp->GetComponentLocation();
            FRotator MeshWorldRotation = PrimComp->GetComponentRotation();

            Owner->SetActorLocation(MeshWorldLocation);
            Owner->SetActorRotation(MeshWorldRotation);

            PrimComp->SetRelativeLocation(FVector::ZeroVector);
            PrimComp->SetRelativeRotation(FRotator::ZeroRotator);
        }

        PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetEnableGravity(false);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

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

        if (PrimComp && PrimComp != Owner->GetRootComponent())
        {
            PrimComp->SetRelativeLocation(FVector::ZeroVector);
            PrimComp->SetRelativeRotation(FRotator::ZeroRotator);
        }
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

    FVector DropLocation = Owner->GetActorLocation();

    Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (PrimComp && PrimComp != Owner->GetRootComponent())
    {
        FVector MeshWorldLocation = PrimComp->GetComponentLocation();
        Owner->SetActorLocation(MeshWorldLocation);
        DropLocation = MeshWorldLocation;

        PrimComp->SetRelativeLocation(FVector::ZeroVector);
        PrimComp->SetRelativeRotation(FRotator::ZeroRotator);
    }
    else
    {
        Owner->SetActorLocation(DropLocation);
    }

    if (PrimComp)
    {
        PrimComp->SetVisibility(true, true);
        Owner->SetActorHiddenInGame(false);

        if (bEnablePhysicsWhenDropped)
        {
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            PrimComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

            GetWorld()->GetTimerManager().SetTimerForNextTick([Owner, PrimComp]()
                {
                    if (PrimComp && PrimComp->IsValidLowLevel())
                    {
                        if (PrimComp != Owner->GetRootComponent())
                        {
                            FVector RootLocation = Owner->GetActorLocation();
                            PrimComp->SetWorldLocation(RootLocation);
                            PrimComp->SetRelativeLocation(FVector::ZeroVector);
                        }

                        PrimComp->SetEnableGravity(true);
                        PrimComp->SetSimulatePhysics(true);
                        PrimComp->WakeAllRigidBodies();
                    }
                });
        }
    }

    HoldingActor = nullptr;
    return true;
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