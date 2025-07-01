#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HamoniaGameMode.generated.h"

// ���� ����
class ALevelQuestManager;

UCLASS()
class DISTRICT_TEST_API AHamoniaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHamoniaGameMode();

    virtual void StartPlay() override;
    virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

    // === ����Ʈ �ý��� �߰� ===

    // ����Ʈ �Ŵ���
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    ALevelQuestManager* QuestManager;

    // ����Ʈ ���� �Լ��� (�������Ʈ���� ���� ���)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteLevel();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartNewLevel(const FString& LevelID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsLevelDone(const FString& LevelID);
};