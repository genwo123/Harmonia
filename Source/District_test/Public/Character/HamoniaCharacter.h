// HamoniaCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "InputActionValue.h"
#include "Interaction/InteractableInterface.h"
#include "Gameplay/PuzzleInteractionComponent.h" 
#include "Gameplay/Pedestal.h"
#include "Interaction/InteractionEnums.h"
#include "Character/Unia.h"
#include "Core/DialogueManagerComponent.h" 
#include "HamoniaCharacter.generated.h"

class AUnia;
class UInventoryComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueProgressRequested);

UCLASS(Blueprintable)
class DISTRICT_TEST_API AHamoniaCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AHamoniaCharacter();

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueProgressRequested OnDialogueProgressRequested;


    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    class UDialogueManagerComponent* DialogueManager;

    // DataTable ¼³Á¤¿ë
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Dialogue")
    UDataTable* DefaultDialogueDataTable;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    UDialogueManagerComponent* GetDialogueManager();

    UFUNCTION()
    void OnDialogueStarted(ESpeakerType Speaker, FText DialogueText, EDialogueType Type, float Duration);

    UFUNCTION()
    void OnDialogueEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueStartedBP(ESpeakerType Speaker, const FText& DialogueText, EDialogueType Type);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueEndedBP();


    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RotateAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* PushAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpHeight = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LookSensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance = 400.0f;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryToggleAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryLeftAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryRightAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryUseAction;

    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnEKeyPressed();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryToggle();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryMoveLeft();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryMoveRight();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryUse();



    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    class UDialogueManagerComponent* GetDialogueManagerComponent() const { return DialogueManager; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetDefaultDialogueID() const { return DefaultDialogueID; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDefaultDialogueID(const FString& NewDialogueID) { DefaultDialogueID = NewDialogueID; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
    void OnInteractComplete(AActor* InteractedActor);


    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bIsLookingAtInteractable;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    FString CurrentInteractionText;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* CurrentInteractableActor;


    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    EInteractionType CurrentInteractionType;


    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartSprint(const FInputActionValue& Value);
    void StopSprint(const FInputActionValue& Value);
    void ToggleCrouch(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void Interact();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CheckForInteractables();


    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RotateObject();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnRotateObject();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PushObject();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPushObject();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInteractionLine();


    UFUNCTION(BlueprintCallable, Category = "Interaction")
    EInteractionType GetCurrentInteractionType() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsLookingAtInteractable() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FString GetCurrentInteractionText() const;


    UFUNCTION(BlueprintCallable, Category = "Interaction")
    AActor* GetHeldObject();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugLines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Dialogue")
    FString DefaultDialogueID = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Dialogue")
    bool bAutoStartDialogue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Dialogue")
    float DelayBeforeDialogue = 0.0f;


    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetCurrentHeldInventoryItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HandleInventoryItemInteraction(UItem* Item, AActor* TargetActor);


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UStaticMeshComponent* HeldItemDisplay;

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void ShowHeldItemMeshBP(UItem* Item);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
    void HideHeldItemMeshBP();

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    UItem* CurrentDisplayedItem;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateHeldItemDisplay(UItem* NewItem);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ShowHeldItemMesh(UItem* Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void HideHeldItemMesh();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventorySelectionChanged(int32 NewSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetCurrentInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RemoveInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AUnia* GetCurrentInteractableNPC() const { return CurrentInteractableNPC; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool HasInteractableNPC() const { return CurrentInteractableNPC != nullptr; }


protected:
    virtual void BeginPlay() override;


    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeDialogueSystem();


    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueSystemReady();

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TArray<AUnia*> InteractableNPCs;


    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AUnia* CurrentInteractableNPC;



private:
    bool bIsSprinting = false;


    void SetupEnhancedInput();
};