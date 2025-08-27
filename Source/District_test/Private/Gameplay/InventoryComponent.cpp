// InventoryComponent.cpp
#include "Gameplay/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    Capacity = 8; // 기본 인벤토리 용량
    CurrentSelectedSlot = 0;
    bIsInventoryOpen = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // 노트 아이템 생성 (없는 경우)
    if (!NoteItem)
    {
        NoteItem = NewObject<UItem>(this, UItem::StaticClass());
        NoteItem->Name = TEXT("Note");
        NoteItem->Description = TEXT("A notebook for recording discovered memos and notes.");
        // NoteItem->Icon은 블루프린트에서 설정

        // 노트를 첫 번째 슬롯에 추가
        Items.Add(NoteItem);
    }
}

bool UInventoryComponent::AddItem(UItem* Item)
{
    if (!Item)
    {
        return false;
    }

    if (Items.Num() >= Capacity)
    {
        return false; // 인벤토리가 가득 참
    }

    Items.Add(Item);
    OnInventoryUpdated.Broadcast(this);
    return true;
}

bool UInventoryComponent::RemoveItem(UItem* Item)
{
    if (!Item)
    {
        return false;
    }

    // 노트는 제거할 수 없음
    if (Item == NoteItem)
    {
        return false;
    }

    if (Items.Remove(Item) > 0)
    {
        OnInventoryUpdated.Broadcast(this);
        return true;
    }
    return false;
}

void UInventoryComponent::UseItem(UItem* Item)
{
    if (!Item || !Item->bCanBeUsed)
    {
        return;
    }

    Item->Use(GetOwner());
}

UItem* UInventoryComponent::FindItemByName(const FString& ItemName)
{
    for (UItem* Item : Items)
    {
        if (Item && Item->Name.Equals(ItemName, ESearchCase::IgnoreCase))
        {
            return Item;
        }
    }
    return nullptr;
}

bool UInventoryComponent::HasRoomForItem() const
{
    return Items.Num() < Capacity;
}

void UInventoryComponent::AddNoteEntry(const FString& NoteText)
{
    // 노트 아이템의 설명에 텍스트 추가
    if (NoteItem)
    {
        // 새 항목은 새 줄로 구분
        if (!NoteItem->Description.IsEmpty() && !NoteItem->Description.EndsWith("\n"))
        {
            NoteItem->Description.Append("\n");
        }
        NoteItem->Description.Append(NoteText);
        OnInventoryUpdated.Broadcast(this);
    }
}

void UInventoryComponent::ToggleInventory()
{
    bIsInventoryOpen = !bIsInventoryOpen;
    OnInventoryToggled.Broadcast(bIsInventoryOpen); // 이 줄 추가
}

// EXPANDED: MoveSelection 함수 수정
void UInventoryComponent::MoveSelection(int32 Direction)
{
    if (!bIsInventoryOpen)
    {
        return;
    }

    int32 NewSelection = CurrentSelectedSlot + Direction;

    if (NewSelection < 0)
    {
        NewSelection = Capacity - 1;
    }
    else if (NewSelection >= Capacity)
    {
        NewSelection = 0;
    }

    CurrentSelectedSlot = NewSelection;
    OnSelectionChanged.Broadcast(CurrentSelectedSlot); // 추가된 라인
}

void UInventoryComponent::UseSelectedItem()
{
    if (bIsInventoryOpen)
    {
        UItem* SelectedItem = GetSelectedItem();
        if (SelectedItem)
        {
            UseItem(SelectedItem);
        }
    }
}

UItem* UInventoryComponent::GetSelectedItem()
{
    return GetItemAtSlot(CurrentSelectedSlot);
}

UItem* UInventoryComponent::GetItemAtSlot(int32 SlotIndex)
{
    if (SlotIndex >= 0 && SlotIndex < Items.Num())
    {
        return Items[SlotIndex];
    }
    return nullptr;
}