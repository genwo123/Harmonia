#pragma once
#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Gameplay/PuzzleArea.h"
#include "Components/SphereComponent.h"
#include "Components/ChildActorComponent.h"
#include "Pedestal.generated.h"

class APickupActor;

UENUM(BlueprintType)
enum class EPedestalState : uint8
{
    Empty,
    Occupied
};

UCLASS(BlueprintType, Blueprintable)
class DISTRICT_TEST_API APedestal : public AInteractableActor
{
    GENERATED_BODY()
public:
    APedestal();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* AttachmentPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment | Editor Setup")
    TSubclassOf<APickupActor> PreAttachedActorClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attachment | Editor Setup")
    UChildActorComponent* AttachedActorComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment | Editor Setup",
        meta = (EditCondition = "PreAttachedActorClass != nullptr", EditConditionHides))
    FVector AttachmentOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment | Editor Setup",
        meta = (EditCondition = "PreAttachedActorClass != nullptr", EditConditionHides))
    FRotator AttachmentRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment | Editor Setup",
        meta = (EditCondition = "PreAttachedActorClass != nullptr", EditConditionHides))
    FVector AttachmentScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment | Editor Setup")
    bool bApplyInEditor = true;

    virtual void Interact_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool PlaceObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* RemoveObject();

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    AActor* GetPlacedObject() const { return PlacedObject; }

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void Rotate(float Degrees = 90.0f);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    bool Push(FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Attachment")
    void RotateAttachment(float Degrees);

    UFUNCTION(BlueprintPure, Category = "Attachment")
    float GetAttachmentRotation() const;

    UFUNCTION(BlueprintCallable, Category = "Attachment")
    void ClearAttachment();

    UFUNCTION(BlueprintCallable, Category = "Attachment")
    APickupActor* DetachAttachedActor();

    UFUNCTION(BlueprintPure, Category = "Attachment")
    APickupActor* GetAttachedActor() const;

    UFUNCTION(BlueprintPure, Category = "Attachment")
    bool HasAttachedActor() const;

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SetPuzzleArea(APuzzleArea* PuzzleArea);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SetGridPosition(int32 Row, int32 Column);

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void GetGridPosition(int32& OutRow, int32& OutColumn) const;

    UFUNCTION(BlueprintCallable, Category = "Pedestal")
    void SnapToGridCenter();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ExposeOnSpawn = true, AllowedClasses = "PuzzleArea"))
    TObjectPtr<APuzzleArea> TargetPuzzleArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ExposeOnSpawn = true))
    int32 TargetGridRow = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ExposeOnSpawn = true))
    int32 TargetGridColumn = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    bool bObjectFollowsRotation = true;

    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool MoveToGridPosition(int32 NewRow, int32 NewColumn);

#if WITH_EDITOR
    virtual void PostEditMove(bool bFinished) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedestal")
    EPedestalState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    AActor* PlacedObject;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    APuzzleArea* OwnerPuzzleArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    int32 GridRow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pedestal")
    int32 GridColumn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bAutoSnapToGrid = true;

    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    void FindOwnerPuzzleArea();
    void ClearPreviousCell();
    void UpdateAttachedActor();

    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};