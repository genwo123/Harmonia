#include "Interaction/InteractableMechanism.h"
#include "Gameplay/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
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

    InteractionText = TEXT("Interact");
    bRequiresKey = false;
    bCanInteract = true;
    bIsOpen = false;
    MechanismType = EMechanismType::Widget;
    WidgetSubType = EWidgetSubType::Keypad;
    MaxAttempts = 3;
    OpenAngle = 90.0f;
    OpenSpeed = 2.0f;
}

void AInteractableMechanism::BeginPlay()
{
    Super::BeginPlay();

    // Door/Shutter가 필요한 메커니즘들을 구독
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

void AInteractableMechanism::HideInteractionWidget()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    OnHideWidget();
}

void AInteractableMechanism::OnWidgetInteractionSuccess()
{
    bIsCompleted = true;

    // 연결된 문/셔터들에게 직접 신호 전송
    for (AInteractableMechanism* Door : ConnectedDoors)
    {
        if (Door)
        {
            Door->OnRequiredMechanismCompleted(MechanismID);
        }
    }

    OnInteractionSuccess(UGameplayStatics::GetPlayerPawn(this, 0));
}

void AInteractableMechanism::OnWidgetInteractionFailed()
{
    HideInteractionWidget();
    OnInteractionFailed(UGameplayStatics::GetPlayerPawn(this, 0));
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
        // 필수 메커니즘 체크
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

        // 키 아이템 체크
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
        case EWidgetSubType::Quiz:
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

    // 모든 필수 메커니즘이 완료됐는지 확인
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