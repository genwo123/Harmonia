#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/InteractionEnums.h"
#include "PlayerInteractionComponent.generated.h"

class UCameraComponent;
class UInventoryComponent;
class APedestal;
class AUnia;
class UItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInteractionChanged,
    bool, bIsInteractable,
    FString, InteractionText,
    EInteractionType, InteractionType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UPlayerInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerInteractionComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PerformInteraction();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RotateObject();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PushObject();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnEKeyPressed();

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool IsLookingAtInteractable() const { return bIsLookingAtInteractable; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    FString GetCurrentInteractionText() const { return CurrentInteractionText; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    EInteractionType GetCurrentInteractionType() const { return CurrentInteractionType; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AActor* GetCurrentInteractableActor() const { return CurrentInteractableActor; }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetCurrentInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RemoveInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AUnia* GetCurrentInteractableNPC() const { return CurrentInteractableNPC; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool HasInteractableNPC() const { return CurrentInteractableNPC != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetupReferences(UCameraComponent* Camera, UInventoryComponent* Inventory, USceneComponent* HeldAttachPoint);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugLines = false;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractionChanged OnInteractionChanged;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CheckForInteractables();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    AActor* GetHeldObject() const;

protected:
    virtual void BeginPlay() override;

private:
    void DrawDebugInteractionLine();

    bool HandleInventoryItemInteraction(UItem* Item, AActor* TargetActor);
    
    APedestal* FindPedestalFromActor(AActor* Actor) const;
    UItem* GetCurrentHeldInventoryItem() const;

    UPROPERTY()
    UCameraComponent* CameraRef;

    UPROPERTY()
    UInventoryComponent* InventoryRef;

    UPROPERTY()
    USceneComponent* HeldObjectAttachPoint;

    bool bIsLookingAtInteractable;

    UPROPERTY()
    AActor* CurrentInteractableActor;

    FString CurrentInteractionText;
    EInteractionType CurrentInteractionType;

    UPROPERTY()
    AUnia* CurrentInteractableNPC;

    UPROPERTY()
    TArray<AUnia*> InteractableNPCs;
};