#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeldItemDisplayComponent.generated.h"

class UStaticMeshComponent;
class UItem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UHeldItemDisplayComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeldItemDisplayComponent();

    UFUNCTION(BlueprintCallable, Category = "Item Display")
    void UpdateDisplay(UItem* NewItem, bool bInventoryOpen);

    UFUNCTION(BlueprintCallable, Category = "Item Display")
    void ShowItemMesh(UItem* Item);

    UFUNCTION(BlueprintCallable, Category = "Item Display")
    void HideItemMesh();

    UFUNCTION(BlueprintPure, Category = "Item Display")
    UItem* GetCurrentDisplayedItem() const { return CurrentDisplayedItem; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Item Display")
    void ShowHeldItemMeshBP(UItem* Item);

    UFUNCTION(BlueprintImplementableEvent, Category = "Item Display")
    void HideHeldItemMeshBP();

    UFUNCTION(BlueprintCallable, Category = "Item Display")
    void SetupMeshComponent(UStaticMeshComponent* MeshComp);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UStaticMeshComponent* HeldItemMesh;

    UPROPERTY()
    UItem* CurrentDisplayedItem;
};