// PuzzleInteractionComponent.cpp
#include "Gameplay/PuzzleInteractionComponent.h"
#include "Gameplay/Pedestal.h"
#include "GameFramework/Character.h"

UPuzzleInteractionComponent::UPuzzleInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // �⺻�� ����
    bCanBePickedUp = true;
    bCanBePlacedOnPedestal = true;
    CurrentPedestal = nullptr;
    HoldingActor = nullptr;
}

void UPuzzleInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPuzzleInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (HoldingActor)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            ACharacter* Character = Cast<ACharacter>(HoldingActor);
            if (Character)
            {
                FVector CameraLocation;
                FRotator CameraRotation;
                Character->GetActorEyesViewPoint(CameraLocation, CameraRotation);

                // ������ ����
                FVector NewLocation = CameraLocation +
                    (CameraRotation.Vector() * HoldOffset.X) +
                    (FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y) * HoldOffset.Y) +
                    (FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Z) * HoldOffset.Z);

                Owner->SetActorLocation(NewLocation);

                if (bMatchCameraRotation)
                {
                    Owner->SetActorRotation(CameraRotation);
                }
            }
        }
    }
}

bool UPuzzleInteractionComponent::PickUp(AActor* Picker)
{
    if (!bCanBePickedUp || HoldingActor)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    // ���� �� ������Ʈ�� ��ħ�뿡 �ִٸ� ���� ����
    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    // ���� ��Ȱ��ȭ (�ʿ��)
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetComponentByClass(UPrimitiveComponent::StaticClass()));
    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // ��� �ִ� ���� ����
    HoldingActor = Picker;

    // ������Ʈ�� ���������� ��ġ�� �� �ֵ��� �и�
    Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    return true;
}

bool UPuzzleInteractionComponent::PutDown(FVector Location, FRotator Rotation)
{
    if (!HoldingActor)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    // �� ��ġ�� ȸ�� ����
    Owner->SetActorLocation(Location);
    Owner->SetActorRotation(Rotation);

    // ���� �ٽ� Ȱ��ȭ (�ʿ��)
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetComponentByClass(UPrimitiveComponent::StaticClass()));
    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(true);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // ��� �ִ� ���� �ʱ�ȭ
    HoldingActor = nullptr;

    return true;
}

bool UPuzzleInteractionComponent::PlaceOnPedestal(APedestal* Pedestal)
{
    if (!Pedestal || !bCanBePlacedOnPedestal)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    // ���� ��ħ�밡 �ִٸ� ����
    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    // �� ��ħ�뿡 ��ġ
    if (Pedestal->PlaceObject(Owner))
    {
        CurrentPedestal = Pedestal;

        // �� �̻� ��� ���� ����
        HoldingActor = nullptr;

        return true;
    }

    return false;
}

bool UPuzzleInteractionComponent::RemoveFromPedestal()
{
    if (!CurrentPedestal)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveFromPedestal failed - no current pedestal"));
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveFromPedestal failed - no owner"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Attempting to remove %s from pedestal %s"),
        *Owner->GetName(), *CurrentPedestal->GetName());

    // ��ħ�뿡�� ����
    AActor* RemovedObject = CurrentPedestal->RemoveObject();

    UE_LOG(LogTemp, Warning, TEXT("RemoveObject returned: %s"),
        RemovedObject ? *RemovedObject->GetName() : TEXT("None"));

    if (RemovedObject == Owner)
    {
        CurrentPedestal = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("RemoveFromPedestal successful"));
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("RemoveFromPedestal failed - wrong object returned"));
    return false;
}