// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Capacity = 10; // 기본 인벤토리 용량
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 노트 아이템 생성 (없는 경우)
	if (!NoteItem)
	{
		NoteItem = NewObject<UItem>(this, UItem::StaticClass());
		NoteItem->Name = TEXT("노트");
		NoteItem->Description = TEXT("발견한 메모와 쪽지를 기록하는 노트입니다.");
		// 노트 아이템은 인벤토리에 추가하지 않고 별도로 관리
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