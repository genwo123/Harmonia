#include "Core/PlayerInteractionComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/OutlineComponent.h"
#include "Gameplay/InventoryComponent.h"
#include "Gameplay/Pedestal.h"
#include "Gameplay/PuzzleInteractionComponent.h"
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
    DropDistance = 150.0f;

    bIsLookingAtInteractable = false;
    CurrentInteractableActor = nullptr;
    CurrentInteractableNPC = nullptr;
    CameraRef = nullptr;
    InventoryRef = nullptr;
    HeldObjectAttachPoint = nullptr;
    bIgnoreInteractionCheck = false;
}

void UPlayerInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHeldObject = nullptr;

    if (AActor* Owner = GetOwner())
    {
        if (!CameraRef)
        {
            CameraRef = Owner->FindComponentByClass<UCameraComponent>();
        }

        if (!InventoryRef)
        {
            InventoryRef = Owner->FindComponentByClass<UInventoryComponent>();
        }

        if (!HeldObjectAttachPoint)
        {
            HeldObjectAttachPoint = Cast<USceneComponent>(
                Owner->GetDefaultSubobjectByName(TEXT("HeldObjectAttachPoint"))
            );
        }
    }
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
    if (bIgnoreInteractionCheck)
    {
        UE_LOG(LogTemp, Error, TEXT("Interaction check is BLOCKED"));
        return;
    }

    if (!CameraRef)
    {
        UE_LOG(LogTemp, Error, TEXT("CameraRef is NULL"));
        return;
    }

    FVector Start = CameraRef->GetComponentLocation();
    FVector End = Start + (CameraRef->GetForwardVector() * InteractionDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    // LineTrace 자체가 작동하는지 확인
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );

    UE_LOG(LogTemp, Warning, TEXT("LineTrace Hit: %s"), bHit ? TEXT("YES") : TEXT("NO"));
    if (bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());
    }

    AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;

    if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
    {
        if (HitActor != CurrentInteractableActor)
        {
            if (CurrentInteractableActor)
            {
                UOutlineComponent* OldOutline = CurrentInteractableActor->FindComponentByClass<UOutlineComponent>();
                if (OldOutline)
                {
                    OldOutline->HideOutline();
                }
            }

            CurrentInteractableActor = HitActor;

            UOutlineComponent* NewOutline = CurrentInteractableActor->FindComponentByClass<UOutlineComponent>();
            if (NewOutline)
            {
                NewOutline->ShowOutline();
            }

            bool bCanInteract = IInteractableInterface::Execute_CanInteract(CurrentInteractableActor, GetOwner());
            FString InteractionText = IInteractableInterface::Execute_GetInteractionText(CurrentInteractableActor);
            EInteractionType InteractionType = IInteractableInterface::Execute_GetInteractionType(CurrentInteractableActor);

            bIsLookingAtInteractable = bCanInteract;
            CurrentInteractionText = InteractionText;
            CurrentInteractionType = InteractionType;

            OnInteractionChanged.Broadcast(bCanInteract, InteractionText, InteractionType);
        }
    }
    else
    {
        if (CurrentInteractableActor)
        {
            UOutlineComponent* OldOutline = CurrentInteractableActor->FindComponentByClass<UOutlineComponent>();
            if (OldOutline)
            {
                OldOutline->HideOutline();
            }
            CurrentInteractableActor = nullptr;
        }

        if (bIsLookingAtInteractable)
        {
            bIsLookingAtInteractable = false;
            CurrentInteractionText = TEXT("");
            CurrentInteractionType = EInteractionType::Default;
            OnInteractionChanged.Broadcast(false, TEXT(""), EInteractionType::Default);
        }
    }
}


void UPlayerInteractionComponent::PerformInteraction()
{
    if (!GetOwner())
    {
        return;
    }

    AActor* HeldObject = GetHeldObject();

    if (HeldObject)
    {
        if (CurrentInteractableActor)
        {
            APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
            if (Pedestal)
            {
                bool bPlaced = PlaceOnPedestal(Pedestal, HeldObject);
                if (bPlaced)
                {
                    OnInteractionChanged.Broadcast(false, TEXT(""), EInteractionType::Default);
                }
                return;
            }
        }
        else
        {
            OnShowWarningMessage.Broadcast(TEXT("Use G to drop object"));
        }
    }
    else
    {
        if (CurrentInteractableActor)
        {
            if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
            {
                APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
                if (Pedestal)
                {
                    bool bPickedUp = PickUpFromPedestal(Pedestal);
                    if (bPickedUp)
                    {
                        OnInteractionChanged.Broadcast(false, TEXT(""), EInteractionType::Default);
                    }
                    return;
                }

                bool bPickedUp = PickUpObject(CurrentInteractableActor);
                if (bPickedUp)
                {
                    OnInteractionChanged.Broadcast(false, TEXT(""), EInteractionType::Default);
                }
                else
                {
                    IInteractableInterface::Execute_Interact(CurrentInteractableActor, GetOwner());
                }
            }
        }
    }
}

bool UPlayerInteractionComponent::PlaceOnPedestal(APedestal* Pedestal, AActor* ObjectToPlace)
{
    if (!Pedestal || !ObjectToPlace)
    {
        return false;
    }

    UPuzzleInteractionComponent* PuzzleComp = ObjectToPlace->FindComponentByClass<UPuzzleInteractionComponent>();
    if (!PuzzleComp)
    {
        return false;
    }

    bool bResult = PuzzleComp->PlaceOnPedestal(Pedestal);

    if (bResult)
    {
        if (CurrentHeldObject == ObjectToPlace)
        {
            CurrentHeldObject = nullptr;
        }

        OnInteractionChanged.Broadcast(false, TEXT(""), EInteractionType::Default);
    }

    return bResult;
}

