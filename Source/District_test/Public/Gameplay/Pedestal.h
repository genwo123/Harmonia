#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractionEnums.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "Gameplay/PuzzleArea.h"
#include "Components/ChildActorComponent.h"
#include "Pedestal.generated.h"

UENUM(BlueprintType)
enum class EPedestalState : uint8
{
    Empty,
    Occupied
};

UCLASS()
class DISTRICT_TEST_API APedestal : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    APedestal();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* AttachmentPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attached Actor")
    TSubclassOf<AActor> PreAttachedActorClass;

    UPROPERTY(BlueprintReadOnly, Category = "Attached Actor")
    AActor* SpawnedChildActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    bool bCanRotate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    bool bObjectFollowsRotation = true;

    UPROPERTY(BlueprintReadOnly, Category = "Pedestal")
    EPedestalState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Pedestal")
    AActor* PlacedObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bUseGridSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bAutoSnapToGrid = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    APuzzleArea* TargetPuzzleArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 TargetGridRow = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 TargetGridColumn = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Grid")
    APuzzleArea* OwnerPuzzleArea;

    UPROPERTY(BlueprintReadOnly, Category = "Grid")
    int32 GridRow;

    UPROPERTY(BlueprintReadOnly, Category = "Grid")
    int32 GridColumn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FString InteractionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EInteractionType InteractionType;

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void Rotate(float Degrees = 45.0f);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool Push(FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool PlaceObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* RemoveObject();

    UFUNCTION(BlueprintPure, Category = "Pedestal")
    AActor* GetPlacedObject() const { return PlacedObject; }

    UFUNCTION(BlueprintCallable, Category = "Attached Actor")
    AActor* GetAttachedChildActor() const;

    UFUNCTION(BlueprintCallable, Category = "Attached Actor")
    UActorComponent* GetAttachedActorComponent(TSubclassOf<UActorComponent> ComponentClass);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SnapToGridCenter();

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool MoveToGridPosition(int32 NewRow, int32 NewColumn);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetPuzzleArea(APuzzleArea* PuzzleArea);

    UFUNCTION(BlueprintCallable, Category = "Grid")
    void SetGridPosition(int32 Row, int32 Column);


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* RotateSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* PushSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* PlaceObjectSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* RemoveObjectSound;

    UFUNCTION(BlueprintPure, Category = "Grid")
    void GetGridPosition(int32& OutRow, int32& OutColumn) const;

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) override { return true; }
    virtual FString GetInteractionText_Implementation() override { return InteractionText; }
    virtual EInteractionType GetInteractionType_Implementation() override { return InteractionType; }

protected:
    void FindOwnerPuzzleArea();
    void ClearPreviousCell();

    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditMove(bool bFinished) override;
#endif
};