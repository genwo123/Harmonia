// HamoniaPlayerController.cpp
// HamoniaPlayerController.cpp
#include "HamoniaPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h" // �� ��� ���� �߰�

AHamoniaPlayerController::AHamoniaPlayerController()
{
    // �⺻ ������
    bShowMouseCursor = false;
    DefaultMouseCursor = EMouseCursor::Default;

    // ���콺 �Է� ����
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AHamoniaPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input Subsystem ����
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            UE_LOG(LogTemp, Warning, TEXT("HamoniaPlayerController: ���� ���ؽ�Ʈ �߰� - %s"), *DefaultMappingContext->GetName());
            Subsystem->AddMappingContext(DefaultMappingContext, InputMappingPriority);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HamoniaPlayerController: DefaultMappingContext�� NULL�Դϴ�!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HamoniaPlayerController: Enhanced Input Subsystem�� ã�� �� �����ϴ�!"));
    }
}

void AHamoniaPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Enhanced Input Component Ȯ��
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        

        // ���⿡ ��Ʈ�ѷ� Ưȭ �Է� ���ε� �߰� ����
        // ��: �Ͻ�����, UI Ž�� ��
    }
    else
    {
        
    }
}