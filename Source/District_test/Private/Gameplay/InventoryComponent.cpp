// InventoryComponent.cpp
#include "Gameplay/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    Capacity = 8; // �⺻ �κ��丮 �뷮
    CurrentSelectedSlot = 0;
    bIsInventoryOpen = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // ��Ʈ ������ ���� (���� ���)
    if (!NoteItem)
    {
        NoteItem = NewObject<UItem>(this, UItem::StaticClass());
        NoteItem->Name = TEXT("Note");
        NoteItem->Description = TEXT("A notebook for recording discovered memos and notes.");
        // NoteItem->Icon�� �������Ʈ���� ����

        // ��Ʈ�� ù ��° ���Կ� �߰�
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
        return false; // �κ��丮�� ���� ��
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

    // ��Ʈ�� ������ �� ����
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
    // ��Ʈ �������� ���� �ؽ�Ʈ �߰�
    if (NoteItem)
    {
        // �� �׸��� �� �ٷ� ����
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
    OnInventoryToggled.Broadcast(bIsInventoryOpen); // �� �� �߰�
}

// EXPANDED: MoveSelection �Լ� ����
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
    OnSelectionChanged.Broadcast(CurrentSelectedSlot); // �߰��� ����
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