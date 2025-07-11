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

    // 회전 및 밀기 함수 - C++에서 기본 구현 + BP에서 확장
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RotateObject();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnRotateObject(); // 블루프린트에서 구현 가능

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void PushObject();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnPushObject(); // 블루프린트에서 구현 가능

    // 디버그용 시각적 표시
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInteractionLine();

    // 상호작용 정보 획득 함수들
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    EInteractionType GetCurrentInteractionType() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool IsLookingAtInteractable() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    FString GetCurrentInteractionText() const;

    // 들고 있는 오브젝트 찾기 함수
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

    // 컴포넌트 초기화 보장 함수 추가
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitializeDialogueSystem();

    // DataTable 설정 확인 함수 추가
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueSystemReady();


private:
    bool bIsSprinting = false;

    // 입력 시스템 설정을 위한 함수
    void SetupEnhancedInput();
};