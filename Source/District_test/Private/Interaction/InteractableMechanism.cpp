#include "Interaction/InteractableMechanism.h"
#include "Gameplay/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"           
#include "Components/WidgetComponent.h" 
#include "Kismet/GameplayStatics.h"

AInteractableMechanism::AInteractableMechanism()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetGenerateOverlapEvents(true);
    MeshComponent->SetMobility(EComponentMobility::Movable);

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetGenerateOverlapEvents(true);
    InteractionSphere->SetMobility(EComponentMobility::Movable);
    InteractionSphere->bHiddenInGame = false;


    InteractionPromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPromptWidgetComponent"));
    InteractionPromptWidgetComponent->SetupAttachment(RootComponent);
    InteractionPromptWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
    InteractionPromptWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    InteractionPromptWidgetComponent->SetDrawSize(FVector2D(200.f, 50.f));
    InteractionPromptWidgetComponent->SetVisibility(false);


    InteractionText = TEXT("Interact");
    bRequiresKey = false;
    bCanInteract = true;
    bIsOpen = false;
    MechanismType = EMechanismType::Widget;
    WidgetSubType = EWidgetSubType::Keypad;
    MaxAttempts = 3;
    OpenAngle = 90.0f;
    OpenSpeed = 2.0f;
    HintLevelNumber = 1;
}

void AInteractableMechanism::BeginPlay()
{
    Super::BeginPlay();

    if (MechanismType == EMechanismType::Door && RequiredMechanisms.Num() > 0)
    {
        for (AInteractableMechanism* RequiredMech : RequiredMechanisms)
        {
            if (RequiredMech)
            {
                RequiredMech->OnMechanismCompleted.AddDynamic(
                    this, &AInteractableMechanism::OnRequiredMechanismCompleted);
            }
        }
    }
}

void AInteractableMechanism::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AInteractableMechanism::Interact_Implementation(AActor* Interactor)
{
    if (!bCanInteract)
    {
        return;
    }

    switch (MechanismType)
    {
    case EMechanismType::Door:
        HandleDoorInteraction(Interactor);
        break;

    case EMechanismType::Widget:
        HandleWidgetInteraction(Interactor);
        break;

    default:
        if (InteractionSound)
        {
            UGameplayStatics::PlaySound2D(this, InteractionSound);
        }
        OnInteractionSuccess(Interactor);
        break;
    }
}

void AInteractableMechanism::HandleWidgetInteraction(AActor* Interactor)
{
    if (InteractionSound)
    {
        UGameplayStatics::PlaySound2D(this, InteractionSound);
    }

    OnShowWidget(Cast<APlayerController>(Interactor->GetInstigatorController()));
}


void AInteractableMechanism::OnWidgetInteractionSuccess()
{
    bIsCompleted = true;

    for (AInteractableMechanism* Door : ConnectedDoors)
    {
        if (Door)
        {
            Door->OnRequiredMechanismCompleted(MechanismID);
        }
    }

    AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0);

    OnWidgetSuccess.Broadcast(Player);

    OnInteractionSuccess(Player);
}

void AInteractableMechanism::OnWidgetInteractionFailed()
{
    HideInteractionWidget();

    AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0);

    OnWidgetFailed.Broadcast(Player);

    OnInteractionFailed(Player);
}

bool AInteractableMechanism::CanInteract_Implementation(AActor* Interactor)
{
    if (!bCanInteract)
    {
        return false;
    }

    if (bIsCompleted && !bCanBeUsedAgain)
    {
        return false;
    }

    switch (MechanismType)
    {
    case EMechanismType::Door:
    {
        if (RequiredMechanisms.Num() > 0)
        {
            for (AInteractableMechanism* RequiredMech : RequiredMechanisms)
            {
                if (RequiredMech && !RequiredMech->bIsCompleted)
                {
                    return false;
                }
            }
        }

        if (bRequiresKey)
        {
            UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
            if (!InventoryComponent)
            {
                return false;
            }

            UItem* Key = InventoryComponent->FindItemByName(RequiredKeyName);
            if (!Key)
            {
                return false;
            }
        }
    }
    break;

    case EMechanismType::Widget:
        if (bIsCompleted)
        {
            return bCanBeUsedAgain;
        }
        return true;

    default:
        break;
    }

    return true;
}

FString AInteractableMechanism::GetInteractionText_Implementation()
{
    return InteractionText;
}

EInteractionType AInteractableMechanism::GetInteractionType_Implementation()
{
    switch (MechanismType)
    {
    case EMechanismType::Door:
        return EInteractionType::Open;

    case EMechanismType::Widget:
        switch (WidgetSubType)
        {
        case EWidgetSubType::Keypad:
            return EInteractionType::Use;
        case EWidgetSubType::MiniGame:
            return EInteractionType::Activate;
        case EWidgetSubType::Hint:
            return EInteractionType::Read;
        default:
            return EInteractionType::Default;
        }

    default:
        return EInteractionType::Default;
    }
}

void AInteractableMechanism::HandleDoorInteraction(AActor* Interactor)
{
    if (InteractionSound)
    {
        UGameplayStatics::PlaySound2D(this, InteractionSound);
    }

    ToggleDoor();
    OnInteractionSuccess(Interactor);
}

void AInteractableMechanism::ToggleDoor()
{
    if (!bIsOpen)
    {
        bIsOpen = true;
        OnDoorOpened();
    }
}

void AInteractableMechanism::OnRequiredMechanismCompleted(FString CompletedMechanismID)
{
    if (!CompletedRequiredMechanisms.Contains(CompletedMechanismID))
    {
        CompletedRequiredMechanisms.Add(CompletedMechanismID);
    }

    bool bAllCompleted = true;
    for (AInteractableMechanism* RequiredMech : RequiredMechanisms)
    {
        if (RequiredMech && !RequiredMech->bIsCompleted)
        {
            bAllCompleted = false;
            break;
        }
    }

    if (bAllCompleted)
    {
        OnAllRequiredMechanismsCompleted();
    }
}

void AInteractableMechanism::ShowInteractionWidget_Implementation()
{
    if (InteractionPromptWidgetComponent)
    {
        InteractionPromptWidgetComponent->SetVisibility(true);
    }
}


void AInteractableMechanism::HideInteractionWidget_Implementation()
{
    if (InteractionPromptWidgetComponent)
    {
        InteractionPromptWidgetComponent->SetVisibility(false);
    }
}

void AInteractableMechanism::OpenConnectedDoors()
{
    if (bIsCompleted && !bCanBeUsedAgain)
    {
        return;
    }

    if (InteractionSound)
    {
        UGameplayStatics::PlaySound2D(this, InteractionSound);
    }

    bIsCompleted = true;

    for (AInteractableMechanism* Door : ConnectedDoors)
    {
        if (Door)
        {
            Door->OnRequiredMechanismCompleted(MechanismID);
        }
    }
}