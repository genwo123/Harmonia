#include "Core/PlayerInteractionComponent.h"
#include "Character/HamoniaCharacter.h"
#include "Character/Unia.h"
#include "Gameplay/InventoryComponent.h"
#include "Gameplay/PuzzleInteractionComponent.h"
#include "Gameplay/Pedestal.h"
#include "Gameplay/PickupActor.h"
#include "Gameplay/PuzzleStarter.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractableMechanism.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Core/DialogueManagerComponent.h"

UPlayerInteractionComponent::UPlayerInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsLookingAtInteractable = false;
    CurrentInteractableActor = nullptr;
    CurrentInteractableNPC = nullptr;
}

void UPlayerInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentInteractableActor && !IsValid(CurrentInteractableActor))
    {
        bIsLookingAtInteractable = false;
        CurrentInteractableActor = nullptr;
        CurrentInteractionText.Empty();
    }

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

void UPlayerInteractionComponent::PerformInteraction()
{
    AHamoniaCharacter* OwnerCharacter = Cast<AHamoniaCharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    UDialogueManagerComponent* DialogueManager = OwnerCharacter->GetDialogueManager();
    if (DialogueManager && DialogueManager->bIsInDialogue)
    {
        OwnerCharacter->OnDialogueProgressRequested.Broadcast();
        return;
    }

    if (CurrentInteractableNPC)
    {
        CurrentInteractableNPC->HandlePlayerInteraction();
        return;
    }

    AActor* HeldObject = GetHeldObject();
    if (HeldObject)
    {
        UPuzzleInteractionComponent* HeldItemComp = HeldObject->FindComponentByClass<UPuzzleInteractionComponent>();
        if (HeldItemComp)
        {
            if (bIsLookingAtInteractable && CurrentInteractableActor)
            {
                APedestal* Pedestal = FindPedestalFromActor(CurrentInteractableActor);
                if (Pedestal)
                {
                    if (Pedestal->GetPlacedObject() != nullptr)
                        return;

                    if (!Pedestal->CanPlaceObjectByFilter(HeldObject))
                        return;

                    HeldItemComp->PlaceOnPedestal(Pedestal);

                    if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
                    {
                        IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, OwnerCharacter);
                    }
                    return;
                }
            }

            FVector DropLocation = HeldObjectAttachPoint->GetComponentLocation();
            HeldItemComp->PutDown(DropLocation, GetOwner()->GetActorRotation());

            if (bIsLookingAtInteractable && CurrentInteractableActor &&
                CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
            {
                IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, OwnerCharacter);
            }
            return;
        }
    }

    if (bIsLookingAtInteractable && CurrentInteractableActor)
    {
        UItem* HeldInventoryItem = GetCurrentHeldInventoryItem();
        if (HeldInventoryItem && HandleInventoryItemInteraction(HeldInventoryItem, CurrentInteractableActor))
        {
            return;
        }

        APedestal* Pedestal = Cast<APedestal>(CurrentInteractableActor);
        if (Pedestal)
        {
            AActor* ObjectOnPedestal = Pedestal->GetPlacedObject();
            if (ObjectOnPedestal)
            {
                UPuzzleInteractionComponent* InteractionComp = ObjectOnPedestal->FindComponentByClass<UPuzzleInteractionComponent>();
                if (InteractionComp)
                {
                    Pedestal->RemoveObject();
                    InteractionComp->PickUp(OwnerCharacter);

                    if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
                    {
                        IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, OwnerCharacter);
                    }
                    return;
                }
            }

            IInteractableInterface::Execute_Interact(Pedestal, OwnerCharacter);

            if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
            {
                IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, OwnerCharacter);
            }
            return;
        }

        UPuzzleInteractionComponent* InteractionComp = CurrentInteractableActor->FindComponentByClass<UPuzzleInteractionComponent>();
        if (InteractionComp && InteractionComp->bCanBePickedUp)
        {
            APedestal* ParentPedestal = FindPedestalFromActor(CurrentInteractableActor);
            if (ParentPedestal && ParentPedestal->GetPlacedObject() == CurrentInteractableActor)
            {
                ParentPedestal->RemoveObject();
            }

            InteractionComp->PickUp(OwnerCharacter);

            if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
            {
                IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, OwnerCharacter);
            }
            return;
        }

        IInteractableInterface::Execute_Interact(CurrentInteractableActor, OwnerCharacter);

        if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            IInteractableInterface::Execute_OnQuestInteract(CurrentInteractableActor, OwnerCharacter);
        }
    }
}

