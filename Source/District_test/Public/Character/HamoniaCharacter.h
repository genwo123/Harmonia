#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interaction/InteractionEnums.h"
#include "HamoniaCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UInventoryComponent;
class UPlayerInteractionComponent;
class USceneComponent;
class UHeldItemDisplayComponent;
class UStaticMeshComponent;
class UDialogueManagerComponent;
class UDataTable;
class AInteractableMechanism;
class AUnia;

UCLASS()
class DISTRICT_TEST_API AHamoniaCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AHamoniaCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    UPlayerInteractionComponent* InteractionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HeldObject")
    USceneComponent* HeldObjectAttachPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HeldObject")
    UHeldItemDisplayComponent* HeldItemDisplay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HeldObject")
    UStaticMeshComponent* HeldItemMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    UDialogueManagerComponent* DialogueManager;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RotateAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* PushAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* DropAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InventoryToggleAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InventoryLeftAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InventoryRightAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InventoryUseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float WalkSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float CrouchSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float JumpHeight = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
    float LookSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    UDataTable* DefaultDialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DefaultDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoStartDialogue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DelayBeforeDialogue = 1.0f;

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    UDialogueManagerComponent* GetDialogueManagerComponent() const { return DialogueManager; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FString GetDefaultDialogueID() const { return DefaultDialogueID; }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeDialogueSystem();

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueSystemReady() const;

    UFUNCTION(BlueprintCallable, Category = "Level Transition")
    void SaveBeforeLevelTransition();

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void SetCurrentInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void RemoveInteractableNPC(AUnia* NPC);

    UFUNCTION(BlueprintCallable, Category = "HeldItem")
    void ShowHeldItemMesh();

    UFUNCTION(BlueprintCallable, Category = "HeldItem")
    void HideHeldItemMesh();

protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartSprint();
    void StopSprint();
    void ToggleCrouch();
    void Interact();
    void RotateObject();
    void PushObject();
    void DropHeldObject();
    void OnInventoryToggle();
    void OnInventoryMoveLeft();
    void OnInventoryMoveRight();
    void OnInventoryUse();


    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bIsLookingAtInteractable;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* CurrentInteractableActor;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    FString CurrentInteractionText;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    EInteractionType CurrentInteractionType;

private:
    bool bIsSprinting = false;

    

    FTimerHandle DialogueInitTimerHandle;
    FTimerHandle LegacySyncTimer;
};