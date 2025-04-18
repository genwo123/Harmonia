// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, class UInventoryComponent*, Inventory);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// �κ��丮 �� ������ ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<UItem*> Items;

	// �κ��丮 �ִ� �뷮
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Capacity;

	// �κ��丮 ���� �̺�Ʈ
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	// ��Ʈ ������ - ���ӿ��� �ֿ� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UItem* NoteItem;

	// ������ �߰�
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UItem* Item);

	// ������ ����
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UItem* Item);

	// ������ ���
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(UItem* Item);

	// Ư�� �̸��� ������ ã��
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItem* FindItemByName(const FString& ItemName);

	// �κ��丮�� ������ �߰� �������� Ȯ��
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasRoomForItem() const;

	// ��Ʈ�� �޽��� �߰� (����, �޸� �� �߰� ��)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddNoteEntry(const FString& NoteText);
};