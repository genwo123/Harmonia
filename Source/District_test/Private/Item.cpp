// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"

UItem::UItem()
{
	// �⺻�� ����
	Name = TEXT("Item");
	Description = TEXT("Descript");
	Icon = nullptr;
	bCanBeUsed = false;
	MaxStackSize = 1;
}

void UItem::Use_Implementation(AActor* User)
{
	// �⺻ ������ �ƹ� �۾��� �������� �ʽ��ϴ�.
	// �ڽ� Ŭ�������� ���� ������ �����ؾ� �մϴ�.
}