#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestUIHelper.generated.h"

UCLASS(BlueprintType)
class DISTRICT_TEST_API UQuestUIHelper : public UObject
{
    GENERATED_BODY()

public:
    // 보여줄 SubStep 인덱스 배열 반환 (최대 MaxVisible개)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest UI")
    static TArray<int32> GetVisibleQuestIndices(
        const TArray<bool>& SubStepStatus,
        int32 MaxVisible = 3
    );

    // 첫 번째 미완료 SubStep 인덱스 반환 (-1 = 모두 완료)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest UI")
    static int32 GetNextIncompleteQuestIndex(const TArray<bool>& SubStepStatus);

    // 완료된 SubStep 개수
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest UI")
    static int32 GetCompletedQuestCount(const TArray<bool>& SubStepStatus);

    // 특정 인덱스의 SubStep이 보여야 하는지
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest UI")
    static bool ShouldQuestBeVisible(
        const TArray<bool>& SubStepStatus,
        int32 QuestIndex,
        int32 MaxVisible = 3
    );

    // 슬라이딩 윈도우 시작 인덱스
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest UI")
    static int32 GetSlidingWindowStartIndex(
        const TArray<bool>& SubStepStatus,
        int32 MaxVisible = 3
    );
};