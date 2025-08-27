#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NoteSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FNoteSaveData
{
    GENERATED_BODY()

    // 수집한 쪽지들 (월드에서 픽업한 것들)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Collection")
    TArray<FString> CollectedNotes; // ["Note_Tutorial_01", "Note_Level1_Secret", "Note_Hint_Puzzle"]

    // 노트 읽기 상태 (노트ID -> 읽었는지 여부)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Reading")
    TMap<FString, bool> NoteReadStatus; // {"Note_Tutorial_01": true, "Note_Level1_Secret": false}

    // 노트별 읽은 횟수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Reading")
    TMap<FString, int32> NoteReadCount; // 몇 번 읽었는지

    // 노트 페이지별 활성화 상태 (쪽지 획득 시 해당 페이지 열림)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Pages")
    TMap<FString, int32> NotePageProgress; // {"Chapter1": 5} - 5페이지까지 열림

    // 노트 카테고리별 진행도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Categories")
    TMap<FString, float> CategoryProgress; // {"Tutorial": 1.0, "Story": 0.6, "Hints": 0.3}

    // 노트 발견 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Discovery")
    TMap<FString, FDateTime> NoteFoundTime; // 언제 발견했는지

    // 즐겨찾기된 노트들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Favorites")
    TArray<FString> FavoriteNotes;

    // 노트 UI 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note UI")
    FString LastViewedNote; // 마지막으로 본 노트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note UI")
    int32 LastViewedPage = 1; // 마지막으로 본 페이지

    // 생성자
    FNoteSaveData()
    {
        LastViewedNote = TEXT("");
        LastViewedPage = 1;
    }
};