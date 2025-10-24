#include "Save_Instance/Hamoina_GameInstance.h"
#include "Kismet/GameplayStatics.h"
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

    if (!NoteItem)
    {
        NoteItem = NewObject<UItem>(this, UItem::StaticClass());
        NoteItem->Name = TEXT("Note");
        NoteItem->Description = TEXT("A notebook for recording discovered memos and notes.");
        Items.Add(NoteItem);
    }

    LoadInventoryFromGameInstance();
}

bool UInventoryComponent::AddItem(UItem* Item)
{
    if (!Item)
    {
        return false;
    }
    if (Items.Num() >= Capacity)
    {
        return false;
    }
    Items.Add(Item);
    SaveInventoryToGameInstance();
    OnInventoryUpdated.Broadcast(this);
    return true;
}

bool UInventoryComponent::RemoveItem(UItem* Item)
{
    if (!Item)
    {
        return false;
    }
    if (Item == NoteItem)
    {
        return false;
    }
    if (Items.Remove(Item) > 0)
    {
        SaveInventoryToGameInstance();
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

void UInventoryComponent::SaveInventoryToGameInstance()
{
    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(
        UGameplayStatics::GetGameInstance(this));

    if (!GameInstance || !GameInstance->CurrentSaveData)
    {
        return;
    }

    GameInstance->CurrentSaveData->PlayerData.InventoryItems.Empty();
    GameInstance->CurrentSaveData->PlayerData.ItemQuantities.Empty();

    for (UItem* Item : Items)
    {
        if (Item && Item != NoteItem)
        {
            FString ItemID = Item->GetClass()->GetName();

            if (GameInstance->CurrentSaveData->PlayerData.InventoryItems.Contains(ItemID))
            {
                int32* Quantity = GameInstance->CurrentSaveData->PlayerData.ItemQuantities.Find(ItemID);
                if (Quantity)
                {
                    (*Quantity)++;
                }
            }
            else
            {
                GameInstance->CurrentSaveData->PlayerData.InventoryItems.Add(ItemID);
                GameInstance->CurrentSaveData->PlayerData.ItemQuantities.Add(ItemID, 1);
            }
        }
    }

    GameInstance->SaveContinueGame();
}

void UInventoryComponent::LoadInventoryFromGameInstance()
{
    UHamoina_GameInstance* GameInstance = Cast<UHamoina_GameInstance>(
        UGameplayStatics::GetGameInstance(this));

    if (!GameInstance || !GameInstance->CurrentSaveData)
    {
        return;
    }

    for (int32 i = Items.Num() - 1; i >= 0; i--)
    {
        if (Items[i] != NoteItem)
        {
            Items.RemoveAt(i);
        }
    }

    for (const FString& ItemID : GameInstance->CurrentSaveData->PlayerData.InventoryItems)
    {
        int32* Quantity = GameInstance->CurrentSaveData->PlayerData.ItemQuantities.Find(ItemID);
        int32 ItemCount = Quantity ? *Quantity : 1;

        UClass* ItemClass = StaticLoadClass(UItem::StaticClass(), nullptr, *ItemID);

        if (ItemClass)
        {
            for (int32 i = 0; i < ItemCount; i++)
            {
                UItem* NewItem = NewObject<UItem>(this, ItemClass);
                if (NewItem)
                {
                    Items.Add(NewItem);
                }
            }
        }
    }

    OnInventoryUpdated.Broadcast(this);
}