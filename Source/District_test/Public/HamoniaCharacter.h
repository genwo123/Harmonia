#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InteractableInterface.h"
#include "InteractionEnums.h" // �߰��� �κ�
#include "HamoniaCharacter.generated.h"

class UInventoryComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS(Blueprintable)
class DISTRICT_TEST_API AHamoniaCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AHamoniaCharacter();
protected:
    virtual void BeginPlay() override;
public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* CameraComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;
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
    float InteractionDistance = 400.0f;  // ��ȣ�ۿ� �Ÿ� ����
    // ��ȣ�ۿ� UI ����
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

private:
    bool bIsSprinting = false;
    // �Է� �ý��� ������ ���� �Լ�
    void SetupEnhancedInput();
};