bool UPlayerInteractionComponent::PickUpFromPedestal(APedestal* Pedestal)
{
    if (!Pedestal)
    {
        return false;
    }

    AActor* RemovedObject = Pedestal->RemoveObject();
    if (!RemovedObject)
    {
        return false;
    }

    UPuzzleInteractionComponent* PuzzleComp = RemovedObject->FindComponentByClass<UPuzzleInteractionComponent>();
    if (!PuzzleComp)
    {
        return false;
    }

    bool bResult = PuzzleComp->PickUp(GetOwner());

    if (bResult)
    {
        CurrentHeldObject = RemovedObject;
    }

    return bResult;
}

bool UPlayerInteractionComponent::PickUpObject(AActor* ObjectToPickUp)
{
    if (!ObjectToPickUp)
    {
        return false;
    }

    if (CurrentHeldObject)
    {
        OnShowWarningMessage.Broadcast(TEXT("Already holding an object!"));
        return false;
    }

    AActor* HeldViaAttach = GetHeldObject();
    if (HeldViaAttach)
    {
        OnShowWarningMessage.Broadcast(TEXT("Already holding an object!"));
        return false;
    }

    UPuzzleInteractionComponent* PuzzleComp = ObjectToPickUp->FindComponentByClass<UPuzzleInteractionComponent>();
    if (!PuzzleComp)
    {
        return false;
    }

    bool bResult = PuzzleComp->PickUp(GetOwner());

    if (bResult)
    {
        CurrentHeldObject = ObjectToPickUp;
    }

    return bResult;
}

void UPlayerInteractionComponent::DropHeldObject()
{
    AActor* HeldObject = GetHeldObject();

    if (!HeldObject)
    {
        return;
    }

    UPuzzleInteractionComponent* PuzzleComp = HeldObject->FindComponentByClass<UPuzzleInteractionComponent>();
    if (!PuzzleComp || !CameraRef)
    {
        return;
    }

    FVector DropLocation = CameraRef->GetComponentLocation() + (CameraRef->GetForwardVector() * DropDistance);
    FRotator DropRotation = HeldObject->GetActorRotation();

    PuzzleComp->PutDown(DropLocation, DropRotation);

    CurrentHeldObject = nullptr;
}

void UPlayerInteractionComponent::RotatePedestal()
{
    if (!CurrentInteractableActor)
    {
        return;
    }

    APedestal* Pedestal = FindPedestalFromActor(CurrentInteractableActor);
    if (Pedestal && Pedestal->bCanRotate)
    {
        Pedestal->Rotate(45.0f);
    }
}

void UPlayerInteractionComponent::PushPedestal()
{
    if (!CameraRef || !CurrentInteractableActor)
    {
        return;
    }

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
    {
        return false;
    }

    return false;
}

APedestal* UPlayerInteractionComponent::FindPedestalFromActor(AActor* Actor) const
{
    if (!Actor)
    {
        return nullptr;
    }

    APedestal* Pedestal = Cast<APedestal>(Actor);
    if (Pedestal)
    {
        return Pedestal;
    }

    AActor* ParentActor = Actor->GetAttachParentActor();
    if (ParentActor)
    {
        Pedestal = Cast<APedestal>(ParentActor);
        if (Pedestal)
        {
            return Pedestal;
        }
    }

    return nullptr;
}

UItem* UPlayerInteractionComponent::GetCurrentHeldInventoryItem() const
{
    if (!InventoryRef)
    {
        return nullptr;
    }

    return nullptr;
}

AActor* UPlayerInteractionComponent::GetHeldObject() const
{
    if (CurrentHeldObject && IsValid(CurrentHeldObject))
    {
        return CurrentHeldObject;
    }

    if (!HeldObjectAttachPoint)
    {
        return nullptr;
    }

    TArray<USceneComponent*> ChildComponents;
    HeldObjectAttachPoint->GetChildrenComponents(false, ChildComponents);

    for (USceneComponent* ChildComp : ChildComponents)
    {
        if (ChildComp)
        {
            AActor* Owner = ChildComp->GetOwner();
            if (Owner && IsValid(Owner))
            {
                USceneComponent* RootComp = Owner->GetRootComponent();
                if (RootComp && RootComp->GetAttachParent() == HeldObjectAttachPoint)
                {
                    return Owner;
                }
            }
        }
    }

    return nullptr;
}

void UPlayerInteractionComponent::EnableOutline(AActor* Actor)
{
    if (!Actor || !bEnableOutline)
    {
        return;
    }

    UOutlineComponent* OutlineComp = Actor->FindComponentByClass<UOutlineComponent>();
    if (OutlineComp)
    {
        OutlineComp->ShowOutline();
    }

    APedestal* Pedestal = Cast<APedestal>(Actor);
    if (Pedestal)
    {
        Pedestal->UpdateOutlineForPlacedObject(true);
    }
}

void UPlayerInteractionComponent::DisableOutline(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    UOutlineComponent* OutlineComp = Actor->FindComponentByClass<UOutlineComponent>();
    if (OutlineComp)
    {
        OutlineComp->HideOutline();
    }

    APedestal* Pedestal = Cast<APedestal>(Actor);
    if (Pedestal)
    {
        Pedestal->UpdateOutlineForPlacedObject(false);
    }
}

void UPlayerInteractionComponent::DrawDebugInteractionLine()
{
    if (!CameraRef)
    {
        return;
    }

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
        DebugLineThickness
    );
}

void UPlayerInteractionComponent::ReEnableInteractionCheck()
{
    bIgnoreInteractionCheck = false;
}