// DialogueManagerComponent.cpp
#include "Core/DialogueManagerComponent.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UDialogueManagerComponent::StartDialogue(const FString& DialogueID)
{
    if (bIsInDialogue)
    {
        return false;
    }

    FDialogueData* DialogueData = GetDialogueData(DialogueID);
    if (!DialogueData)
    {
        return false;
    }

    if (!CheckAllConditions(*DialogueData))
    {
        return false;
    }

    bIsInDialogue = true;
    CurrentDialogueID = DialogueID;
    CurrentDialogue = *DialogueData;

    ProcessDialogue(*DialogueData);

    return true;
}

void UDialogueManagerComponent::EndDialogue()
{
    if (!bIsInDialogue)
    {
        return;
    }

    bIsInDialogue = false;
    CurrentDialogueID = "";

    OnDialogueEnded.Broadcast();
}

void UDialogueManagerComponent::ProgressDialogue()
{
    if (!bIsInDialogue)
    {
        return;
    }

    if (CurrentDialogue.NextDialogueID.IsEmpty())
    {
        EndDialogue();
        return;
    }

    FString NextID = CurrentDialogue.NextDialogueID;
    EndDialogue();
    StartDialogue(NextID);
}

void UDialogueManagerComponent::SelectChoice(int32 ChoiceIndex)
{
    if (!bIsInDialogue || !CurrentDialogue.bHasChoices)
    {
        return;
    }

    if (ChoiceIndex < 0 || ChoiceIndex >= CurrentDialogue.ChoiceTargetIDs.Num())
    {
        return;
    }

    FString TargetDialogueID = CurrentDialogue.ChoiceTargetIDs[ChoiceIndex];
    if (TargetDialogueID.IsEmpty())
    {
        EndDialogue();
        return;
    }

    // 퀘스트 이벤트 처리
    if (!CurrentDialogue.QuestEventID.IsEmpty())
    {
        // ProcessQuestEvent(CurrentDialogue.QuestEventID, ChoiceIndex);
    }

    // 대화 전환
    if (!StartDialogue(TargetDialogueID))
    {
        EndDialogue();
    }
}

TArray<FString> UDialogueManagerComponent::GetCurrentChoices()
{
    if (bIsInDialogue && CurrentDialogue.bHasChoices)
    {
        return CurrentDialogue.ChoiceTexts;
    }

    return TArray<FString>();
}

FString UDialogueManagerComponent::FindDialogueForQuest(const FString& QuestState)
{
    if (!DialogueDataTable)
    {
        return "";
    }

    TArray<FDialogueData*> AllDialogues;
    DialogueDataTable->GetAllRows<FDialogueData>("", AllDialogues);

    for (FDialogueData* DialogueData : AllDialogues)
    {
        if (DialogueData && DialogueData->RequiredQuestState == QuestState)
        {
            return DialogueData->DialogueID;
        }
    }

    return "";
}

void UDialogueManagerComponent::PlayMonologue(const FString& MonologueID)
{
    FDialogueData* MonologueData = GetDialogueData(MonologueID);
    if (!MonologueData)
    {
        return;
    }

    ProcessDialogue(*MonologueData);
}

bool UDialogueManagerComponent::CheckQuestCondition(const FString& RequiredQuestState)
{
    if (RequiredQuestState.IsEmpty())
    {
        return true;
    }

    // TODO: 퀘스트 시스템과 연동
    return true;
}

bool UDialogueManagerComponent::CheckItemCondition(const FString& RequiredItemID)
{
    if (RequiredItemID.IsEmpty())
    {
        return true;
    }

    // TODO: 인벤토리 시스템과 연동
    return true;
}

bool UDialogueManagerComponent::CheckAllConditions(const FDialogueData& DialogueData)
{
    if (!CheckQuestCondition(DialogueData.RequiredQuestState))
    {
        return false;
    }

    if (DialogueData.bRequireItem && !CheckItemCondition(DialogueData.RequiredItemID))
    {
        return false;
    }

    return true;
}

FDialogueData* UDialogueManagerComponent::GetDialogueData(const FString& DialogueID)
{
    if (!DialogueDataTable)
    {
        return nullptr;
    }

    TArray<FDialogueData*> AllDialogues;
    DialogueDataTable->GetAllRows<FDialogueData>("", AllDialogues);

    for (FDialogueData* DialogueData : AllDialogues)
    {
        if (DialogueData && DialogueData->DialogueID == DialogueID)
        {
            return DialogueData;
        }
    }

    return nullptr;
}

void UDialogueManagerComponent::ProcessDialogue(const FDialogueData& DialogueData)
{
    OnDialogueStarted.Broadcast(
        DialogueData.Speaker,
        DialogueData.DialogueText,
        DialogueData.DialogueType,
        DialogueData.DisplayDuration
    );
}