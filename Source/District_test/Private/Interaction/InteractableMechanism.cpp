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

    // 메시 컴포넌트를 루트로 생성
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // 메시 컴포넌트 설정
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetGenerateOverlapEvents(true);
    MeshComponent->SetMobility(EComponentMobility::Movable);

    // 인터렉션 쉐잎 생성 (메시에 붙이기)
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetGenerateOverlapEvents(true);
    InteractionSphere->SetMobility(EComponentMobility::Movable);

    // 시각적 디버깅을 위해 게임 내에서도 보이게 설정
    InteractionSphere->bHiddenInGame = false;

    // 기본 속성 설정
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
    // 상호작용 가능 여부 확인
    if (!bCanInteract)
    {
        return;
    }

    // 메커니즘 유형에 따른 처리
    switch (MechanismType)
    {
    case EMechanismType::Door:
        HandleDoorInteraction(Interactor);
        break;

    case EMechanismType::Widget:
        HandleWidgetInteraction(Interactor);
        break;

    default:
        // 기본 상호작용
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
    // 상호작용 소리 재생
    if (InteractionSound)
    {
        UGameplayStatics::PlaySound2D(this, InteractionSound);
    }

    // 블루프린트 이벤트 호출로 위젯 표시 처리
    OnShowWidget(Cast<APlayerController>(Interactor->GetInstigatorController()));
}




void AInteractableMechanism::HideInteractionWidget()
{
    // 입력 모드만 복원하고 위젯 처리는 블루프린트에서
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    // 블루프린트 이벤트 호출
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
    // 위젯 숨기기
    HideInteractionWidget();

    // 실패 처리
    OnInteractionFailed(UGameplayStatics::GetPlayerPawn(this, 0));
}


bool AInteractableMechanism::CanInteract_Implementation(AActor* Interactor)
{
    // 기본 상호작용 가능 여부 확인
    if (!bCanInteract)
    {
        UE_LOG(LogTemp, Warning, TEXT("bCanInteract is FALSE"));
        return false;
    }

    // 이미 완료되었고 재사용 불가능하면 상호작용 차단
    if (bIsCompleted && !bCanBeUsedAgain)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mechanism already completed and cannot be reused"));
        return false;
    }

    // 메커니즘 유형별 추가 검사
    switch (MechanismType)
    {
    case EMechanismType::Door:
    {
        // 1. 키패드 조건 확인
        if (bRequiresKeypadCompletion)
        {
            bool bAllKeypadCompleted = CheckRequiredKeypads();
            if (!bAllKeypadCompleted)
            {
                UE_LOG(LogTemp, Warning, TEXT("Required keypads not completed"));
                return false;
            }
        }

        // 2. 키 아이템 조건 확인
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
        // 위젯의 경우 완료 상태가 아니라면 상호작용 가능
        if (bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Widget mechanism already completed"));
            return bCanBeUsedAgain; // 재사용 가능 여부에 따라 결정
        }

        UE_LOG(LogTemp, Warning, TEXT("Widget mechanism - interaction allowed"));
        return true;

    default:
        // 기본 타입의 경우 항상 상호작용 가능
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
    // 메커니즘 유형에 따라 적절한 인터랙션 타입 반환
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

// InteractableMechanism.cpp에 추가할 헬퍼 함수들

bool AInteractableMechanism::CheckRequiredKeypads()
{
    if (RequiredKeypadIDs.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No required keypads specified"));
        return true; // 요구사항이 없으면 통과
    }

    // 월드의 모든 InteractableMechanism 액터 검색
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