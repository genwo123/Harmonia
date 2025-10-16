#include "Gameplay/PuzzleInteractionComponent.h"
#include "Gameplay/Pedestal.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Character/HamoniaCharacter.h" 
#include "Components/StaticMeshComponent.h"

UPuzzleInteractionComponent::UPuzzleInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 기본값 설정
    bCanBePickedUp = true;
    bCanBePlacedOnPedestal = true;
    bStartWithPhysicsDisabled = true;
    bEnablePhysicsWhenDropped = true;
    CurrentPedestal = nullptr;
    HoldingActor = nullptr;

    // 원래 설정 초기화
    bOriginalSimulatePhysics = false;
    OriginalCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
}

void UPuzzleInteractionComponent::BeginPlay()
{
    Super::BeginPlay();
    SetupInitialPhysics();
}

void UPuzzleInteractionComponent::SetupInitialPhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        // StaticMeshComponent 찾기
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        // 원래 설정 저장
        bOriginalSimulatePhysics = PrimComp->IsSimulatingPhysics();
        OriginalCollisionEnabled = PrimComp->GetCollisionEnabled();

        if (bStartWithPhysicsDisabled)
        {
            // 초기에는 피직스 비활성화 (에디터에서 안정적)
            PrimComp->SetSimulatePhysics(false);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);


        }
    }
}

void UPuzzleInteractionComponent::EnablePhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(true);
        PrimComp->SetEnableGravity(true);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // 적당한 물리 속성 설정
        PrimComp->SetMassOverrideInKg(NAME_None, 5.0f);
        PrimComp->SetLinearDamping(0.1f);
        PrimComp->SetAngularDamping(0.1f);


    }
}

void UPuzzleInteractionComponent::DisablePhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
    if (!PrimComp)
    {
        PrimComp = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (PrimComp)
    {
        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);


    }
}

void UPuzzleInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Attach 방식을 사용하면 Tick에서 위치 업데이트 불필요
    if (HoldingActor)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            // Attach되어 있으면 자동으로 따라감
            // HeldObjectAttachPoint가 없는 경우에만 수동 업데이트
            if (!Owner->GetAttachParentActor())
            {
                ACharacter* Character = Cast<ACharacter>(HoldingActor);
                if (Character)
                {
                    FVector CameraLocation;
                    FRotator CameraRotation;
                    Character->GetActorEyesViewPoint(CameraLocation, CameraRotation);

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
}

bool UPuzzleInteractionComponent::PickUp(AActor* Picker)
{
    if (!bCanBePickedUp)
    {
        return false;
    }

    if (HoldingActor)
    {
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    // 받침대에서 제거 (이미 Interact에서 처리했지만 안전하게 한 번 더)
    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    // 피직스 비활성화
    DisablePhysics();

    // HoldingActor 설정
    HoldingActor = Picker;

    // 플레이어의 HeldObjectAttachPoint 찾기
    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(Picker);
    if (Character && Character->HeldObjectAttachPoint)
    {
        // AttachPoint에 부착
        Owner->AttachToComponent(
            Character->HeldObjectAttachPoint,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );

        // 상대 위치/회전 초기화 (AttachPoint의 위치 그대로 사용)
        Owner->SetActorRelativeLocation(FVector::ZeroVector);
        Owner->SetActorRelativeRotation(FRotator::ZeroRotator);
    }
    else
    {
        // HeldObjectAttachPoint가 없으면 기존 방식 (수동 위치 업데이트)
        Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

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

    // 현재 월드 위치/회전 저장 (Attach된 상태에서의 위치)
    FVector CurrentWorldLocation = Owner->GetActorLocation();
    FRotator CurrentWorldRotation = Owner->GetActorRotation();

    // Detach
    Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // KeepWorldTransform 규칙으로 Detach했으므로 이미 현재 위치 유지됨
    // 추가 설정 불필요

    // 내려놓을 때 피직스 활성화
    if (bEnablePhysicsWhenDropped)
    {
        EnablePhysics();
    }

    // HoldingActor 초기화
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

    // 플레이어로부터 Detach
    if (HoldingActor)
    {
        Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

    // 새 받침대에 배치
    if (Pedestal->PlaceObject(Owner))
    {
        CurrentPedestal = Pedestal;

        // 받침대에 놓을 때는 피직스 비활성화
        DisablePhysics();

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
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {

        return false;
    }

    // 받침대에서 제거
    AActor* RemovedObject = CurrentPedestal->RemoveObject();

    if (RemovedObject == Owner)
    {
        CurrentPedestal = nullptr;
 
        return true;
    }

    
    return false;
}