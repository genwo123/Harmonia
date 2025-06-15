// GridTile.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "Interaction/InteractableInterface.h"
#include "GridTile.generated.h"

UENUM(BlueprintType)
enum class ETileState : uint8
{
    Default     UMETA(DisplayName = "Default"),     // 회색 (기본)
    Correct     UMETA(DisplayName = "Correct"),     // 파란불 (올바른 경로)
    Wrong       UMETA(DisplayName = "Wrong"),       // 빨간불 (틀린 경로)
    Start       UMETA(DisplayName = "Start"),       // 시작점
    Goal        UMETA(DisplayName = "Goal"),        // 도착점
    Hint        UMETA(DisplayName = "Hint")         // 힌트 표시
};

// Forward Declaration
class AGridTile;

// 델리게이트 선언 수정
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileSteppedDelegate, AGridTile*, SteppedTile, AActor*, SteppedActor);

UCLASS()
class DISTRICT_TEST_API AGridTile : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AGridTile();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ====== COMPONENTS ======
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TileMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* TileLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    // ====== MANAGER REFERENCE ======
    UPROPERTY(BlueprintReadOnly, Category = "Manager")
    class AGridMazeManager* OwnerManager;

    // ====== TILE PROPERTIES ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Settings")
    ETileState CurrentState = ETileState::Default;

    UPROPERTY(BlueprintReadOnly, Category = "Tile Settings")
    int32 GridX = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Tile Settings")
    int32 GridY = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Settings")
    bool bIsPartOfCorrectPath = false;

    // ====== VISUAL SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor DefaultColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); // 어두운 회색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor CorrectColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // 파란색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor WrongColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // 빨간색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor StartColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // 초록색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor GoalColor = FLinearColor(1.0f, 1.0f, 0.2f, 1.0f); // 노란색

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor HintColor = FLinearColor(0.8f, 0.4f, 1.0f, 1.0f); // 보라색

    // ====== LIGHT SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float DefaultLightIntensity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float ActiveLightIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float LightRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float LightHeight = 50.0f; // 타일 위쪽 높이

    // ====== ANIMATION SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bEnableBlinking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlinkSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bEnablePulse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float PulseSpeed = 1.5f;

    // ====== AUDIO SETTINGS ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bPlayStepSound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* StepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SoundVolume = 1.0f;

public:
    // ====== EVENTS ======
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTileSteppedDelegate OnTileStepped;

    // ====== PUBLIC FUNCTIONS ======
    UFUNCTION(BlueprintCallable, Category = "Tile")
    void SetTileState(ETileState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tile")
    ETileState GetTileState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Tile")
    void SetGridPosition(int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, Category = "Tile")
    FVector2D GetGridPosition() const { return FVector2D(GridX, GridY); }

    UFUNCTION(BlueprintCallable, Category = "Tile")
    void SetCorrectPath(bool bIsCorrect);

    UFUNCTION(BlueprintCallable, Category = "Tile")
    bool IsCorrectPath() const { return bIsPartOfCorrectPath; }

    UFUNCTION(BlueprintCallable, Category = "Manager")
    void SetOwnerManager(class AGridMazeManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void StartBlinking(float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void StopBlinking();

    UFUNCTION(BlueprintCallable, Category = "Visual")
    void ShowHint(float Duration = 2.0f);

protected:
    // ====== INTERNAL VARIABLES ======
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsBlinking = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsShowingHint = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float BlinkTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float HintTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    UMaterialInstanceDynamic* DynamicMaterial;

    // Timer Handles
    FTimerHandle BlinkTimerHandle;
    FTimerHandle HintTimerHandle;

    // ====== TRIGGER EVENTS ======
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // ====== VISUAL FUNCTIONS ======
    void UpdateVisuals();
    void UpdateLightAndMaterial();
    void UpdateBlinking(float DeltaTime);
    void UpdateHint(float DeltaTime);

    FLinearColor GetCurrentStateColor();
    void PlayTileSound(USoundBase* Sound);

    // ====== BLUEPRINT EVENTS ======
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnTileStateChanged(ETileState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnPlayerEntered(AActor* Player);

public:
    // ====== INTERACTABLE INTERFACE ======
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;
};