void UPlayerInteractionComponent::RotateObject()
{
    if (bIsLookingAtInteractable && CurrentInteractableActor)
    {
        APedestal* Pedestal = FindPedestalFromActor(CurrentInteractableActor);
        if (Pedestal)
        {
            Pedestal->Rotate();
        }
    }
}

void UPlayerInteractionComponent::PushObject()
{
    if (bIsLookingAtInteractable && CurrentInteractableActor)
    {
        APedestal* Pedestal = FindPedestalFromActor(CurrentInteractableActor);
        if (Pedestal && CameraRef)
        {
            FVector Direction = CameraRef->GetForwardVector();
            Direction.Z = 0;
            Direction.Normalize();
            Pedestal->Push(Direction);
        }
    }
}

void UPlayerInteractionComponent::OnEKeyPressed()
{
    if (InventoryRef && InventoryRef->bIsInventoryOpen)
    {
        InventoryRef->UseSelectedItem();
    }
    else
    {
        PushObject();
    }
}

void UPlayerInteractionComponent::CheckForInteractables()
{
    if (!CameraRef)
        return;

    if (CurrentInteractableActor && IsValid(CurrentInteractableActor))
    {
        if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            IInteractableInterface::Execute_HideInteractionWidget(CurrentInteractableActor);
        }
    }

    FVector Start = CameraRef->GetComponentLocation();
    FVector End = Start + (CameraRef->GetForwardVector() * InteractionDistance);
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;

    AActor* HeldObject = GetHeldObject();
    if (HeldObject)
    {
        QueryParams.AddIgnoredActor(HeldObject);
    }

    ECollisionChannel TraceChannel = ECC_Visibility;

    bIsLookingAtInteractable = false;
    CurrentInteractableActor = nullptr;
    CurrentInteractionText.Empty();
    CurrentInteractionType = EInteractionType::Default;

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TraceChannel, QueryParams);

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();

        if (HitActor && HitActor != HeldObject && HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            bool bCanInteractResult = IInteractableInterface::Execute_CanInteract(HitActor, GetOwner());

            if (bCanInteractResult)
            {
                bIsLookingAtInteractable = true;
                CurrentInteractableActor = HitActor;
                CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(HitActor);
                CurrentInteractionType = IInteractableInterface::Execute_GetInteractionType(HitActor);
            }
        }
    }

    if (!bIsLookingAtInteractable)
    {
        float ClosestDistance = InteractionDistance;
        AActor* ClosestActor = nullptr;

        for (TActorIterator<AUnia> It(GetWorld()); It; ++It)
        {
            AUnia* Unia = *It;
            if (IsValid(Unia))
            {
                float Distance = FVector::Distance(GetOwner()->GetActorLocation(), Unia->GetActorLocation());
                if (Distance < ClosestDistance && Unia->CanInteract_Implementation(GetOwner()))
                {
                    ClosestDistance = Distance;
                    ClosestActor = Unia;
                }
            }
        }

        for (TActorIterator<APedestal> It(GetWorld()); It; ++It)
        {
            APedestal* Pedestal = *It;
            if (IsValid(Pedestal))
            {
                float Distance = FVector::Distance(GetOwner()->GetActorLocation(), Pedestal->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestActor = Pedestal;
                }
            }
        }

        for (TActorIterator<APickupActor> It(GetWorld()); It; ++It)
        {
            APickupActor* PickupActor = *It;
            if (IsValid(PickupActor) && PickupActor != HeldObject)
            {
                float Distance = FVector::Distance(GetOwner()->GetActorLocation(), PickupActor->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestActor = PickupActor;
                }
            }
        }

        if (ClosestActor)
        {
            bIsLookingAtInteractable = true;
            CurrentInteractableActor = ClosestActor;
            CurrentInteractionText = IInteractableInterface::Execute_GetInteractionText(ClosestActor);
            CurrentInteractionType = IInteractableInterface::Execute_GetInteractionType(ClosestActor);
        }
    }

    if (bIsLookingAtInteractable && CurrentInteractableActor)
    {
        if (CurrentInteractableActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
        {
            IInteractableInterface::Execute_ShowInteractionWidget(CurrentInteractableActor);
        }
    }

    OnInteractionChanged.Broadcast(bIsLookingAtInteractable, CurrentInteractionText, CurrentInteractionType);
}

