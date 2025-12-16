#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HamoniaGameMode.generated.h"

// 전방 선언
class ALevelQuestManager;

UCLASS()
class DISTRICT_TEST_API AHamoniaGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHamoniaGameMode();

    virtual void StartPlay() override;
    virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

    // === 퀘스트 시스템 추가 ===

    // 퀘스트 매니저
    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    ALevelQuestManager* QuestManager;

    // 퀘스트 관련 함수들 (블루프린트에서 쉽게 사용)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteLevel();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartNewLevel(const FString& LevelID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsLevelDone(const FString& LevelID);
};