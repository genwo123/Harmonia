#include "HamoniaPlayerController.h"
#include "HamoniaCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AHamoniaPlayerController::AHamoniaPlayerController()
{
    bShowMouseCursor = false;
    DefaultMouseCursor = EMouseCursor::Default;
}

void AHamoniaPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Display, TEXT("HamoniaPlayerController BeginPlay"));

    // 향상된 입력 시스템 설정
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        UE_LOG(LogTemp, Display, TEXT("Enhanced Input Subsystem Found"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Enhanced Input Subsystem NOT Found"));
    }
}

void AHamoniaPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AHamoniaCharacter* HamoniaChar = Cast<AHamoniaCharacter>(InPawn);
    if (HamoniaChar)
    {
        UE_LOG(LogTemp, Display, TEXT("HamoniaPlayerController possessing HamoniaCharacter at location: %s"),
            *HamoniaChar->GetActorLocation().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HamoniaPlayerController possessing non-HamoniaCharacter"));
    }
}