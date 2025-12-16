#include "Core/OutlineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UOutlineComponent::UOutlineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UOutlineComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UOutlineComponent::ShowOutline()
{
    if (OutlineMeshes.Num() > 0)
    {
        for (UStaticMeshComponent* OutlineMesh : OutlineMeshes)
        {
            if (OutlineMesh)
            {
                OutlineMesh->SetVisibility(true);
            }
        }
        return;
    }

    CreateOutlineMeshes();
}

void UOutlineComponent::HideOutline()
{
    for (UStaticMeshComponent* OutlineMesh : OutlineMeshes)
    {
        if (OutlineMesh)
        {
            OutlineMesh->SetVisibility(false);
        }
    }
}

void UOutlineComponent::CreateOutlineMeshes()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TArray<UStaticMeshComponent*> OriginalMeshes;
    Owner->GetComponents<UStaticMeshComponent>(OriginalMeshes);

    for (UStaticMeshComponent* OriginalMesh : OriginalMeshes)
    {
        if (!OriginalMesh || !OriginalMesh->GetStaticMesh())
        {
            continue;
        }

        UStaticMeshComponent* OutlineMesh = NewObject<UStaticMeshComponent>(Owner);
        OutlineMesh->SetStaticMesh(OriginalMesh->GetStaticMesh());
        OutlineMesh->AttachToComponent(OriginalMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
        OutlineMesh->SetRelativeLocation(FVector::ZeroVector);
        OutlineMesh->SetRelativeRotation(FRotator::ZeroRotator);
        OutlineMesh->SetRelativeScale3D(FVector(OutlineThickness));
        OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        OutlineMesh->SetCastShadow(false);
        OutlineMesh->bRenderInMainPass = true;
        OutlineMesh->bRenderInDepthPass = false;
        OutlineMesh->RegisterComponent();

        UMaterialInterface* MatToUse = OutlineMaterial;
        if (!MatToUse)
        {
            MatToUse = LoadObject<UMaterialInterface>(
                nullptr,
                TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")
            );
        }

        if (MatToUse)
        {
            UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(MatToUse, this);
            if (DynMat)
            {
                DynMat->SetVectorParameterValue(TEXT("Color"), OutlineColor);
                for (int32 i = 0; i < OutlineMesh->GetNumMaterials(); i++)
                {
                    OutlineMesh->SetMaterial(i, DynMat);
                }
            }
        }

        OutlineMeshes.Add(OutlineMesh);
    }
}

void UOutlineComponent::DestroyOutlineMeshes()
{
    for (UStaticMeshComponent* OutlineMesh : OutlineMeshes)
    {
        if (OutlineMesh)
        {
            OutlineMesh->DestroyComponent();
        }
    }
    OutlineMeshes.Empty();
}