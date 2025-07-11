// HamoniaCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "InputActionValue.h"
#include "Interaction/InteractableInterface.h"
#include "Gameplay/PuzzleInteractionComponent.h" 
#include "Gameplay/Pedestal.h"
#include "Interaction/InteractionEnums.h"
#include "Core/DialogueManagerComponent.h" 
#include "HamoniaCharacter.generated.h"

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

    // DataTable ������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Dialogue")
    UDataTable* DefaultDialogueDataTable;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    UDialogueManagerComponent* GetDialogueManager();

    // ��ȭ �̺�Ʈ �ڵ鷯 (���� ���� �� ��)
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

    // �߰��� �Է� �׼� - �������Ʈ���� ����
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

    // ��ȣ�ۿ� UI ����
    // DialogueTrigger���� ����� Getter �Լ���
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

    // �߰�: ��ȣ�ۿ� Ÿ��
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

    // ȸ�� �� �б� �Լ� - C++���� �⺻ ���� + BP���� Ȯ��
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RotateObject();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnRotateObject(); // �������Ʈ���� ���� ����

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PushObject();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPushObject(); // �������Ʈ���� ���� ����

    // ����׿� �ð��� ǥ��
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInteractionLine();

    // ��ȣ�ۿ� ���� ȹ�� �Լ���
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    EInteractionType GetCurrentInteractionType() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsLookingAtInteractable() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FString GetCurrentInteractionText() const;

    // ��� �ִ� ������Ʈ ã�� �Լ�
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

protected:
    virtual void BeginPlay() override;

    // ������Ʈ �ʱ�ȭ ���� �Լ� �߰�
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeDialogueSystem();

    // DataTable ���� Ȯ�� �Լ� �߰�
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueSystemReady();


private:
    bool bIsSprinting = false;

    // �Է� �ý��� ������ ���� �Լ�
    void SetupEnhancedInput();
};