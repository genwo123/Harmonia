#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NoteSaveData.generated.h"

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FNoteSaveData
{
    GENERATED_BODY()

    // ������ ������ (���忡�� �Ⱦ��� �͵�)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Collection")
    TArray<FString> CollectedNotes; // ["Note_Tutorial_01", "Note_Level1_Secret", "Note_Hint_Puzzle"]

    // ��Ʈ �б� ���� (��ƮID -> �о����� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Reading")
    TMap<FString, bool> NoteReadStatus; // {"Note_Tutorial_01": true, "Note_Level1_Secret": false}

    // ��Ʈ�� ���� Ƚ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Reading")
    TMap<FString, int32> NoteReadCount; // �� �� �о�����

    // ��Ʈ �������� Ȱ��ȭ ���� (���� ȹ�� �� �ش� ������ ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Pages")
    TMap<FString, int32> NotePageProgress; // {"Chapter1": 5} - 5���������� ����

    // ��Ʈ ī�װ��� ���൵
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Categories")
    TMap<FString, float> CategoryProgress; // {"Tutorial": 1.0, "Story": 0.6, "Hints": 0.3}

    // ��Ʈ �߰� �ð�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Discovery")
    TMap<FString, FDateTime> NoteFoundTime; // ���� �߰��ߴ���

    // ���ã��� ��Ʈ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note Favorites")
    TArray<FString> FavoriteNotes;

    // ��Ʈ UI ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note UI")
    FString LastViewedNote; // ���������� �� ��Ʈ

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Note UI")
    int32 LastViewedPage = 1; // ���������� �� ������

    // ������
    FNoteSaveData()
    {
        LastViewedNote = TEXT("");
        LastViewedPage = 1;
    }
};