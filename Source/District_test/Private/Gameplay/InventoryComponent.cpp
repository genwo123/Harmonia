// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Capacity = 10; // �⺻ �κ��丮 �뷮
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ��Ʈ ������ ���� (���� ���)
	if (!NoteItem)
	{
		NoteItem = NewObject<UItem>(this, UItem::StaticClass());
		NoteItem->Name = TEXT("��Ʈ");
		NoteItem->Description = TEXT("�߰��� �޸�� ������ ����ϴ� ��Ʈ�Դϴ�.");
		// ��Ʈ �������� �κ��丮�� �߰����� �ʰ� ������ ����
	}
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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