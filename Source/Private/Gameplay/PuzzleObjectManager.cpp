#include "Gameplay/PuzzleObjectManager.h"
#include "Gameplay/PuzzleInteractionComponent.h"
#include "Kismet/GameplayStatics.h"

APuzzleObjectManager::APuzzleObjectManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APuzzleObjectManager::BeginPlay()
{
    Super::BeginPlay();

    FindAllPuzzleObjects();
    DeactivateAllPuzzleObjects();
}

void APuzzleObjectManager::FindAllPuzzleObjects()
{
    ManagedObjects.Empty();

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->FindComponentByClass<UPuzzleInteractionComponent>())
        {
            ManagedObjects.Add(Actor);
        }
    }
}

void APuzzleObjectManager::ActivateAllPuzzleObjects()
{
    SetObjectsActive(true);
}

void APuzzleObjectManager::DeactivateAllPuzzleObjects()
{
    SetObjectsActive(false);
}

void APuzzleObjectManager::SetObjectsActive(bool bActive)
{
    for (AActor* Obj : ManagedObjects)
    {
        if (!Obj) continue;

        UPuzzleInteractionComponent* PuzzleComp =
            Obj->FindComponentByClass<UPuzzleInteractionComponent>();

        if (PuzzleComp)
        {
            PuzzleComp->SetPuzzleActive(bActive);
        }
    }
}