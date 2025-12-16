// PuzzleInteractionComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PuzzleInteractionComponent.generated.h"

UENUM(BlueprintType)
enum class EPuzzleObjectSoundType : uint8
{
    Type1 UMETA(DisplayName = "Type 1 Box"),
    Type2 UMETA(DisplayName = "Type 2 Mirror")
};

UCLASS(ClassGroup = (Puzzle), meta = (BlueprintSpawnableComponent))
class DISTRICT_TEST_API UPuzzleInteractionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPuzzleInteractionComponent();
protected:
    virtual void BeginPlay() override;
public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FVector HoldOffset = FVector(100.0f, 0.0f, -20.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bMatchCameraRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bStartWithPhysicsDisabled = true; // 기본적으로 피직스 비활성화로 시작

    bool bIsPhysicsSimulating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsWhenDropped = true; // 떨어뜨릴 때 피직스 활성화

    UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
    class APedestal* CurrentPedestal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bCanBePickedUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bCanBePlacedOnPedestal;


    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PickUp(AActor* Picker);

    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    EPuzzleObjectSoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* PickupSound_Type1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* DropSound_Type1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* PickupSound_Type2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* DropSound_Type2;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PutDown(FVector Location, FRotator Rotation);


    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PlaceOnPedestal(class APedestal* Pedestal);


    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool RemoveFromPedestal();


    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* HoldingActor;



private:

    void SetupInitialPhysics();
    void EnablePhysics();
    void DisablePhysics();

    bool bOriginalSimulatePhysics;
    TEnumAsByte<ECollisionEnabled::Type> OriginalCollisionEnabled;
};
