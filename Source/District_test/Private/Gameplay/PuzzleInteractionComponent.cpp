#include "Gameplay/PuzzleInteractionComponent.h"
#include "Gameplay/Pedestal.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
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

                FVector NewLocation = CameraLocation +
                    (CameraRotation.Vector() * HoldOffset.X) +
                    (FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y) * HoldOffset.Y) +
                    (FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Z) * HoldOffset.Z);

                Owner->SetActorLocation(NewLocation);

                // 1초마다 한 번씩만 로그 (Tick은 너무 많이 찍혀서)
                static float LogTimer = 0.0f;
                LogTimer += DeltaTime;
                if (LogTimer >= 1.0f)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[Holding] Object: %s | HoldOffset: %s | Current Pos: %s"),
                        *Owner->GetName(), *HoldOffset.ToString(), *NewLocation.ToString());
                    LogTimer = 0.0f;
                }

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
    UE_LOG(LogTemp, Warning, TEXT("[PickUp] Attempting - bCanBePickedUp: %d, HoldingActor: %s"),
        bCanBePickedUp, HoldingActor ? *HoldingActor->GetName() : TEXT("NULL"));

    if (!bCanBePickedUp)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PickUp] FAILED - Cannot be picked up"));
        return false;
    }

    if (HoldingActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PickUp] FAILED - Already being held by: %s"), *HoldingActor->GetName());
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[PickUp] FAILED - No Owner"));
        return false;
    }

    if (CurrentPedestal)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PickUp] Removing from Pedestal"));
        RemoveFromPedestal();
    }

    DisablePhysics();

    HoldingActor = Picker;

    Owner->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    UE_LOG(LogTemp, Warning, TEXT("[PickUp] SUCCESS - Object: %s, Picker: %s, HoldingActor set to: %s"),
        *Owner->GetName(), *Picker->GetName(), *HoldingActor->GetName());

    return true;
}

bool UPuzzleInteractionComponent::PutDown(FVector Location, FRotator Rotation)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[PutDown] FAILED - No Owner"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[PutDown] Called - Object: %s, HoldingActor: %s"),
        *Owner->GetName(), HoldingActor ? *HoldingActor->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("[PutDown] Requested Location: %s"), *Location.ToString());

    // HoldingActor를 먼저 nullptr로 설정
    HoldingActor = nullptr;

    Owner->SetActorLocation(Location);
    Owner->SetActorRotation(Rotation);

    FVector ActualLocation = Owner->GetActorLocation();
    UE_LOG(LogTemp, Warning, TEXT("[PutDown] Actual Location: %s"), *ActualLocation.ToString());

    if (bEnablePhysicsWhenDropped)
    {
        EnablePhysics();
        UE_LOG(LogTemp, Warning, TEXT("[PutDown] Physics Enabled"));
    }

    UE_LOG(LogTemp, Warning, TEXT("[PutDown] SUCCESS - HoldingActor now: %s"),
        HoldingActor ? TEXT("STILL SET!") : TEXT("NULL (correct)"));

    return true;
}

bool UPuzzleInteractionComponent::PlaceOnPedestal(APedestal* Pedestal)
{
    if (!Pedestal || !bCanBePlacedOnPedestal)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlaceOnPedestal] FAILED - Pedestal: %d, CanBePlaced: %d"),
            Pedestal != nullptr, bCanBePlacedOnPedestal);
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlaceOnPedestal] FAILED - No Owner"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[PlaceOnPedestal] Attempting to place %s on %s"),
        *Owner->GetName(), *Pedestal->GetName());

    if (CurrentPedestal)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlaceOnPedestal] Removing from current pedestal first"));
        RemoveFromPedestal();
    }

    // HoldingActor를 먼저 nullptr로 설정
    HoldingActor = nullptr;

    if (Pedestal->PlaceObject(Owner))
    {
        CurrentPedestal = Pedestal;
        DisablePhysics();

        UE_LOG(LogTemp, Warning, TEXT("[PlaceOnPedestal] SUCCESS - Object placed"));
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("[PlaceOnPedestal] FAILED - Pedestal->PlaceObject returned false"));
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