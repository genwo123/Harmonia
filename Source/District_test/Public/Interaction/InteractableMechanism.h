// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "InteractableMechanism.generated.h"

// ��ȣ�ۿ� ���� (�ܼ�ȭ��)
UENUM(BlueprintType)
enum class EMechanismType : uint8
{
    None        UMETA(DisplayName = "None"),
    Door        UMETA(DisplayName = "Door"),
    Widget      UMETA(DisplayName = "Widget/UMG")
};

// ���� ���� ���� (�ʿ�� �������Ʈ���� ������)
UENUM(BlueprintType)
enum class EWidgetSubType : uint8
{
    Keypad      UMETA(DisplayName = "Keypad Input"),
    MiniGame    UMETA(DisplayName = "Mini Game"),
    Quiz        UMETA(DisplayName = "Quiz")
};

UCLASS()
class DISTRICT_TEST_API AInteractableMechanism : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AInteractableMechanism();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ========================================================================================
    // COMPONENTS
    // ========================================================================================

    // ��ȣ�ۿ� �޽� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    // ��ȣ�ۿ� ������ ���� ������Ʈ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    // ========================================================================================
    // BASIC INTERACTION PROPERTIES
    // ========================================================================================

    // ��ȣ�ۿ� �ؽ�Ʈ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText;

    // ��ȣ�ۿ� �Ҹ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    class USoundBase* InteractionSound;

    // ��ȣ�ۿ� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanInteract;

    // ��Ŀ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EMechanismType MechanismType;

    // ========================================================================================
    // IDENTITY & STATE
    // ========================================================================================

    // ��Ŀ���� ���� ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity")
    FString MechanismID = TEXT("Keypad_01");

    // ��Ŀ���� �׷� (���û���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity")
    FString MechanismGroup = TEXT("MainBuilding");

    // �Ϸ� ���� ����
    UPROPERTY(BlueprintReadWrite, Category = "Interaction|State")
    bool bIsCompleted = false;

    // ���� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|State")
    bool bCanBeUsedAgain = false;

    // ========================================================================================
    // DOOR TYPE PROPERTIES
    // ========================================================================================

    // === Key Requirements ===
    // ��ȣ�ۿ뿡 ���谡 �ʿ����� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Key Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    bool bRequiresKey;

    // �ʿ��� ���� �̸�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Key Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door && bRequiresKey"))
    FString RequiredKeyName;

    // === Keypad Requirements ===
    // Ű�е� �Ϸᰡ �ʿ����� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Keypad Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    bool bRequiresKeypadCompletion = false;

    // �ʿ��� Ű�е� ID ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Keypad Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door && bRequiresKeypadCompletion"))
    TArray<FString> RequiredKeypadIDs;

    // === Door State & Animation ===
    // �� ����
    UPROPERTY(BlueprintReadWrite, Category = "Interaction|Door|State")
    bool bIsOpen;

    // �� �ִϸ��̼� ���� �Ӽ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Animation",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    float OpenAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Animation",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    float OpenSpeed;

    // === Door Behavior ===
    // �� ���� �� �ڵ� ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Behavior",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    bool bAutoClose = false;

    // �ڵ� �ݱ� ���� �ð�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Behavior",
        meta = (EditCondition = "MechanismType == EMechanismType::Door && bAutoClose"))
    float AutoCloseDelay = 5.0f;

    // ========================================================================================
    // WIDGET TYPE PROPERTIES
    // ========================================================================================

    // ���� ���� ���� (Widget ���� ���� �� �߰� �з���)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget"))
    EWidgetSubType WidgetSubType;

    // ��ȣ�ۿ� ���� Ŭ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget"))
    TSubclassOf<class UUserWidget> InteractionWidgetClass;

    // ���� ���� �ν��Ͻ�
    UPROPERTY(BlueprintReadOnly, Category = "Interaction|Widget")
    class UUserWidget* CurrentInteractionWidget;

    // === Keypad Widget Properties ===
    // Ű�е� ���� �ڵ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Keypad",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::Keypad"))
    FString CorrectCode;

    // Ű�е� �ִ� �õ� Ƚ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Keypad",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::Keypad"))
    int32 MaxAttempts;

    // ========================================================================================
    // INTERFACE IMPLEMENTATIONS
    // ========================================================================================

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);
    virtual void Interact_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor);
    virtual bool CanInteract_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FString GetInteractionText();
    virtual FString GetInteractionText_Implementation() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    EInteractionType GetInteractionType();
    virtual EInteractionType GetInteractionType_Implementation() override;

    // ========================================================================================
    // MECHANISM TYPE HANDLERS
    // ========================================================================================

    void HandleDoorInteraction(AActor* Interactor);
    void HandleWidgetInteraction(AActor* Interactor);

    // ========================================================================================
    // DOOR FUNCTIONS
    // ========================================================================================

    UFUNCTION(BlueprintCallable, Category = "Interaction|Door")
    void ToggleDoor();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Door")
    void OnDoorOpened();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Door")
    void OnDoorClosed();

    // ========================================================================================
    // WIDGET FUNCTIONS
    // ========================================================================================

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    void HideInteractionWidget();

    // �������� ȣ���� �Լ���
    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    void OnWidgetInteractionSuccess();

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    void OnWidgetInteractionFailed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Widget")
    void OnShowWidget(APlayerController* PlayerController);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Widget")
    void OnHideWidget();

    // ���� �Ӽ� �����ڵ�
    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    FString GetCorrectCode() const { return CorrectCode; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    EWidgetSubType GetWidgetSubType() const { return WidgetSubType; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    int32 GetMaxAttempts() const { return MaxAttempts; }

    // ========================================================================================
    // BLUEPRINT EVENTS
    // ========================================================================================

    // ��ȣ�ۿ� ȿ�� ó�� (�������Ʈ���� ����)
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionSuccess(AActor* Interactor);

    // ��ȣ�ۿ� ���� ó�� (Ű�� ���� ��� ��)
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionFailed(AActor* Interactor);

private:
    // ========================================================================================
    // PRIVATE HELPER FUNCTIONS
    // ========================================================================================

    // Ű�е� �Ϸ� ���� Ȯ��
    bool CheckRequiredKeypads();

    // �ڵ� �� �ݱ�
    void AutoCloseDoor();

    // �ڵ� �ݱ� Ÿ�̸�
    FTimerHandle AutoCloseTimerHandle;
};