#include "Core/PlayerInteractionComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/OutlineComponent.h"
#include "Gameplay/InventoryComponent.h"
#include "Gameplay/Pedestal.h"
#include "Gameplay/Item.h"
#include "Character/Unia.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UPlayerInteractionComponent::UPlayerInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    InteractionDistance = 400.0f;
    bShowDebugLines = false;
    bEnableOutline = true;

    bIsLookingAtInteractable = false;
    CurrentInteractableActor = nullptr;
    CurrentInteractableNPC = nullptr;
    CameraRef = nullptr;
    InventoryRef = nullptr;
    HeldObjectAttachPoint = nullptr;
}

void UPlayerInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        if (!CameraRef)
        {
            CameraRef = Owner->FindComponentByClass<UCameraComponent>();
            UE_LOG(LogTemp, Warning, TEXT(" Auto-found CameraRef: %s"), CameraRef ? *CameraRef->GetName() : TEXT("NULL"));
        }

        if (!InventoryRef)
        {
            InventoryRef = Owner->FindComponentByClass<UInventoryComponent>();
            UE_LOG(LogTemp, Warning, TEXT(" Auto-found InventoryRef: %s"), InventoryRef ? TEXT("OK") : TEXT("NULL"));
        }

        if (!HeldObjectAttachPoint)
        {
            HeldObjectAttachPoint = Cast<USceneComponent>(
                Owner->GetDefaultSubobjectByName(TEXT("HeldObjectAttachPoint"))
            );
            UE_LOG(LogTemp, Warning, TEXT(" Auto-found HeldObjectAttachPoint: %s"), HeldObjectAttachPoint ? *HeldObjectAttachPoint->GetName() : TEXT("NULL"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("=== PlayerInteractionComponent BeginPlay ==="));
    UE_LOG(LogTemp, Warning, TEXT("CameraRef: %s"), CameraRef ? *CameraRef->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("InventoryRef: %s"), InventoryRef ? TEXT("OK") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("HeldObjectAttachPoint: %s"), HeldObjectAttachPoint ? *HeldObjectAttachPoint->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("bShowDebugLines: %s"), bShowDebugLines ? TEXT("TRUE") : TEXT("FALSE"));
    UE_LOG(LogTemp, Warning, TEXT("bEnableOutline: %s"), bEnableOutline ? TEXT("TRUE") : TEXT("FALSE"));
}


void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    CheckForInteractables();

    if (bShowDebugLines)
    {
        DrawDebugInteractionLine();
    }
}

void UPlayerInteractionComponent::SetupReferences(UCameraComponent* Camera, UInventoryComponent* Inventory, USceneComponent* HeldAttachPoint)
{
    CameraRef = Camera;
    InventoryRef = Inventory;
    HeldObjectAttachPoint = HeldAttachPoint;
}

void UPlayerInteractionComponent::CheckForInteractables()
{
    if (!CameraRef)
    {
        UE_LOG(LogTemp, Error, TEXT("CheckForInteractables: CameraRef is NULL!"));
        return;
    }

    FVector StartLocation = CameraRef->GetComponentLocation();
    FVector ForwardVector = CameraRef->GetForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * InteractionDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    AActor* HeldObject = GetHeldObject();
    if (HeldObject)
    {
        QueryParams.AddIgnoredActor(HeldObject);
    }

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());
    }

    AActor* PreviousInteractableActor = CurrentInteractableActor;
    bool bPreviousInteractableState = bIsLookingAtInteractable;

    bIsLookingAtInteractable = false;
    CurrentInteractableActor = nullptr;
    CurrentInteractionText = FString();
    CurrentInteractionType = EInteractionType::Default;

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();

        if (HitActor && HitActor != HeldObject &&
            HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            UE_LOG(LogTemp, Warning, TEXT("HitActor has InteractableInterface!"));

            bool bCanInteractResult = IInteractableInterface::Execute_CanInteract(HitActor, GetOwner());

            UE_LOG(LogTemp, Warning, TEXT("CanInteract: %s"), bCanInteractResult ? TEXT("TRUE") : TEXT("FALSE"));

            if (bCanInteractResult)
            {
                bIsLookingAtInteractable = true;
                CurrentInteractableActor = HitActor;
                CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(HitActor);
                CurrentInteractionType = IInteractableInterface::Execute_GetInteractionType(HitActor);

                UE_LOG(LogTemp, Warning, TEXT("Interactable found: %s"), *CurrentInteractionText);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("HitActor does NOT have InteractableInterface!"));
        }
    }

    if (PreviousInteractableActor != CurrentInteractableActor)
    {
        if (PreviousInteractableActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Disabling outline on: %s"), *PreviousInteractableActor->GetName());
            DisableOutline(PreviousInteractableActor);
        }

        if (CurrentInteractableActor && bEnableOutline)
        {
            UE_LOG(LogTemp, Warning, TEXT("Enabling outline on: %s"), *CurrentInteractableActor->GetName());
            EnableOutline(CurrentInteractableActor);
        }
    }

    if (bPreviousInteractableState != bIsLookingAtInteractable)
    {
        OnInteractionChanged.Broadcast(
            bIsLookingAtInteractable,
            CurrentInteractionText,
            CurrentInteractionType
        );
    }
}

