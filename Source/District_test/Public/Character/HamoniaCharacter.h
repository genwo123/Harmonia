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

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    class UDialogueManagerComponent* DialogueManager;

    // DataTable 설정용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Dialogue")
    UDataTable* DefaultDialogueDataTable;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    UDialogueManagerComponent* GetDialogueManager();

    // 대화 이벤트 핸들러 (현재 구현 안 됨)
    UFUNCTION()
    void OnDialogueStarted(ESpeakerType Speaker, FText DialogueText, EDialogueType Type, float Duration);

    UFUNCTION()
    void OnDialogueEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueStartedBP(ESpeakerType Speaker, const FText& DialogueText, EDialogueType Type);

    UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
    void OnDialogueEndedBP();


    // Enhanced Input System 
    
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

    // 추가된 입력 액션 - 블루프린트에서 구현
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RotateAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* PushAction;

    // Character movement properties (BP editable)
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

    // Inventory Input
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryToggleAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryLeftAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryRightAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Inventory")
    UInputAction* InventoryUseAction;
    // E키 통합 처리 함수
    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnEKeyPressed();
    // 인벤토리 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryToggle();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryMoveLeft();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryMoveRight();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void OnInventoryUse();


    // 상호작용 UI 정보
    // DialogueTrigger에서 사용할 Getter 함수들
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

    // 추가: 상호작용 타입
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    EInteractionType CurrentInteractionType;

    // Input handlers
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

    //Inventory
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    UItem* GetCurrentHeldInventoryItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HandleInventoryItemInteraction(UItem* Item, AActor* TargetActor);

    // EXPANDED: 손에 든 아이템 표시 시스템
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

    // 컴포넌트 초기화 보장 함수 추가
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeDialogueSystem();

    // DataTable 설정 확인 함수 추가
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueSystemReady();

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TArray<AUnia*> InteractableNPCs;

    // 현재 가장 가까운 NPC
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AUnia* CurrentInteractableNPC;



private:
    bool bIsSprinting = false;

    // 입력 시스템 설정을 위한 함수
    void SetupEnhancedInput();
};