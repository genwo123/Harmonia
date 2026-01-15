// PuzzleStarter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraActor.h"
#include "PuzzleStarter.generated.h"

class AGridMazeManager;

USTRUCT(BlueprintType)
struct FCoreSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slot")
    FName RequiredCoreTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slot")
    FRotator SlotRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slot")
    FVector SlotScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slot")
    UStaticMesh* CustomSlotMesh = nullptr;

    UPROPERTY(BlueprintReadOnly)
    bool bIsInserted = false;

    UPROPERTY(BlueprintReadOnly)
    UStaticMeshComponent* SlotMesh = nullptr;

    UPROPERTY(BlueprintReadOnly)
    AActor* InsertedCoreActor = nullptr;

    FCoreSlot()
    {
        RequiredCoreTag = NAME_None;
        bIsInserted = false;
        SlotMesh = nullptr;
        InsertedCoreActor = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoreInserted, FName, CoreTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllCoresInserted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWrongCoreInserted, FName, WrongCoreTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyForUniaMode);

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API APuzzleStarter : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    APuzzleStarter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Slots")
    UStaticMeshComponent* SlotMesh1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Slots")
    UStaticMeshComponent* SlotMesh2;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Slots")
    UStaticMeshComponent* SlotMesh3;

#if WITH_EDITOR
    virtual void OnConstruction(const FTransform& Transform) override;
#endif

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    AGridMazeManager* ConnectedMazeManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview Camera")
    ACameraActor* PreviewCameraActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slots")
    TArray<FCoreSlot> CoreSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor EmptySlotColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor InsertedSlotColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float SlotSize = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* CoreInsertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* WrongCoreSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* AllCoresCompleteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float SoundVolume = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bAllCoresInserted = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bPuzzleStarted = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bWaitingForUniaMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* SlotMeshAsset;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCoreInserted OnCoreInserted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAllCoresInserted OnAllCoresInserted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWrongCoreInserted OnWrongCoreInserted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnReadyForUniaMode OnReadyForUniaMode;

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    bool TryInsertCore(AActor* CoreActor);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Puzzle")
    bool AreAllSlotsFilled() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Puzzle")
    bool IsSlotEmpty(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void StartConnectedPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    bool TryInsertCoreByTag(FName CoreTag);

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void ResetAllSlots();

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void OnUniaModeCompleted();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCoreInsertedBP(FName CoreTag, int32 SlotIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnAllCoresInsertedBP();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnWrongCoreInsertedBP(FName WrongCoreTag);

private:
    void CreateSlotMeshes();
    void UpdateSlotVisual(int32 SlotIndex, bool bInserted);
    FLinearColor GetCoreColorByTag(FName CoreTag);
    void PlaySound(USoundBase* Sound);
    int32 FindEmptySlotForCore(FName CoreTag);
};