void UPlayerInteractionComponent::PerformInteraction()
{
    if (CurrentInteractableNPC)
    {
        CurrentInteractableNPC->HandlePlayerInteraction();
        return;
    }

    if (!bIsLookingAtInteractable || !CurrentInteractableActor)
        return;

    if (!CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        return;

    bool bCanInteract = IInteractableInterface::Execute_CanInteract(CurrentInteractableActor, GetOwner());
    if (!bCanInteract)
        return;

    UItem* HeldItem = GetCurrentHeldInventoryItem();
    if (HeldItem)
    {
        bool bHandled = HandleInventoryItemInteraction(HeldItem, CurrentInteractableActor);
        if (bHandled)
            return;
    }

    IInteractableInterface::Execute_Interact(CurrentInteractableActor, GetOwner());
}

void UPlayerInteractionComponent::OnEKeyPressed()
{
    RotateObject();
}

void UPlayerInteractionComponent::RotateObject()
{
    if (!CurrentInteractableActor)
        return;

    APedestal* Pedestal = FindPedestalFromActor(CurrentInteractableActor);
    if (Pedestal && Pedestal->bCanRotate)
    {
        Pedestal->Rotate(45.0f);
    }
}

void UPlayerInteractionComponent::PushObject()
{
    if (!CameraRef || !CurrentInteractableActor)
        return;

    FVector PushDirection = CameraRef->GetForwardVector();
    PushDirection.Z = 0;
    PushDirection.Normalize();

    APedestal* Pedestal = FindPedestalFromActor(CurrentInteractableActor);
    if (Pedestal)
    {
        Pedestal->Push(PushDirection);
    }
}

void UPlayerInteractionComponent::SetCurrentInteractableNPC(AUnia* NPC)
{
    CurrentInteractableNPC = NPC;

    if (NPC && !InteractableNPCs.Contains(NPC))
    {
        InteractableNPCs.Add(NPC);
    }
}

void UPlayerInteractionComponent::RemoveInteractableNPC(AUnia* NPC)
{
    if (CurrentInteractableNPC == NPC)
    {
        CurrentInteractableNPC = nullptr;
    }

    InteractableNPCs.Remove(NPC);
}

bool UPlayerInteractionComponent::HandleInventoryItemInteraction(UItem* Item, AActor* TargetActor)
{
    if (!Item || !TargetActor || !InventoryRef)
        return false;

    return false;
}

APedestal* UPlayerInteractionComponent::FindPedestalFromActor(AActor* Actor) const
{
    if (!Actor)
        return nullptr;

    APedestal* Pedestal = Cast<APedestal>(Actor);
    if (Pedestal)
        return Pedestal;

    AActor* ParentActor = Actor->GetAttachParentActor();
    if (ParentActor)
    {
        Pedestal = Cast<APedestal>(ParentActor);
        if (Pedestal)
            return Pedestal;
    }

    return nullptr;
}

UItem* UPlayerInteractionComponent::GetCurrentHeldInventoryItem() const
{
    if (!InventoryRef)
        return nullptr;

    // TODO: InventoryComponent에 실제 함수 확인 후 수정
    return nullptr;
}

AActor* UPlayerInteractionComponent::GetHeldObject() const
{
    if (!HeldObjectAttachPoint)
        return nullptr;

    TArray<USceneComponent*> ChildComponents;
    HeldObjectAttachPoint->GetChildrenComponents(false, ChildComponents);

    if (ChildComponents.Num() > 0 && ChildComponents[0])
    {
        return ChildComponents[0]->GetOwner();
    }

    return nullptr;
}

void UPlayerInteractionComponent::EnableOutline(AActor* Actor)
{
    if (!Actor || !bEnableOutline)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnableOutline: Actor NULL or Outline disabled"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EnableOutline: %s"), *Actor->GetName());

    UOutlineComponent* OutlineComp = Actor->FindComponentByClass<UOutlineComponent>();
    if (OutlineComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found OutlineComponent!"));
        OutlineComp->ShowOutline();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OutlineComponent NOT FOUND on %s!"), *Actor->GetName());
    }
}
void UPlayerInteractionComponent::DisableOutline(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("DisableOutline: Actor NULL"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("DisableOutline: %s"), *Actor->GetName());

    UOutlineComponent* OutlineComp = Actor->FindComponentByClass<UOutlineComponent>();
    if (OutlineComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found OutlineComponent for disable!"));
        OutlineComp->HideOutline();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OutlineComponent NOT FOUND on %s for disable!"), *Actor->GetName());
    }
}

void UPlayerInteractionComponent::DrawDebugInteractionLine()
{
    if (!CameraRef)
        return;

    FVector StartLocation = CameraRef->GetComponentLocation();
    FVector ForwardVector = CameraRef->GetForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * InteractionDistance);

    FColor LineColor = bIsLookingAtInteractable ? FColor::Green : FColor::Red;

    DrawDebugLine(
        GetWorld(),
        StartLocation,
        EndLocation,
        LineColor,
        false,
        -1.0f,
        0,
        2.0f
    );
}