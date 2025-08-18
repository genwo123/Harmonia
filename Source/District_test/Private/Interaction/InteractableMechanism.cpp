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
    bIsCompleted = true;
    OnInteractionSuccess(UGameplayStatics::GetPlayerPawn(this, 0));

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

        return false;
    }

    // �̹� �Ϸ�Ǿ��� ���� �Ұ����ϸ� ��ȣ�ۿ� ����
    if (bIsCompleted && !bCanBeUsedAgain)
    {

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
    
                return false;
            }
        }

        // 2. Ű ������ ���� Ȯ��
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
        // ������ ��� �Ϸ� ���°� �ƴ϶�� ��ȣ�ۿ� ����
        if (bIsCompleted)
        {

            return bCanBeUsedAgain; // ���� ���� ���ο� ���� ����
        }


        return true;

    default:
        // �⺻ Ÿ���� ��� �׻� ��ȣ�ۿ� ����
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
 
            return false;
        }
    }


    return true;
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

void AInteractableMechanism::AutoCloseDoor()
{
    if (bIsOpen)
    {

        ToggleDoor();
    }
}

void AInteractableMechanism::ToggleDoor()
{


    if (bIsOpen)
    {
        bIsOpen = false;

        OnDoorClosed();
    }
    else
    {
        bIsOpen = true;

        OnDoorOpened();
    }
}