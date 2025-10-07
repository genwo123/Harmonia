#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "GridTile.generated.h"

UENUM(BlueprintType)
enum class ETileState : uint8
{
    Default     UMETA(DisplayName = "Default"),
    Start       UMETA(DisplayName = "Start"),
    Correct     UMETA(DisplayName = "Correct"),
    Wrong       UMETA(DisplayName = "Wrong"),
    Goal        UMETA(DisplayName = "Goal"),
    Hint        UMETA(DisplayName = "Hint"),
    StartPoint  UMETA(DisplayName = "StartPoint")
};

class AGridTile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileSteppedDelegate, AGridTile*, SteppedTile, AActor*, SteppedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileStateChangedDelegate, ETileState, NewState);

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API AGridTile : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AGridTile();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* TileMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* PillarMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UPointLightComponent* TileLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Settings")
    ETileState InitialState = ETileState::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Settings")
    bool bCanBeInteracted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Colors")
    FLinearColor WaitingColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Colors")
    FLinearColor ReadyColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Colors")
    FLinearColor CorrectColor = FLinearColor(0.2f, 0.2f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Colors")
    FLinearColor WrongColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Colors")
    FLinearColor GoalColor = FLinearColor(1.0f, 1.0f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Colors")
    FLinearColor HintColor = FLinearColor(0.8f, 0.4f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Colors")
    FLinearColor StartPointColor = FLinearColor(0.4f, 1.0f, 0.4f, 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Manager Connection")
    void UpdateFromManagerColors();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float DefaultLightIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float ActiveLightIntensity = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float LightRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
    float LightHeight = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableBlinking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float BlinkSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnablePulsing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float PulseSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    bool bPlayInteractionSound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    class USoundBase* InteractionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    class USoundBase* CorrectSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    class USoundBase* WrongSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Settings")
    float SoundVolume = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ETileState CurrentState = ETileState::Default;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 GridX = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    int32 GridY = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsBlinking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsPulsing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsActivated = false;

public:
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTileSteppedDelegate OnTileStepped;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTileStateChangedDelegate OnTileStateChanged;

    UPROPERTY(BlueprintReadOnly, Category = "Manager")
    class AGridMazeManager* OwnerManager;

    UFUNCTION(BlueprintCallable, Category = "Tile Control")
    void SetTileState(ETileState NewState);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tile Information")
    ETileState GetTileState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Tile Control")
    void SetGridPosition(int32 X, int32 Y);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tile Information")
    FVector2D GetGridPosition() const { return FVector2D(GridX, GridY); }

    UFUNCTION(BlueprintCallable, Category = "Manager")
    void SetOwnerManager(class AGridMazeManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Tile Settings")
    void SetTileThickness(float NewThickness);

    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void TurnOnLight(FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void TurnOffLight();

    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void SetLightColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Light Control")
    void SetLightIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StartBlinking(float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopBlinking();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StartPulsing(float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopPulsing();

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void UpdateTileVisuals();

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ResetToDefault();

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayTileSound(USoundBase* Sound);

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayInteractionSound();

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayCorrectSound();

    UFUNCTION(BlueprintCallable, Category = "Sound")
    void PlayWrongSound();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetCanBeInteracted(bool bCanInteract);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
    bool CanBeInteracted() const { return bCanBeInteracted; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
    bool IsActivated() const { return bIsActivated; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    FLinearColor GetCurrentStateColor() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Information")
    bool IsAnimating() const { return bIsBlinking || bIsPulsing; }

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnPlayerInteracted(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnStateChanged(ETileState OldState, ETileState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnCorrectInteraction();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnWrongInteraction();

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnLightTurnedOn(FLinearColor Color);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint Events")
    void OnLightTurnedOff();

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Logic")
    bool ShouldProcessInteraction(AActor* InteractingActor);
    virtual bool ShouldProcessInteraction_Implementation(AActor* InteractingActor) { return true; }

    UFUNCTION(BlueprintNativeEvent, Category = "Custom Appearance")
    FLinearColor GetCustomStateColor(ETileState State) const;
    virtual FLinearColor GetCustomStateColor_Implementation(ETileState State) const;

protected:
    FTimerHandle BlinkTimerHandle;
    FTimerHandle PulseTimerHandle;

private:
    void UpdateBlinking(float DeltaTime);
    void UpdatePulsing(float DeltaTime);
    void ForceMobilitySettings();
};