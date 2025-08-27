// PuzzleStarter.cpp - 사운드만 간단히 추가
#include "Gameplay/PuzzleStarter.h"
#include "Gameplay/GridMazeManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"  // 추가

APuzzleStarter::APuzzleStarter()
{
    PrimaryActorTick.bCanEverTick = false;

    // 루트 컴포넌트
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 메시 컴포넌트
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    // 기본 큐브 메시 로드
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
        MeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 0.5f)); // 버튼 같은 모양
    }

    // 상호작용 영역
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // 기본 설정
    bSingleUse = true;
    bUsed = false;
    SoundVolume = 1.0f;  // 추가
}

void APuzzleStarter::StartConnectedPuzzle()
{
    if (!ConnectedMazeManager || !IsValid(ConnectedMazeManager))
    {
        UE_LOG(LogTemp, Warning, TEXT("No connected maze manager found"));
        return;
    }

    EPuzzleState CurrentState = ConnectedMazeManager->GetCurrentState();

    UE_LOG(LogTemp, Warning, TEXT("PuzzleStarter activated - Current state: %d"), (int32)CurrentState);

    // 성공 상태에서는 사용 불가 (bSingleUse가 true일 때만)
    if (CurrentState == EPuzzleState::Success && bSingleUse)
    {
        bUsed = true;
        UE_LOG(LogTemp, Warning, TEXT("Puzzle already completed - PuzzleStarter disabled"));
        return;
    }

    // 모든 상태에서 완전 초기화 후 카운트다운 시작
    UE_LOG(LogTemp, Warning, TEXT("Complete reset and starting countdown"));
    ConnectedMazeManager->CompleteReset();  // 완전 초기화

    // 다음 틱에 카운트다운 시작 (리셋 완료 후)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (ConnectedMazeManager && IsValid(ConnectedMazeManager))
            {
                ConnectedMazeManager->StartPuzzleWithCountdown();  // 카운트다운과 함께 시작
                UE_LOG(LogTemp, Warning, TEXT("Countdown started via PuzzleStarter"));
            }
        });
}

// ====== InteractableInterface 구현 ======
void APuzzleStarter::Interact_Implementation(AActor* Interactor)
{
    if (!CanInteract_Implementation(Interactor))
    {
        return;
    }

    // ====== 사운드 재생 (추가된 부분) ======
    if (ButtonSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ButtonSound, GetActorLocation(), SoundVolume);
    }

    StartConnectedPuzzle();
}

bool APuzzleStarter::CanInteract_Implementation(AActor* Interactor)
{
    if (bSingleUse && bUsed)
    {
        return false;
    }
    return ConnectedMazeManager != nullptr;
}

FString APuzzleStarter::GetInteractionText_Implementation()
{
    if (bSingleUse && bUsed)
    {
        return TEXT("Already Used");
    }
    if (!ConnectedMazeManager)
    {
        return TEXT("No Puzzle Connected");
    }
    return TEXT("Start Puzzle");
}

EInteractionType APuzzleStarter::GetInteractionType_Implementation()
{
    return EInteractionType::Activate;
}