void UPlayerInteractionComponent::DrawDebugInteractionLine()
{
    if (!bShowDebugLines || !CameraRef)
        return;

    FVector Start = CameraRef->GetComponentLocation();
    FVector End = Start + (CameraRef->GetForwardVector() * InteractionDistance);

    FColor LineColor = bIsLookingAtInteractable ? FColor::Green : FColor::Red;

    DrawDebugLine(GetWorld(), Start, End, LineColor, false, -1.0f, 0, 1.0f);

    if (bIsLookingAtInteractable && CurrentInteractableActor)
    {
        DrawDebugSphere(
            GetWorld(),
            CurrentInteractableActor->GetActorLocation(),
            30.0f,
            12,
            FColor::Green,
            false,
            -1.0f,
            0,
            1.0f
        );
    }
}

void UPlayerInteractionComponent::SetCurrentInteractableNPC(AUnia* NPC)
{
    if (NPC && !InteractableNPCs.Contains(NPC))
    {
        InteractableNPCs.Add(NPC);
    }
    CurrentInteractableNPC = NPC;
}

void UPlayerInteractionComponent::RemoveInteractableNPC(AUnia* NPC)
{
    InteractableNPCs.Remove(NPC);

    if (CurrentInteractableNPC == NPC)
    {
        CurrentInteractableNPC = InteractableNPCs.Num() > 0 ? InteractableNPCs[0] : nullptr;
    }
}

AActor* UPlayerInteractionComponent::GetHeldObject() const
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        UPuzzleInteractionComponent* InteractionComp = Actor->FindComponentByClass<UPuzzleInteractionComponent>();

        if (InteractionComp && InteractionComp->HoldingActor == GetOwner())
        {
            return Actor;
        }
    }

    return nullptr;
}

APedestal* UPlayerInteractionComponent::FindPedestalFromActor(AActor* Actor) const
{
    if (!Actor)
        return nullptr;

    APedestal* Pedestal = Cast<APedestal>(Actor);
    if (Pedestal)
        return Pedestal;

    AActor* ParentActor = Actor->GetAttachParentActor();
    while (ParentActor)
    {
        Pedestal = Cast<APedestal>(ParentActor);
        if (Pedestal)
            return Pedestal;

        ParentActor = ParentActor->GetAttachParentActor();
    }

    return nullptr;
}

UItem* UPlayerInteractionComponent::GetCurrentHeldInventoryItem() const
{
    if (InventoryRef && InventoryRef->bIsInventoryOpen)
    {
        return InventoryRef->GetSelectedItem();
    }
    return nullptr;
}

bool UPlayerInteractionComponent::HandleInventoryItemInteraction(UItem* Item, AActor* TargetActor)
{
    if (!Item || !TargetActor)
        return false;

    APuzzleStarter* PuzzleStarter = Cast<APuzzleStarter>(TargetActor);
    if (PuzzleStarter)
    {
        FName ItemTag = FName(*Item->Name);
        bool bInserted = PuzzleStarter->TryInsertCoreByTag(ItemTag);

        if (bInserted && InventoryRef)
        {
            InventoryRef->RemoveItem(Item);
        }
        return bInserted;
    }

    if (Item->Name.Contains("Key"))
    {
        AInteractableMechanism* Door = Cast<AInteractableMechanism>(TargetActor);
        if (Door && Door->MechanismType == EMechanismType::Door)
        {
            if (Door->RequiredKeyName.Equals(Item->Name, ESearchCase::IgnoreCase))
            {
                IInteractableInterface::Execute_Interact(Door, GetOwner());
                return true;
            }
        }
    }

    if (Item->Name.Contains("Tool"))
    {
        APedestal* Pedestal = Cast<APedestal>(TargetActor);
        if (Pedestal)
        {
            Item->Use(Cast<AHamoniaCharacter>(GetOwner()));
            return true;
        }
    }

    if (Item->bCanBeUsed)
    {
        Item->Use(Cast<AHamoniaCharacter>(GetOwner()));
        return true;
    }

    return false;
}