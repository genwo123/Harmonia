// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"

UItem::UItem()
{
	// 기본값 설정
	Name = TEXT("Item");
	Description = TEXT("Descript");
	Icon = nullptr;
	bCanBeUsed = false;
	MaxStackSize = 1;
}

void UItem::Use_Implementation(AActor* User)
{
	// 기본 구현은 아무 작업도 수행하지 않습니다.
	// 자식 클래스에서 실제 동작을 구현해야 합니다.
}