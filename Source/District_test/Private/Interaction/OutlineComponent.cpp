#include "Interaction/OutlineComponent.h"
#include "Components/StaticMeshComponent.h"

UOutlineComponent::UOutlineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    OutlineThickness = 1.0f;
    OutlineColor = FLinearColor::White;
}

void UOutlineComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TargetMesh = Owner->FindComponentByClass<UStaticMeshComponent>();

    if (!TargetMesh)
    {
        return;
    }

    TargetMesh->SetRenderCustomDepth(false);
    TargetMesh->SetCustomDepthStencilValue(1);
}

void UOutlineComponent::ShowOutline()
{
    if (!TargetMesh)
    {
        return;
    }

    TargetMesh->SetRenderCustomDepth(true);
}

void UOutlineComponent::HideOutline()
{
    if (!TargetMesh)
    {
        return;
    }

    TargetMesh->SetRenderCustomDepth(false);
}