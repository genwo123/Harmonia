// HamoniaPlayerController.cpp
// HamoniaPlayerController.cpp
#include "HamoniaPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h" // 이 헤더 파일 추가

AHamoniaPlayerController::AHamoniaPlayerController()
{
    // 기본 설정들
    bShowMouseCursor = false;
    DefaultMouseCursor = EMouseCursor::Default;

    // 마우스 입력 설정
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AHamoniaPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input Subsystem 설정
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            UE_LOG(LogTemp, Warning, TEXT("HamoniaPlayerController: 매핑 컨텍스트 추가 - %s"), *DefaultMappingContext->GetName());
            Subsystem->AddMappingContext(DefaultMappingContext, InputMappingPriority);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HamoniaPlayerController: DefaultMappingContext가 NULL입니다!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HamoniaPlayerController: Enhanced Input Subsystem을 찾을 수 없습니다!"));
    }
}

void AHamoniaPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Enhanced Input Component 확인
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        

        // 여기에 컨트롤러 특화 입력 바인딩 추가 가능
        // 예: 일시정지, UI 탐색 등
    }
    else
    {
        
    }
}