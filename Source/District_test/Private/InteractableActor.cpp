// InteractableActor.cpp
#include "InteractableActor.h"

AInteractableActor::AInteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // 메시 컴포넌트 생성
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();
}

void AInteractableActor::Interact_Implementation(AActor* Interactor)
{
    // 블루프린트에서 오버라이드할 수 있도록 기본 구현은 비어 있음
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