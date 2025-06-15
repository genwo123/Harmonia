// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "InteractableMechanism.generated.h"

// 상호작용 유형 (단순화됨)
UENUM(BlueprintType)
enum class EMechanismType : uint8
{
    None        UMETA(DisplayName = "None"),
    Door        UMETA(DisplayName = "Door"),
    Widget      UMETA(DisplayName = "Widget/UMG")
};

// 위젯 하위 유형 (필요시 블루프린트에서 참조용)
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

    // 상호작용 메시 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    // 상호작용 감지용 구형 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    // ========================================================================================
    // BASIC INTERACTION PROPERTIES
    // ========================================================================================

    // 상호작용 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText;

    // 상호작용 소리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    class USoundBase* InteractionSound;

    // 상호작용 가능 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanInteract;

    // 메커니즘 유형
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EMechanismType MechanismType;

    // ========================================================================================
    // IDENTITY & STATE
    // ========================================================================================

    // 메커니즘 고유 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity")
    FString MechanismID = TEXT("Keypad_01");

    // 메커니즘 그룹 (선택사항)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity")
    FString MechanismGroup = TEXT("MainBuilding");

    // 완료 상태 여부
    UPROPERTY(BlueprintReadWrite, Category = "Interaction|State")
    bool bIsCompleted = false;

    // 재사용 가능 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|State")
    bool bCanBeUsedAgain = false;

    // ========================================================================================
    // DOOR TYPE PROPERTIES
    // ========================================================================================

    // === Key Requirements ===
    // 상호작용에 열쇠가 필요한지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Key Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    bool bRequiresKey;

    // 필요한 열쇠 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Key Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door && bRequiresKey"))
    FString RequiredKeyName;

    // === Keypad Requirements ===
    // 키패드 완료가 필요한지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Keypad Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    bool bRequiresKeypadCompletion = false;

    // 필요한 키패드 ID 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Keypad Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door && bRequiresKeypadCompletion"))
    TArray<FString> RequiredKeypadIDs;

    // === Door State & Animation ===
    // 문 상태
    UPROPERTY(BlueprintReadWrite, Category = "Interaction|Door|State")
    bool bIsOpen;

    // 문 애니메이션 관련 속성
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Animation",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    float OpenAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Animation",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    float OpenSpeed;

    // === Door Behavior ===
    // 문 개방 후 자동 닫힘 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Behavior",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    bool bAutoClose = false;

    // 자동 닫기 지연 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Behavior",
        meta = (EditCondition = "MechanismType == EMechanismType::Door && bAutoClose"))
    float AutoCloseDelay = 5.0f;

    // ========================================================================================
    // WIDGET TYPE PROPERTIES
    // ========================================================================================

    // 위젯 하위 유형 (Widget 유형 선택 시 추가 분류용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget"))
    EWidgetSubType WidgetSubType;

    // 상호작용 위젯 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget"))
    TSubclassOf<class UUserWidget> InteractionWidgetClass;

    // 현재 위젯 인스턴스
    UPROPERTY(BlueprintReadOnly, Category = "Interaction|Widget")
    class UUserWidget* CurrentInteractionWidget;

    // === Keypad Widget Properties ===
    // 키패드 정답 코드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Keypad",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::Keypad"))
    FString CorrectCode;

    // 키패드 최대 시도 횟수
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

    // 위젯에서 호출할 함수들
    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    void OnWidgetInteractionSuccess();

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    void OnWidgetInteractionFailed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Widget")
    void OnShowWidget(APlayerController* PlayerController);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Widget")
    void OnHideWidget();

    // 위젯 속성 접근자들
    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    FString GetCorrectCode() const { return CorrectCode; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    EWidgetSubType GetWidgetSubType() const { return WidgetSubType; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    int32 GetMaxAttempts() const { return MaxAttempts; }

    // ========================================================================================
    // BLUEPRINT EVENTS
    // ========================================================================================

    // 상호작용 효과 처리 (블루프린트에서 구현)
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionSuccess(AActor* Interactor);

    // 상호작용 실패 처리 (키가 없는 경우 등)
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionFailed(AActor* Interactor);

private:
    // ========================================================================================
    // PRIVATE HELPER FUNCTIONS
    // ========================================================================================

    // 키패드 완료 상태 확인
    bool CheckRequiredKeypads();

    // 자동 문 닫기
    void AutoCloseDoor();

    // 자동 닫기 타이머
    FTimerHandle AutoCloseTimerHandle;
};