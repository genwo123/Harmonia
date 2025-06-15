// PuzzleInteractionComponent.cpp
#include "Gameplay/PuzzleInteractionComponent.h"
#include "Gameplay/Pedestal.h"
#include "GameFramework/Character.h"

UPuzzleInteractionComponent::UPuzzleInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 기본값 설정
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

                // 오프셋 적용
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

    // 만약 이 오브젝트가 받침대에 있다면 먼저 제거
    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    // 물리 비활성화 (필요시)
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetComponentByClass(UPrimitiveComponent::StaticClass()));
    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 들고 있는 액터 설정
    HoldingActor = Picker;

    // 오브젝트가 독립적으로 위치할 수 있도록 분리
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

    // 새 위치와 회전 설정
    Owner->SetActorLocation(Location);
    Owner->SetActorRotation(Rotation);

    // 물리 다시 활성화 (필요시)
    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetComponentByClass(UPrimitiveComponent::StaticClass()));
    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(true);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // 들고 있는 액터 초기화
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

    // 현재 받침대가 있다면 제거
    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    // 새 받침대에 배치
    if (Pedestal->PlaceObject(Owner))
    {
        CurrentPedestal = Pedestal;

        // 더 이상 들고 있지 않음
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

    // 받침대에서 제거
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