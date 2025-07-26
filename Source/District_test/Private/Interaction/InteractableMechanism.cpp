// Fill out your copyright notice in the Description page of Project Settings.
#include "Interaction/InteractableMechanism.h"
#include "Gameplay/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "Misc/OutputDeviceNull.h"
#include "Kismet/GameplayStatics.h"

AInteractableMechanism::AInteractableMechanism()
{
    PrimaryActorTick.bCanEverTick = true;

    // �޽� ������Ʈ�� ��Ʈ�� ����
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // �޽� ������Ʈ ����
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetGenerateOverlapEvents(true);
    MeshComponent->SetMobility(EComponentMobility::Movable);

    // ���ͷ��� ���� ���� (�޽ÿ� ���̱�)
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetGenerateOverlapEvents(true);
    InteractionSphere->SetMobility(EComponentMobility::Movable);

    // �ð��� ������� ���� ���� �������� ���̰� ����
    InteractionSphere->bHiddenInGame = false;

    // �⺻ �Ӽ� ����
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

    UE_LOG(LogTemp, Error, TEXT("=== %s BeginPlay ==="), *GetName());
    UE_LOG(LogTemp, Error, TEXT("MeshComponent Collision: %d"),
        MeshComponent ? static_cast<int32>(MeshComponent->GetCollisionEnabled()) : -1);
    UE_LOG(LogTemp, Error, TEXT("InteractionSphere Collision: %d"),
        InteractionSphere ? static_cast<int32>(InteractionSphere->GetCollisionEnabled()) : -1);
}

