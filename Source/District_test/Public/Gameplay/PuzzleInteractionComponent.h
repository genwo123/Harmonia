// PuzzleInteractionComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PuzzleInteractionComponent.generated.h"

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

    // 현재 이 오브젝트가 받침대에 배치되어 있는지 여부
    UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
    class APedestal* CurrentPedestal;

    // 오브젝트가 들려질 수 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bCanBePickedUp;

    // 오브젝트가 받침대에 놓일 수 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
    bool bCanBePlacedOnPedestal;

    // 오브젝트 집기 함수 (플레이어가 호출)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PickUp(AActor* Picker);

    // 오브젝트 내려놓기 함수 (플레이어가 호출)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PutDown(FVector Location, FRotator Rotation);

    // 받침대에 배치 함수
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool PlaceOnPedestal(class APedestal* Pedestal);

    // 받침대에서 제거 함수
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool RemoveFromPedestal();

    // 이 오브젝트를 잡고 있는 액터 (일반적으로 플레이어 캐릭터)
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* HoldingActor;
};