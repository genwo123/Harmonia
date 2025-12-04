#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/InteractionEnums.h"
#include "PlayerInteractionComponent.generated.h"

class UCameraComponent;
class UInventoryComponent;
class UItem;
class AUnia;
class APedestal;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInteractionChanged, bool, bCanInteract, FString, InteractionText, EInteractionType, InteractionType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowWarningMessage, FString, Message);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UPlayerInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerInteractionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bShowDebugLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bEnableOutline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float DropDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugLineThickness = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bIsLookingAtInteractable;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* CurrentInteractableActor;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    FString CurrentInteractionText;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    EInteractionType CurrentInteractionType;

    UPROPERTY(BlueprintAssignable, Category = "Interaction Events")
    FOnInteractionChanged OnInteractionChanged;

    UPROPERTY(BlueprintAssignable, Category = "Interaction Events")
    FOnShowWarningMessage OnShowWarningMessage;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetupReferences(UCameraComponent* Camera, UInventoryComponent* Inventory, USceneComponent* HeldAttachPoint);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PerformInteraction();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void DropHeldObject();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RotatePedestal();

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AActor* GetCurrentInteractableActor() const { return CurrentInteractableActor; }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PushPedestal();

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void SetCurrentInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC")
    void RemoveInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HandleInventoryItemInteraction(UItem* Item, AActor* TargetActor);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    UItem* GetCurrentHeldInventoryItem() const;

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AActor* GetHeldObject() const;

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void EnableOutline(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Outline")
    void DisableOutline(AActor* Actor);

    UPROPERTY()
    AActor* CurrentHeldObject;

protected:
    void CheckForInteractables();
    void DrawDebugInteractionLine();

    bool PlaceOnPedestal(APedestal* Pedestal, AActor* ObjectToPlace);
    bool PickUpFromPedestal(APedestal* Pedestal);
    bool PickUpObject(AActor* ObjectToPickUp);

    APedestal* FindPedestalFromActor(AActor* Actor) const;

    void ReEnableInteractionCheck();

private:
    UPROPERTY()
    UCameraComponent* CameraRef;

    UPROPERTY()
    UInventoryComponent* InventoryRef;

    UPROPERTY()
    USceneComponent* HeldObjectAttachPoint;

    UPROPERTY()
    AUnia* CurrentInteractableNPC;

    UPROPERTY()
    TArray<AUnia*> InteractableNPCs;

    bool bIgnoreInteractionCheck;
    FTimerHandle IgnoreCheckTimerHandle;
};