void AInteractableMechanism::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AInteractableMechanism::Interact_Implementation(AActor* Interactor)
{
    // ��ȣ�ۿ� ���� ���� Ȯ��
    if (!bCanInteract)
    {
        return;
    }

    // ��Ŀ���� ������ ���� ó��
    switch (MechanismType)
    {
    case EMechanismType::Door:
        HandleDoorInteraction(Interactor);
        break;

    case EMechanismType::Widget:
        HandleWidgetInteraction(Interactor);
        break;

    default:
        // �⺻ ��ȣ�ۿ�
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
    // ��ȣ�ۿ� �Ҹ� ���
    if (InteractionSound)
    {
        UGameplayStatics::PlaySound2D(this, InteractionSound);
    }

    // �������Ʈ �̺�Ʈ ȣ��� ���� ǥ�� ó��
    OnShowWidget(Cast<APlayerController>(Interactor->GetInstigatorController()));
}




void AInteractableMechanism::HideInteractionWidget()
{
    // �Է� ��常 �����ϰ� ���� ó���� �������Ʈ����
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    // �������Ʈ �̺�Ʈ ȣ��
    OnHideWidget();
}

void AInteractableMechanism::OnWidgetInteractionSuccess()
{
    UE_LOG(LogTemp, Error, TEXT("=== OnWidgetInteractionSuccess called ==="));
    bIsCompleted = true;
    OnInteractionSuccess(UGameplayStatics::GetPlayerPawn(this, 0));
    UE_LOG(LogTemp, Error, TEXT("=== OnInteractionSuccess called ==="));
}

void AInteractableMechanism::OnWidgetInteractionFailed()
{
    // ���� �����
    HideInteractionWidget();

    // ���� ó��
    OnInteractionFailed(UGameplayStatics::GetPlayerPawn(this, 0));
}


bool AInteractableMechanism::CanInteract_Implementation(AActor* Interactor)
{
    // �⺻ ��ȣ�ۿ� ���� ���� Ȯ��
    if (!bCanInteract)
    {
        UE_LOG(LogTemp, Warning, TEXT("bCanInteract is FALSE"));
        return false;
    }

    // �̹� �Ϸ�Ǿ��� ���� �Ұ����ϸ� ��ȣ�ۿ� ����
    if (bIsCompleted && !bCanBeUsedAgain)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mechanism already completed and cannot be reused"));
        return false;
    }

    // ��Ŀ���� ������ �߰� �˻�
    switch (MechanismType)
    {
    case EMechanismType::Door:
    {
        // 1. Ű�е� ���� Ȯ��
        if (bRequiresKeypadCompletion)
        {
            bool bAllKeypadCompleted = CheckRequiredKeypads();
            if (!bAllKeypadCompleted)
            {
                UE_LOG(LogTemp, Warning, TEXT("Required keypads not completed"));
                return false;
            }
        }

        // 2. Ű ������ ���� Ȯ��
        if (bRequiresKey)
        {
            UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
            if (!InventoryComponent)
            {
                UE_LOG(LogTemp, Warning, TEXT("No inventory component found"));
                return false;
            }

            UItem* Key = InventoryComponent->FindItemByName(RequiredKeyName);
            if (!Key)
            {
                return false;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Door interaction allowed"));
    }
    break;

    case EMechanismType::Widget:
        // ������ ��� �Ϸ� ���°� �ƴ϶�� ��ȣ�ۿ� ����
        if (bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget mechanism already completed"));
            return bCanBeUsedAgain; // ���� ���� ���ο� ���� ����
        }

        UE_LOG(LogTemp, Warning, TEXT("Widget mechanism - interaction allowed"));
        return true;

    default:
        // �⺻ Ÿ���� ��� �׻� ��ȣ�ۿ� ����
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("CanInteract returning TRUE"));
    return true;
}

FString AInteractableMechanism::GetInteractionText_Implementation()
{
    return InteractionText;
}

EInteractionType AInteractableMechanism::GetInteractionType_Implementation()
{
    // ��Ŀ���� ������ ���� ������ ���ͷ��� Ÿ�� ��ȯ
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

// InteractableMechanism.cpp�� �߰��� ���� �Լ���

bool AInteractableMechanism::CheckRequiredKeypads()
{
    if (RequiredKeypadIDs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No required keypads specified"));
        return true; // �䱸������ ������ ���
    }

    // ������ ��� InteractableMechanism ���� �˻�
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableMechanism::StaticClass(), FoundActors);

    for (const FString& RequiredID : RequiredKeypadIDs)
    {
        bool bFoundCompleted = false;

        for (AActor* Actor : FoundActors)
        {
            AInteractableMechanism* Mechanism = Cast<AInteractableMechanism>(Actor);
            if (Mechanism &&
                Mechanism->MechanismType == EMechanismType::Widget &&
                Mechanism->MechanismID == RequiredID &&
                Mechanism->bIsCompleted)
            {
                bFoundCompleted = true;
                break;
            }
        }

        if (!bFoundCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Required keypad not completed: %s"), *RequiredID);
            return false;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("All required keypads completed"));
    return true;
}

void AInteractableMechanism::HandleDoorInteraction(AActor* Interactor)
{
    UE_LOG(LogTemp, Error, TEXT("=== HandleDoorInteraction called ==="));

    if (InteractionSound)
    {
        UGameplayStatics::PlaySound2D(this, InteractionSound);
    }

    ToggleDoor();
    OnInteractionSuccess(Interactor);
}

void AInteractableMechanism::AutoCloseDoor()
{
    if (bIsOpen)
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto-closing door"));
        ToggleDoor();
    }
}

void AInteractableMechanism::ToggleDoor()
{
    UE_LOG(LogTemp, Error, TEXT("=== ToggleDoor called, bIsOpen: %s ==="), bIsOpen ? TEXT("TRUE") : TEXT("FALSE"));

    if (bIsOpen)
    {
        bIsOpen = false;
        UE_LOG(LogTemp, Error, TEXT("=== Calling OnDoorClosed ==="));
        OnDoorClosed();
    }
    else
    {
        bIsOpen = true;
        UE_LOG(LogTemp, Error, TEXT("=== Calling OnDoorOpened ==="));
        OnDoorOpened();
    }
}