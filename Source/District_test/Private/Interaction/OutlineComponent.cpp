#include "Interaction/OutlineComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

UOutlineComponent::UOutlineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UOutlineComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (Owner)
    {
        Owner->GetComponents<UPrimitiveComponent>(CachedPrimitiveComponents);
        UE_LOG(LogTemp, Warning, TEXT("OutlineComponent on %s: Found %d primitive components"),
            *Owner->GetName(), CachedPrimitiveComponents.Num());
    }
}
void UOutlineComponent::ShowOutline()
{
    UE_LOG(LogTemp, Warning, TEXT("ShowOutline called!"));

    if (!OutlineMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("OutlineMaterial is NULL!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("OutlineMaterial: %s"), *OutlineMaterial->GetName());

    for (UPrimitiveComponent* Component : CachedPrimitiveComponents)
    {
        if (Component)
        {
            if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
            {
                UE_LOG(LogTemp, Warning, TEXT("Setting overlay on StaticMesh: %s"), *StaticMesh->GetName());
                StaticMesh->OverlayMaterial = OutlineMaterial;
                StaticMesh->MarkRenderStateDirty();
            }
            else if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Component))
            {
                UE_LOG(LogTemp, Warning, TEXT("Setting overlay on SkeletalMesh: %s"), *SkeletalMesh->GetName());
                SkeletalMesh->OverlayMaterial = OutlineMaterial;
                SkeletalMesh->MarkRenderStateDirty();
            }
        }
    }
}

void UOutlineComponent::HideOutline()
{
    UE_LOG(LogTemp, Warning, TEXT("HideOutline called!"));

    for (UPrimitiveComponent* Component : CachedPrimitiveComponents)
    {
        if (Component)
        {
            if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
            {
                UE_LOG(LogTemp, Warning, TEXT("Clearing overlay on StaticMesh: %s"), *StaticMesh->GetName());
                StaticMesh->OverlayMaterial = nullptr;
                StaticMesh->MarkRenderStateDirty();
            }
            else if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Component))
            {
                UE_LOG(LogTemp, Warning, TEXT("Clearing overlay on SkeletalMesh: %s"), *SkeletalMesh->GetName());
                SkeletalMesh->OverlayMaterial = nullptr;
                SkeletalMesh->MarkRenderStateDirty();
            }
        }
    }
}