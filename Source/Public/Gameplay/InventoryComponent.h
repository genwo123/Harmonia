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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<UItem*> Items;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Capacity;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 CurrentSelectedSlot;

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    bool bIsInventoryOpen;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnSelectionChanged OnSelectionChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryToggled OnInventoryToggled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UItem* NoteItem;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SaveInventoryToGameInstance();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void LoadInventoryFromGameInstance();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(UItem* Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(UItem* Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseItem(UItem* Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* FindItemByName(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasRoomForItem() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddNoteEntry(const FString& NoteText);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void MoveSelection(int32 Direction);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UseSelectedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetSelectedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetItemAtSlot(int32 SlotIndex);
};