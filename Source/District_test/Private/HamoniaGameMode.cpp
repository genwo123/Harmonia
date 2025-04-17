// HamoniaGameMode.cpp
#include "HamoniaGameMode.h"
#include "HamoniaCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AHamoniaGameMode::AHamoniaGameMode()
{
    // HamoniaCharacter�� �⺻ ������ ����
    DefaultPawnClass = AHamoniaCharacter::StaticClass();
}

void AHamoniaGameMode::StartPlay()
{
    Super::StartPlay();

}

AActor* AHamoniaGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    // ��� PlayerStart ã��
    TArray<AActor*> FoundStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundStarts);

    for (AActor* Start : FoundStarts)
    {
        UE_LOG(LogTemp, Warning, TEXT("�÷��̾� ��ŸƮ ã��: %s, ��ġ: %s"),
            *Start->GetName(),
            *Start->GetActorLocation().ToString());
    }

    // �÷��̾� ��ŸƮ�� ������ ù ��° �� ���
    if (FoundStarts.Num() > 0)
    {
        return FoundStarts[0];
    }

    // ������ �⺻ ���� ���
    return Super::FindPlayerStart_Implementation(Player, IncomingName);
}