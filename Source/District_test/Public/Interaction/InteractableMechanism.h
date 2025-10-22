#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Engine/DataTable.h"
#include "InteractableMechanism.generated.h"

UENUM(BlueprintType)
enum class EMechanismType : uint8
{
    None        UMETA(DisplayName = "None"),
    Door        UMETA(DisplayName = "Door"),
    Widget      UMETA(DisplayName = "Widget/UMG")
};

UENUM(BlueprintType)
enum class EWidgetSubType : uint8
{
    Keypad      UMETA(DisplayName = "Keypad Input"),
    MiniGame    UMETA(DisplayName = "Mini Game"),
    Hint        UMETA(DisplayName = "Hint Frame")
};

USTRUCT(BlueprintType)
struct DISTRICT_TEST_API FHintImageData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
    int32 LevelNumber = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint")
    UTexture2D* HintImage = nullptr;

    FHintImageData()
    {
        LevelNumber = 1;
        HintImage = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMechanismCompleted, FString, MechanismID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetSuccess, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetFailed, AActor*, Interactor);

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UWidgetComponent* InteractionPromptWidgetComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    class USoundBase* InteractionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanInteract;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EMechanismType MechanismType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity")
    FString MechanismID = TEXT("Mechanism_01");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Identity")
    FString MechanismGroup = TEXT("MainBuilding");

    UPROPERTY(BlueprintReadWrite, Category = "Interaction|State")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|State")
    bool bCanBeUsedAgain = false;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnMechanismCompleted OnMechanismCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Connected Doors",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget"))
    TArray<AInteractableMechanism*> ConnectedDoors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Key Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    bool bRequiresKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Key Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door && bRequiresKey"))
    FString RequiredKeyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Requirements",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    TArray<AInteractableMechanism*> RequiredMechanisms;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction|Door|State")
    bool bIsOpen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Animation",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    float OpenAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Door|Animation",
        meta = (EditCondition = "MechanismType == EMechanismType::Door"))
    float OpenSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget"))
    EWidgetSubType WidgetSubType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget")
    TSubclassOf<class UUserWidget> InteractionWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction|Widget")
    class UUserWidget* CurrentInteractionWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Keypad",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::Keypad"))
    FString CorrectCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Keypad",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::Keypad"))
    int32 MaxAttempts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|MiniGame",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::MiniGame",
            ClampMin = "1", ClampMax = "100"))
    int32 MiniGameStageNumber = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Hint",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::Hint"))
    UDataTable* HintImagesTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Widget|Hint",
        meta = (EditCondition = "MechanismType == EMechanismType::Widget && WidgetSubType == EWidgetSubType::Hint",
            ClampMin = "1", ClampMax = "9"))
    int32 HintLevelNumber = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Interaction|Widget|Hint")
    class UUserWidget* CurrentHintWidget;

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

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void ShowInteractionWidget();
    virtual void ShowInteractionWidget_Implementation() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void HideInteractionWidget();
    virtual void HideInteractionWidget_Implementation() override;

    void HandleDoorInteraction(AActor* Interactor);
    void HandleWidgetInteraction(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Interaction|Door")
    void ToggleDoor();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Door")
    void OnDoorOpened();

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Widget Events")
    FOnWidgetSuccess OnWidgetSuccess;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Widget Events")
    FOnWidgetFailed OnWidgetFailed;

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    void OnWidgetInteractionSuccess();

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    void OnWidgetInteractionFailed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Widget")
    void OnShowWidget(APlayerController* PlayerController);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Widget")
    void OnHideWidget();

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    FString GetCorrectCode() const { return CorrectCode; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    EWidgetSubType GetWidgetSubType() const { return WidgetSubType; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    int32 GetMaxAttempts() const { return MaxAttempts; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget")
    int32 GetMiniGameStageNumber() const { return MiniGameStageNumber; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget|Hint")
    UDataTable* GetHintImagesTable() const { return HintImagesTable; }

    UFUNCTION(BlueprintCallable, Category = "Interaction|Widget|Hint")
    int32 GetHintLevelNumber() const { return HintLevelNumber; }

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionSuccess(AActor* Interactor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void OnInteractionFailed(AActor* Interactor);

protected:
    UFUNCTION()
    void OnRequiredMechanismCompleted(FString CompletedMechanismID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|Events")
    void OnAllRequiredMechanismsCompleted();

private:
    TArray<FString> CompletedRequiredMechanisms;
};