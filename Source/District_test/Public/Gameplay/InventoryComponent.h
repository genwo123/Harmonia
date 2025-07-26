// InventoryComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Item.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, class UInventoryComponent*, Inventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryToggled, bool, bIsOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, int32, NewSlotIndex);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    // �κ��丮 �� ������ ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<UItem*> Items;

    // �κ��丮 �ִ� �뷮
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Capacity;

    // ���� ���õ� ���� (UI��)
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 CurrentSelectedSlot;

    // �κ��丮 ���� ���� (UI��)
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    bool bIsInventoryOpen;

    // �κ��丮 ���� �̺�Ʈ
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    // �� �� �߰�
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSelectionChanged OnSelectionChanged;


    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryToggled OnInventoryToggled;

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

    // UI ���� ���ο� ��ɵ�
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void MoveSelection(int32 Direction); // -1 = ����, 1 = ������

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseSelectedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetSelectedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetItemAtSlot(int32 SlotIndex);
};