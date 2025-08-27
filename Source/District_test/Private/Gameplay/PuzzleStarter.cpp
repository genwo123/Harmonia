// PuzzleStarter.cpp - ���常 ������ �߰�
#include "Gameplay/PuzzleStarter.h"
#include "Gameplay/GridMazeManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"  // �߰�

APuzzleStarter::APuzzleStarter()
{
    PrimaryActorTick.bCanEverTick = false;

    // ��Ʈ ������Ʈ
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // �޽� ������Ʈ
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    // �⺻ ť�� �޽� �ε�
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
        MeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 0.5f)); // ��ư ���� ���
    }

    // ��ȣ�ۿ� ����
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // �⺻ ����
    bSingleUse = true;
    bUsed = false;
    SoundVolume = 1.0f;  // �߰�
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

    // ���� ���¿����� ��� �Ұ� (bSingleUse�� true�� ����)
    if (CurrentState == EPuzzleState::Success && bSingleUse)
    {
        bUsed = true;
        UE_LOG(LogTemp, Warning, TEXT("Puzzle already completed - PuzzleStarter disabled"));
        return;
    }

    // ��� ���¿��� ���� �ʱ�ȭ �� ī��Ʈ�ٿ� ����
    UE_LOG(LogTemp, Warning, TEXT("Complete reset and starting countdown"));
    ConnectedMazeManager->CompleteReset();  // ���� �ʱ�ȭ

    // ���� ƽ�� ī��Ʈ�ٿ� ���� (���� �Ϸ� ��)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (ConnectedMazeManager && IsValid(ConnectedMazeManager))
            {
                ConnectedMazeManager->StartPuzzleWithCountdown();  // ī��Ʈ�ٿ�� �Բ� ����
                UE_LOG(LogTemp, Warning, TEXT("Countdown started via PuzzleStarter"));
            }
        });
}

// ====== InteractableInterface ���� ======
void APuzzleStarter::Interact_Implementation(AActor* Interactor)
{
    if (!CanInteract_Implementation(Interactor))
    {
        return;
    }

    // ====== ���� ��� (�߰��� �κ�) ======
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