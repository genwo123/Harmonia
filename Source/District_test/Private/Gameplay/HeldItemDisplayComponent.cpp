#include "Gameplay/HeldItemDisplayComponent.h"
#include "Gameplay/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"

UHeldItemDisplayComponent::UHeldItemDisplayComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentDisplayedItem = nullptr;
    HeldItemMesh = nullptr;
}

void UHeldItemDisplayComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHeldItemDisplayComponent::SetupMeshComponent(UStaticMeshComponent* MeshComp)
{
    HeldItemMesh = MeshComp;
}

void UHeldItemDisplayComponent::UpdateDisplay(UItem* NewItem, bool bInventoryOpen)
{
    CurrentDisplayedItem = NewItem;

    if (NewItem && bInventoryOpen)
    {
        ShowItemMesh(NewItem);
    }
    else
    {
        HideItemMesh();
    }
}

void UHeldItemDisplayComponent::ShowItemMesh(UItem* Item)
{
    if (Item && HeldItemMesh)
    {
        HeldItemMesh->SetVisibility(true);
    }

    ShowHeldItemMeshBP(Item);
}

void UHeldItemDisplayComponent::HideItemMesh()
{
    if (HeldItemMesh)
    {
        HeldItemMesh->SetVisibility(false);
    }

    HideHeldItemMeshBP();
}