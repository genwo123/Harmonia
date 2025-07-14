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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FVector HoldOffset = FVector(100.0f, 0.0f, -20.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bMatchCameraRotation = true;

    // 피직스 설정 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bStartWithPhysicsDisabled = true; // 기본적으로 피직스 비활성화로 시작

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsWhenDropped = true; // 떨어뜨릴 때 피직스 활성화

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

private:
    // 피직스 설정 관리 함수들
    void SetupInitialPhysics();
    void EnablePhysics();
    void DisablePhysics();

    // 원래 피직스 설정 저장 (복원용)
    bool bOriginalSimulatePhysics;
    TEnumAsByte<ECollisionEnabled::Type> OriginalCollisionEnabled;
};
