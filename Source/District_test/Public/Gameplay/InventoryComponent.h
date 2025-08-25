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
    // 인벤토리 내 아이템 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<UItem*> Items;

    // 인벤토리 최대 용량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Capacity;

    // 현재 선택된 슬롯 (UI용)
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 CurrentSelectedSlot;

    // 인벤토리 열린 상태 (UI용)
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    bool bIsInventoryOpen;

    // 인벤토리 변경 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    // 이 줄 추가
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSelectionChanged OnSelectionChanged;


    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryToggled OnInventoryToggled;

    // 노트 아이템 - 게임에서 주요 역할
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UItem* NoteItem;

    // 아이템 추가
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(UItem* Item);

    // 아이템 제거
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(UItem* Item);

    // 아이템 사용
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseItem(UItem* Item);

    // 특정 이름의 아이템 찾기
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* FindItemByName(const FString& ItemName);

    // 인벤토리에 아이템 추가 가능한지 확인
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasRoomForItem() const;

    // 노트에 메시지 추가 (쪽지, 메모 등 발견 시)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddNoteEntry(const FString& NoteText);

    // UI 관련 새로운 기능들
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void MoveSelection(int32 Direction); // -1 = 왼쪽, 1 = 오른쪽

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseSelectedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetSelectedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetItemAtSlot(int32 SlotIndex);
};