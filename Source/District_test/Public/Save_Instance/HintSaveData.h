#pragma once
#include "CoreMinimal.h"
#include "HintSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FHintBlockState
{
    GENERATED_BODY()

    // 각 블록의 해금 상태 (6개)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> RevealedBlocks;

    // 현재 쿨다운 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentCooldown;

    // 쿨다운 중인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOnCooldown;

    // 마지막 저장 시간 (쿨다운 시간 계산용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastSaveTime;

    FHintBlockState()
    {
        RevealedBlocks.SetNum(6);
        for (int32 i = 0; i < 6; i++)
        {
            RevealedBlocks[i] = false;
        }
        CurrentCooldown = 0.0f;
        bIsOnCooldown = false;
        LastSaveTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FHintSaveData
{
    GENERATED_BODY()

    // 레벨별 힌트 상태 (LevelNumber -> HintBlockState)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<int32, FHintBlockState> LevelHintStates;

    FHintSaveData()
    {
    }
};