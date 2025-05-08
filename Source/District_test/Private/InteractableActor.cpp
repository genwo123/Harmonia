// InteractableActor.cpp
#include "InteractableActor.h"

AInteractableActor::AInteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // �޽� ������Ʈ ����
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();
}

void AInteractableActor::Interact_Implementation(AActor* Interactor)
{
    // �������Ʈ���� �������̵��� �� �ֵ��� �⺻ ������ ��� ����
}

bool AInteractableActor::CanInteract_Implementation(AActor* Interactor)
{
    return bCanBeInteracted;
}

FString AInteractableActor::GetInteractionText_Implementation()
{
    return InteractionText;
}

EInteractionType AInteractableActor::GetInteractionType_Implementation()
{
    return InteractionType;
}