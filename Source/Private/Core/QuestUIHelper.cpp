#include "Core/QuestUIHelper.h"                 

TArray<int32> UQuestUIHelper::GetVisibleQuestIndices(const TArray<bool>& SubStepStatus, int32 MaxVisible)
{
    TArray<int32> VisibleIndices;

    if (SubStepStatus.Num() == 0)
    {
        return VisibleIndices;
    }

    // 슬라이딩 윈도우 시작 인덱스
    int32 StartIndex = GetSlidingWindowStartIndex(SubStepStatus, MaxVisible);

    // 시작 인덱스부터 MaxVisible개만큼 추가
    for (int32 i = StartIndex; i < SubStepStatus.Num() && VisibleIndices.Num() < MaxVisible; i++)
    {
        VisibleIndices.Add(i);
    }

    return VisibleIndices;
}

int32 UQuestUIHelper::GetNextIncompleteQuestIndex(const TArray<bool>& SubStepStatus)
{
    for (int32 i = 0; i < SubStepStatus.Num(); i++)
    {
        if (!SubStepStatus[i])
        {
            return i; // 첫 번째 미완료 발견
        }
    }

    return -1; // 모두 완료
}

int32 UQuestUIHelper::GetCompletedQuestCount(const TArray<bool>& SubStepStatus)
{
    int32 Count = 0;

    for (bool bCompleted : SubStepStatus)
    {
        if (bCompleted)
        {
            Count++;
        }
    }

    return Count;
}

bool UQuestUIHelper::ShouldQuestBeVisible(const TArray<bool>& SubStepStatus, int32 QuestIndex, int32 MaxVisible)
{
    if (QuestIndex < 0 || QuestIndex >= SubStepStatus.Num())
    {
        return false;
    }

    TArray<int32> VisibleIndices = GetVisibleQuestIndices(SubStepStatus, MaxVisible);
    return VisibleIndices.Contains(QuestIndex);
}

int32 UQuestUIHelper::GetSlidingWindowStartIndex(const TArray<bool>& SubStepStatus, int32 MaxVisible)
{
    // 첫 번째 미완료 SubStep 찾기
    int32 FirstIncomplete = GetNextIncompleteQuestIndex(SubStepStatus);

    // 모두 완료 시 → 마지막 MaxVisible개 표시
    if (FirstIncomplete == -1)
    {
        return FMath::Max(0, SubStepStatus.Num() - MaxVisible);
    }

    // 미완료가 있으면 → 그 위치부터 시작
    return FirstIncomplete;
}