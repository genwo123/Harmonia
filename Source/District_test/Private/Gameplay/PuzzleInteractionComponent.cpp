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
        // Physics 완전히 비활성화
        PrimComp->SetSimulatePhysics(false);
        PrimComp->SetEnableGravity(false);  // 추가
        PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

        // 속도 초기화 (중요!) - 추가
        PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

        UE_LOG(LogTemp, Warning, TEXT("[DisablePhysics] Physics disabled for %s"), *Owner->GetName());
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

bool UPuzzleInteractionComponent::PutDown(FVector Location, FRotator Rotation)
{
    if (!HoldingActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PutDown] No HoldingActor"));
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PutDown] No Owner"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[PutDown] Before Detach - Owner: %s, HoldingActor: %s"),
        *Owner->GetName(), *HoldingActor->GetName());

    FVector CurrentWorldLocation = Owner->GetActorLocation();
    FRotator CurrentWorldRotation = Owner->GetActorRotation();

    Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    UE_LOG(LogTemp, Warning, TEXT("[PutDown] Detached"));

    if (bEnablePhysicsWhenDropped)
    {
        EnablePhysics();
        UE_LOG(LogTemp, Warning, TEXT("[PutDown] Physics Enabled"));
    }

    HoldingActor = nullptr;
    UE_LOG(LogTemp, Warning, TEXT("[PutDown] HoldingActor set to nullptr, bCanBePickedUp: %s"),
        bCanBePickedUp ? TEXT("True") : TEXT("False"));

    return true;
}

bool UPuzzleInteractionComponent::PickUp(AActor* Picker)
{
    UE_LOG(LogTemp, Warning, TEXT("[PickUp] Called on %s - bCanBePickedUp: %s"),
        *GetOwner()->GetName(),
        bCanBePickedUp ? TEXT("True") : TEXT("False"));

    if (!bCanBePickedUp)
    {
        UE_LOG(LogTemp, Error, TEXT("[PickUp] Cannot be picked up!"));
        return false;
    }

    if (HoldingActor)
    {
        UE_LOG(LogTemp, Error, TEXT("[PickUp] Already held by: %s"), *HoldingActor->GetName());
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[PickUp] No Owner"));
        return false;
    }

    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    // Physics 완전히 비활성화
    DisablePhysics();

    HoldingActor = Picker;

    AHamoniaCharacter* Character = Cast<AHamoniaCharacter>(Picker);
    if (Character && Character->HeldObjectAttachPoint)
    {
        // Detach 먼저 (이전 attachment 제거)
        Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        // 짧은 지연 후 Attach (Physics 상태 완전히 정리) - 수정된 부분
        FTimerHandle AttachTimer;
        Owner->GetWorldTimerManager().SetTimer(AttachTimer, [Owner, Character]()
            {
                Owner->AttachToComponent(
                    Character->HeldObjectAttachPoint,
                    FAttachmentTransformRules::SnapToTargetNotIncludingScale
                );

                Owner->SetActorRelativeLocation(FVector::ZeroVector);
                Owner->SetActorRelativeRotation(FRotator::ZeroRotator);

                UE_LOG(LogTemp, Warning, TEXT("[PickUp] Attached successfully"));
            }, 0.01f, false);
    }
    else
    {
        Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

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

    if (CurrentPedestal)
    {
        RemoveFromPedestal();
    }

    if (HoldingActor)
    {
        Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

    // Physics 완전히 비활성화 - 추가
    DisablePhysics();

    if (Pedestal->PlaceObject(Owner))
    {
        CurrentPedestal = Pedestal;
        HoldingActor = nullptr;  // 추가
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