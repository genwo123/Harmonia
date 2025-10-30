// PuzzleStarter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Sound/SoundBase.h"
#include "PuzzleStarter.generated.h"

class AGridMazeManager;

USTRUCT(BlueprintType)
struct FCoreSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slot")
    FName RequiredCoreTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slot")
    FVector SlotOffset = FVector::ZeroVector;

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
        SlotOffset = FVector::ZeroVector;
        SlotRotation = FRotator::ZeroRotator;
        SlotScale = FVector(1.0f, 1.0f, 1.0f);
        CustomSlotMesh = nullptr;
        SlotMesh = nullptr;
        InsertedCoreActor = nullptr;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoreInserted, FName, CoreTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllCoresInserted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWrongCoreInserted, FName, WrongCoreTag);

UCLASS(Blueprintable, BlueprintType)
class DISTRICT_TEST_API APuzzleStarter : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    APuzzleStarter();

protected:
    virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void OnConstruction(const FTransform& Transform) override;
#endif

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

public:
    // ============ 연결 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
    AGridMazeManager* ConnectedMazeManager;

    // ============ 코어 슬롯 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slots")
    TArray<FCoreSlot> CoreSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Slots")
    bool bAutoStartWhenComplete = true;

    // ============ 비주얼 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor EmptySlotColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor InsertedSlotColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float SlotSize = 50.0f;

    // ============ 사운드 설정 ============

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* CoreInsertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* WrongCoreSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* AllCoresCompleteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float SoundVolume = 1.0f;

    // ============ 상태 ============

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bAllCoresInserted = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bPuzzleStarted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* SlotMeshAsset;

    // ============ 델리게이트 ============

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCoreInserted OnCoreInserted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAllCoresInserted OnAllCoresInserted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWrongCoreInserted OnWrongCoreInserted;

    // ============ 주요 함수 ============

    // 코어 삽입 시도 (BP에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    bool TryInsertCore(AActor* CoreActor);

    // 모든 슬롯이 채워졌는지 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Puzzle")
    bool AreAllSlotsFilled() const;

    // 특정 슬롯이 비어있는지 확인
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Puzzle")
    bool IsSlotEmpty(int32 SlotIndex) const;

    // 퍼즐 시작
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void StartConnectedPuzzle();

    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    bool TryInsertCoreByTag(FName CoreTag);

    // 슬롯 리셋
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void ResetAllSlots();

    // InteractableInterface 구현
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override;
    virtual FString GetInteractionText_Implementation() override;
    virtual EInteractionType GetInteractionType_Implementation() override;

    // ============ BP 이벤트 ============

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